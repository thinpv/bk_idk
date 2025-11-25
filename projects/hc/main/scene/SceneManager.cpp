#include "SceneManager.h"
#include "Log.h"
#include "DeviceManager.h"
#include "Database.h"
#include "Gateway.h"

SceneManager::SceneManager()
{
}

SceneManager *SceneManager::GetInstance()
{
	static SceneManager *sceneManager = NULL;
	if (!sceneManager)
	{
		sceneManager = new SceneManager();
	}
	return sceneManager;
}

void SceneManager::ForEach(function<void(Scene *)> func)
{
	sceneListMtx.lock();
	for (auto &[id, scene] : sceneList)
	{
		func(scene);
	}
	sceneListMtx.unlock();
}

void SceneManager::AddScene(Json::Value &sharedValue)
{
	startCheckScene();
	if (sharedValue.isObject() &&
			sharedValue.isMember("scenes") && sharedValue["scenes"].isObject())
	{
		Json::Value &scenesValue = sharedValue["scenes"];
		if (scenesValue.isObject())
		{
			for (const auto &id : scenesValue.getMemberNames())
			{
				Scene *scene = AddScene(id, scenesValue[id]);
				scene->setWaitingToCheck(false);
			}
		}
	}
	stopCheckScene();
}

Scene *SceneManager::AddScene(string id, Json::Value &sceneValue, bool isSaveToDB)
{
	LOGD("AddScene id: %s value: %s", id.c_str(), sceneValue.toString().c_str());
	if (sceneValue.isObject() &&
			sceneValue.isMember("name") && sceneValue["name"].isString() &&
			sceneValue.isMember("devices") && sceneValue["devices"].isArray() &&
			sceneValue.isMember("ts") && sceneValue["ts"].isUInt64())
	{
		string name = sceneValue["name"].asString();
		Json::Value devicesValues = sceneValue["devices"];
		uint64_t updatedAt = sceneValue["ts"].asUInt64();

		Scene *scene = GetSceneFromId(id);
		if (scene)
		{
			if (scene->getUpdatedAt() != updatedAt)
			{
				LOGD("Scene %s already exists, updating...", id.c_str());
				scene->setName(name);
				scene->setUpdatedAt(updatedAt);
				Database::GetInstance()->SceneUpdate(scene);
			}
		}
		else
		{
			int sceneAddr = 0;
			if (sceneValue.isMember("addr") && sceneValue["addr"].isUInt())
			{
				sceneAddr = sceneValue["addr"].asUInt();
			}
			else
			{
				sceneAddr = Gateway::GetInstance()->getNextAndIncreaseSceneAddr();
			}

			LOGD("Creating new scene %s addr: %d", id.c_str(), sceneAddr);
			scene = new Scene(id, name, sceneAddr, updatedAt);
			if (scene)
			{
				AddScene(scene);
				if (isSaveToDB)
					Database::GetInstance()->SceneAdd(scene);
			}
		}

		if (scene)
		{
			// Add all devices to scene
			scene->StartCheckDeviceInScene();
			for (Json::Value::ArrayIndex i = 0; i < devicesValues.size(); i++)
			{
				Json::Value deviceValue = devicesValues[i];
				if (deviceValue.isObject() &&
						deviceValue.isMember("id") && deviceValue["id"].isString())
				{
					string deviceId = deviceValue["id"].asString();
					Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
					if (device)
					{
						uint16_t epId = 0;
						if (deviceValue.isMember("epId") && deviceValue["epId"].isUInt())
						{
							epId = deviceValue["epId"].asUInt();
						}

						DeviceInScene *deviceInScene = scene->AddDeviceAndConfig(device, epId);
						if (deviceInScene)
						{
							deviceInScene->setWaitingToCheck(false);
						}
					}
					else
					{
						LOGW("Device %s not found in scene", deviceId.c_str());
					}
				}
				else
				{
					LOGW("Device value error in scene");
				}
			}
			scene->StopCheckDeviceInScene();

			return scene;
		}
		else
		{
			LOGE("New scene error, check format");
		}
	}
	else
	{
		LOGE("New scene error, check format");
	}
	return NULL;
}

void SceneManager::AddScene(Scene *scene)
{
	if (scene)
	{
		sceneListMtx.lock();
		sceneList[scene->getId()] = scene;
		sceneListMtx.unlock();
	}
}

Scene *SceneManager::GetSceneFromId(string id)
{
	sceneListMtx.lock();
	if (sceneList.find(id) != sceneList.end())
	{
		sceneListMtx.unlock();
		return sceneList[id];
	}
	sceneListMtx.unlock();
	return NULL;
}

Scene *SceneManager::GetSceneFromAddr(uint16_t addr)
{
	sceneListMtx.lock();
	for (const auto &[id, scene] : sceneList)
	{
		if (scene->getAddr() == addr)
		{
			sceneListMtx.unlock();
			return scene;
		}
	}
	sceneListMtx.unlock();
	return NULL;
}

uint16_t SceneManager::GetNextSceneAddr()
{
	uint16_t sceneAddr = 1; // start add of normal scene
	sceneListMtx.lock();
	for (const auto &[id, scene] : sceneList)
	{
		if (scene->getAddr() >= sceneAddr && scene->getAddr() < 4096)
		{
			sceneAddr = scene->getAddr() + 1;
		}
	}
	sceneListMtx.unlock();
	return sceneAddr;
}

// Del dev in all scene, scenBle, room
void SceneManager::DelScene(Scene *scene)
{
	sceneListMtx.lock();
	sceneList.erase(scene->getId());
	sceneListMtx.unlock();
	delete scene;
}

void SceneManager::DelAllScene()
{
	sceneListMtx.lock();
	for (auto &[id, scene] : sceneList)
		delete scene;
	sceneList.clear();
	sceneListMtx.unlock();
}

void SceneManager::PrintScene()
{
	sceneListMtx.lock();
	for (auto &[id, grp] : sceneList)
	{
		LOGD("scene: %s", id.c_str());
		for (auto &dev : grp->deviceList)
		{
			LOGD("\tdev:%s: 0X%04X", dev->device->getId().c_str(), dev->epId);
		}
	}
	sceneListMtx.unlock();
}

// đưa tất cả thiết bị trong nhóm vào trạng thái chờ kiểm tra, nếu không còn thuộc nhóm thì phải xóa khỏi danh sách
void SceneManager::startCheckScene()
{
	sceneListMtx.lock();
	for (const auto &[id, scene] : sceneList)
	{
		scene->setWaitingToCheck(true);
	}
	sceneListMtx.unlock();
}

// xóa tất cả các thiết bị không còn thuộc nhóm
void SceneManager::stopCheckScene()
{
	sceneListMtx.lock();
	for (auto it = sceneList.begin(); it != sceneList.end();)
	{
		auto scene = it->second;
		if (scene->isWaitingToCheck())
		{
			LOGI("Delete scene %s", scene->getName().c_str());
			Database::GetInstance()->SceneDel(scene);
			delete scene;
			it = sceneList.erase(it);
		}
		else
		{
			++it;
		}
	}
	sceneListMtx.unlock();
}