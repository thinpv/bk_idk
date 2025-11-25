#include "Scene.h"
#include <thread>
#include <algorithm>
#include "Log.h"
#include "Database.h"
#include "Base64.h"

#ifdef CONFIG_ENABLE_BLE
#include "BleProtocol.h"
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#endif

void DeviceInScene::setIsConfigured(bool value)
{
	isConfigured = value;
	dataValue["isConfigured"] = value;
}

void DeviceInScene::setIsInScene(bool value)
{
	isInScene = value;
	dataValue["isInScene"] = value;
}

void DeviceInScene::setRetryCount(int value)
{
	retryCount = value;
	dataValue["retryCount"] = value;
}

void DeviceInScene::setData(const string &value)
{
	string dataStr;
	string decode = macaron::Base64::Decode(value, dataStr);
	if (decode == "")
	{
		dataValue.parse(dataStr);
	}
	else
	{
		dataValue.parse(value);
	}

	if (!dataValue.isObject())
	{
		dataValue = Json::objectValue;
	}
	parseDataValue(dataValue);
}

void DeviceInScene::parseDataValue(Json::Value &dataValue)
{
	if (dataValue.isObject())
	{
		if (dataValue.isMember("isConfigured") && dataValue["isConfigured"].isBool())
		{
			isConfigured = dataValue["isConfigured"].asBool();
		}

		if (dataValue.isMember("isInScene") && dataValue["isInScene"].isBool())
		{
			isInScene = dataValue["isInScene"].asBool();
		}

		if (dataValue.isMember("retryCount") && dataValue["retryCount"].isInt())
		{
			retryCount = dataValue["retryCount"].asInt();
		}
	}
	LOGD("DeviceInScene: %s, epId: %d, sceneAddr: %d isConfigured: %d, isInScene: %d, retryCount: %d",
			 device->getName().c_str(),
			 epId,
			 sceneAddr,
			 isConfigured,
			 isInScene,
			 retryCount);
}

Scene::Scene(string id, string name, uint16_t addr, uint64_t updatedAt)
		: Object(id, name, addr, updatedAt)
{
}

Scene::Scene(string id, string name, uint16_t addr)
		: Object(id, name, addr, time(NULL))
{
	waitingToCheck = false;
}

Scene::~Scene()
{
	deviceListMtx.lock();
	LOGI("Delete all devices in scene %s", name.c_str());
	for (auto it = deviceList.begin(); it != deviceList.end();)
	{
		DeleteDeviceInScene(*it, true);
		it = deviceList.erase(it);
	}
	deviceList.clear();
	deviceListMtx.unlock();
}

size_t Scene::countDevice()
{
	deviceListMtx.lock();
	size_t size = deviceList.size();
	deviceListMtx.unlock();
	return size;
}

DeviceInScene *Scene::GetDeviceInScene(Device *device, uint16_t epId)
{
	deviceListMtx.lock();
	for (auto &deviceInScene : deviceList)
	{
		if (deviceInScene->device == device && deviceInScene->epId == epId)
		{
			deviceListMtx.unlock();
			return deviceInScene;
		}
	}
	deviceListMtx.unlock();
	return NULL;
}

int Scene::GetPositionDevice(Device *device, uint16_t epId)
{
	string deviceId = device->getId();
	deviceListMtx.lock();
	for (uint32_t i = 0; i < deviceList.size(); i++)
	{
		if ((deviceId == deviceList[i]->device->getId()) && (deviceList[i]->epId == epId))
		{
			deviceListMtx.unlock();
			return i;
		}
	}
	deviceListMtx.unlock();
	return -1;
}

DeviceInScene *Scene::AddDeviceInScene(DeviceInScene *deviceInScene)
{
	if (deviceInScene &&
			GetDeviceInScene(deviceInScene->device, deviceInScene->epId) == NULL)
	{
		deviceListMtx.lock();
		deviceList.push_back(deviceInScene);
		deviceListMtx.unlock();
	}
	return deviceInScene;
}

DeviceInScene *Scene::AddDeviceAndConfig(Device *device, uint16_t epId)
{
	if (device)
	{
		DeviceInScene *deviceInScene = GetDeviceInScene(device, epId);
		if (deviceInScene == NULL)
		{
			deviceInScene = new DeviceInScene(device, epId, addr, true, false);
			if (deviceInScene)
			{
				deviceListMtx.lock();
				deviceList.push_back(deviceInScene);
				deviceListMtx.unlock();

				if (deviceInScene->getIsConfigured() == false)
				{
					if (device->AddToScene(epId, addr) == CODE_OK)
					{
						deviceInScene->setIsConfigured(true);
					}
					else
					{
						device->AddDeviceInSceneUnconfig(deviceInScene);
					}
				}
				Database::GetInstance()->DeviceInSceneAdd(deviceInScene);
			}
		}
		return deviceInScene;
	}
	return NULL;
}

int Scene::DelDevice(Device *device, uint16_t epId)
{
	if (device)
	{
		int deviceIndex = GetPositionDevice(device, epId);
		if (deviceIndex >= 0)
		{
			deviceListMtx.lock();
			deviceList.erase(deviceList.begin() + deviceIndex);
			deviceListMtx.unlock();
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Scene::DelDeviceAndConfig(Device *device, uint16_t epId)
{
	if (device)
	{
		DeviceInScene *deviceInScene = GetDeviceInScene(device, epId);
		if (deviceInScene)
		{
			if (deviceInScene->getIsConfigured())
			{
				if (deviceInScene->device->RemoveFromScene(deviceInScene->epId, addr) == CODE_OK)
				{
					Database::GetInstance()->DeviceInSceneDel(deviceInScene);
					delete deviceInScene;
				}
				else
				{
					deviceInScene->setIsInScene(false);
					Database::GetInstance()->DeviceInSceneUpdateData(deviceInScene);
					device->AddDeviceInSceneUnconfig(deviceInScene);
				}
			}

			deviceListMtx.lock();
			deviceList.erase(remove(deviceList.begin(), deviceList.end(), deviceInScene), deviceList.end());
			deviceListMtx.unlock();
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Scene::DeleteDeviceInScene(DeviceInScene *deviceInScene, bool needConfig)
{
	if (needConfig)
	{
		if (deviceInScene->getIsConfigured())
		{
			if (deviceInScene->device->RemoveFromScene(deviceInScene->epId, addr) == CODE_OK)
			{
				Database::GetInstance()->DeviceInSceneDel(deviceInScene);
				delete deviceInScene;
			}
			else
			{
				deviceInScene->setIsInScene(false);
				Database::GetInstance()->DeviceInSceneUpdateData(deviceInScene);
				deviceInScene->device->AddDeviceInSceneUnconfig(deviceInScene);
			}
		}
	}
	else
	{
		Database::GetInstance()->DeviceInSceneDel(deviceInScene);
		delete deviceInScene;
	}
	return CODE_OK;
}

int Scene::RemoveDevice(Device *device, bool needConfig)
{
	deviceListMtx.lock();
	for (auto it = deviceList.begin(); it != deviceList.end();)
	{
		DeviceInScene *deviceInScene = *it;
		if (deviceInScene->device == device)
		{
			DeleteDeviceInScene(deviceInScene, needConfig);
			it = deviceList.erase(it);
		}
		else
		{
			++it;
		}
	}
	deviceListMtx.unlock();
	return CODE_OK;
}

// đưa tất cả thiết bị trong nhóm vào trạng thái chờ kiểm tra, nếu không còn thuộc nhóm thì phải xóa khỏi danh sách
void Scene::StartCheckDeviceInScene()
{
	deviceListMtx.lock();
	for (auto &deviceInScene : deviceList)
	{
		deviceInScene->setWaitingToCheck(true);
	}
	deviceListMtx.unlock();
}

// xóa tất cả các thiết bị không còn thuộc nhóm
void Scene::StopCheckDeviceInScene()
{
	deviceListMtx.lock();
	for (auto it = deviceList.begin(); it != deviceList.end();)
	{
		DeviceInScene *deviceInScene = *it;
		if (deviceInScene->getWaitingToCheck())
		{
			LOGI("Device %s with epId %d is not in scene %s, remove it", deviceInScene->device->getId().c_str(), deviceInScene->epId, getId().c_str());
			DeleteDeviceInScene(deviceInScene, true);
			it = deviceList.erase(it);
		}
		else
		{
			++it;
		}
	}
	deviceListMtx.unlock();
}

int Scene::Do(Json::Value &dataValue, bool ack)
{
	if (dataValue.isObject())
	{
	}
	return CODE_OK;
}

int Scene::Do(bool ack, uint16_t groupAddr)
{
#ifdef CONFIG_ENABLE_BLE
	BleProtocol::GetInstance()->SceneRecall(0xffff, addr, TRANSITION_DEFAULT, ack);
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
	ZigbeeProtocol::GetInstance()->SceneRecall(0xffff, 0xff, addr, groupAddr);
#endif
	return CODE_OK;
}
