#include "Gateway.h"
#include "Log.h"
#include "Database.h"
#include <unordered_set>
#include <algorithm>

void Gateway::InitRpcScene()
{
	OnRpcCallbackRegister("activeScene", bind(&Gateway::OnActiveScene, this, placeholders::_1, placeholders::_2));

	// 	OnRpcCallbackRegister("controlScene", bind(&Gateway::OnControlScene, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("createScene", bind(&Gateway::OnCreateScene, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("addDevToScene", bind(&Gateway::OnAddDevToScene, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("delDevFromScene", bind(&Gateway::OnDelDevToScene, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("editScene", bind(&Gateway::OnEditScene, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("delScene", bind(&Gateway::OnDeleteScene, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("updateSceneName", bind(&Gateway::OnUpdateSceneName, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("callScene", bind(&Gateway::OnCallScene, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("createSceneController", bind(&Gateway::OnCreateSceneController, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("delSceneController", bind(&Gateway::OnDelSceneController, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("getSceneList", bind(&Gateway::OnGetSceneList, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("getDevListInScene", bind(&Gateway::OnGetDevListInScene, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("scenes", bind(&Gateway::OnScenesSync, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnActiveScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnActiveScene");
	int rs = CODE_ERROR;
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string sceneId = reqValue["id"].asString();
		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
		if (scene)
		{
			if (scene->countDevice() <= 20)
			{
				rs = scene->Do(true);
			}
			else
			{
				rs = scene->Do(false);
			}
		}
	}
	return rs;
}

// int Gateway::OnControlScene(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnControlScene");
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string sceneId = reqValue["id"].asString();
// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 		if (scene)
// 		{
// 			int rs = CODE_OK;
// 			if (scene->deviceList.size() <= 20)
// 				rs = scene->Do(true);
// 			else
// 			{
// 				rs = scene->Do(false);
// 				Json::Value devicesData = Json::arrayValue;
// 				for (auto &devInScene : scene->deviceList)
// 				{
// 					if (devInScene->device->isOnline())
// 					{
// 						Json::Value deviceData;
// 						deviceData["id"] = devInScene->device->getId();
// 						deviceData["data"] = devInScene->data;
// 						devicesData.append(deviceData);
// 					}
// 				}
// 				Json::Value dataPush;
// 				dataPush["device"] = devicesData;
// 				Gateway::GetInstance()->pushDeviceUpdateCloud(dataPush);
// 			}
// 			respValue["params"]["code"] = rs;
// 		}
// 		else
// 		{
// 			LOGW("Scene %s dose not exsit", sceneId.c_str());
// 			respValue["params"]["code"] = CODE_NOT_FOUND_SCENE;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 		LOGW("OnControlScene %s format error", reqValue.toString().c_str());
// 	}
// 	respValue["method"] = "controlSceneRsp";
// 	return CODE_OK;
// }

// int Gateway::OnCreateScene(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString() &&
// 			reqValue.isMember("name") && reqValue["name"].isString() &&
// 			reqValue.isMember("devices") && reqValue["devices"].isArray())
// 	{
// 		Json::Value dataRsp;
// 		dataRsp["method"] = "createSceneRsp";
// 		dataRsp["params"]["code"] = CODE_OK;
// 		// CloudProtocol::Publish(topicRsp, dataRsp.toString());

// 		int msgId = CloudProtocol::GetMsgId();
// 		CloudProtocol::SetMsgId(++msgId);
// 		// topicRsp = "v1/devices/me/rpc/request/" + to_string(CloudProtocol::GetMsgId());

// 		Json::Value successList = Json::arrayValue;
// 		Json::Value failedList = Json::arrayValue;
// 		string sceneId = reqValue["id"].asString();
// 		string sceneName = reqValue["name"].asString();
// 		Json::Value deviceList = reqValue["devices"];
// 		uint16_t roomAdd = 0;

// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 		if (!scene)
// 		{
// 			scene = new Scene(sceneId, sceneName, SceneManager::GetInstance()->GetNextSceneAddr());
// 			if (scene)
// 			{
// 				SceneManager::GetInstance()->AddScene(scene);
// 				Database::GetInstance()->SceneAdd(scene);
// 			}
// 		}

// 		if (scene)
// 		{
// 			if (reqValue.isMember("room") && reqValue["room"].isString())
// 			{
// 				string roomId = reqValue["room"].asString();
// 				// Room *room = RoomManager::GetInstance()->GetRoomFromId(roomId);
// 				// if (room)
// 				// {
// 				// 	respValue["params"]["room"] = room->getId();
// 				// 	roomAdd = room->getAddr();
// 				// 	room->AddScene(scene);
// 				// 	Database::GetInstance()->SceneUpdateRoom(scene, roomId);
// 				// }
// 			}

// 			for (auto &deviceValue : deviceList)
// 			{
// 				if (deviceValue.isObject() &&
// 						deviceValue.isMember("id") && deviceValue["id"].isString() &&
// 						deviceValue.isMember("data") && deviceValue["data"].isObject())
// 				{
// 					Json::Value deviceProperties = deviceValue["data"];
// 					string deviceId = deviceValue["id"].asString();
// 					Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 					if (device)
// 					{
// 						if (device->AddToScene(scene->getAddr(), deviceProperties, roomAdd) == CODE_OK)
// 						{
// 							scene->AddDevice(device, deviceProperties);
// 							Database::GetInstance()->DeviceInSceneAdd(scene, device, deviceProperties.toString());
// 							Json::Value sceneRs = Json::objectValue;
// 							sceneRs["id"] = device->getId();
// 							sceneRs["data"] = deviceProperties;
// 							successList.append(sceneRs);
// 						}
// 						else
// 						{
// 							device->RemoveFromScene(scene->getAddr(), deviceProperties, roomAdd);
// 							failedList.append(device->getId());
// 						}
// 					}
// 					SLEEP_MS(100);
// 				}
// 			}
// 			respValue["params"]["code"] = CODE_OK;
// 			respValue["params"]["id"] = scene->getId();
// 			respValue["params"]["name"] = scene->getName();
// 			respValue["params"]["success"] = successList;
// 			respValue["params"]["failed"] = failedList;

// 			SceneManager::GetInstance()->PrintScene();
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_MEMORY_ERROR;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "createSceneRspComplete";
// 	return CODE_OK;
// }

// int Gateway::OnAddDevToScene(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString() &&
// 			reqValue.isMember("name") && reqValue["name"].isString() &&
// 			reqValue.isMember("devices") && reqValue["devices"].isArray())
// 	{
// 		Json::Value dataRsp;
// 		dataRsp["method"] = "addDevToSceneRsp";
// 		dataRsp["params"]["code"] = CODE_OK;
// 		// CloudProtocol::Publish(topicRsp, dataRsp.toString());

// 		int msgId = CloudProtocol::GetMsgId();
// 		CloudProtocol::SetMsgId(++msgId);
// 		// topicRsp = "v1/devices/me/rpc/request/" + to_string(CloudProtocol::GetMsgId());

// 		Json::Value successList = Json::arrayValue;
// 		Json::Value failedList = Json::arrayValue;
// 		Json::Value deviceList = reqValue["devices"];
// 		string sceneId = reqValue["id"].asString();
// 		string sceneName = reqValue["name"].asString();
// 		uint16_t roomAddr = 0;

// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 		if (!scene)
// 		{
// 			scene = new Scene(sceneId, sceneName, SceneManager::GetInstance()->GetNextSceneAddr());
// 			if (scene)
// 			{
// 				SceneManager::GetInstance()->AddScene(scene);
// 				Database::GetInstance()->SceneAdd(scene);
// 			}
// 		}
// 		if (scene)
// 		{
// 			if (reqValue.isMember("roomId") && reqValue["roomId"].isString())
// 			{
// 				string roomId = reqValue["roomId"].asString();
// 				// Room *room = RoomManager::GetInstance()->GetRoomFromId(roomId);
// 				// if (room)
// 				// {
// 				// 	respValue["params"]["room"] = room->getId();
// 				// 	roomAddr = room->getAddr();
// 				// 	room->AddScene(scene);
// 				// 	Database::GetInstance()->SceneUpdateRoom(scene, roomId);
// 				// }
// 			}
// 			for (auto &deviceValue : deviceList)
// 			{
// 				if (deviceValue.isObject() &&
// 						deviceValue.isMember("id") && deviceValue["id"].isString() &&
// 						deviceValue.isMember("data") && deviceValue["data"].isObject())
// 				{
// 					Json::Value deviceProperties = deviceValue["data"];
// 					string deviceId = deviceValue["id"].asString();
// 					Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 					if (device)
// 					{
// 						if (device->AddToScene(scene->getAddr(), deviceProperties, roomAddr) == CODE_OK)
// 						{
// 							scene->AddDevice(device, deviceProperties);
// 							Database::GetInstance()->DeviceInSceneAdd(scene, device, deviceProperties.toString());
// 							Json::Value sceneRs = Json::objectValue;
// 							sceneRs["id"] = device->getId();
// 							sceneRs["data"] = deviceProperties;
// 							successList.append(sceneRs);
// 						}
// 						else
// 						{
// 							device->RemoveFromScene(scene->getAddr(), deviceProperties, roomAddr);
// 							failedList.append(device->getId());
// 						}
// 					}
// 				}
// 				SLEEP_MS(100);
// 			}
// 			respValue["params"]["code"] = CODE_OK;
// 			respValue["params"]["id"] = scene->getId();
// 			respValue["params"]["name"] = scene->getName();
// 			respValue["params"]["success"] = successList;
// 			respValue["params"]["failed"] = failedList;

// 			SceneManager::GetInstance()->PrintScene();
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_SCENE;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "addDevToSceneRspComplete";
// 	return CODE_OK;
// }

// int Gateway::OnDelDevToScene(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString() &&
// 			reqValue.isMember("devices") && reqValue["devices"].isArray())
// 	{
// 		Json::Value dataRsp;
// 		dataRsp["method"] = "delDevFromSceneRsp";
// 		dataRsp["params"]["code"] = CODE_OK;
// 		// CloudProtocol::Publish(topicRsp, dataRsp.toString());

// 		int msgId = CloudProtocol::GetMsgId();
// 		CloudProtocol::SetMsgId(++msgId);
// 		// topicRsp = "v1/devices/me/rpc/request/" + to_string(CloudProtocol::GetMsgId());

// 		Json::Value successList = Json::arrayValue;
// 		Json::Value failedList = Json::arrayValue;
// 		Json::Value deviceList = reqValue["devices"];
// 		string sceneId = reqValue["id"].asString();
// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 		if (scene)
// 		{
// 			uint16_t roomAddr = 0;
// 			// Room *room = RoomManager::GetInstance()->GetRoomSceneIn(scene);
// 			// if (room)
// 			// {
// 			// 	roomAddr = room->getAddr();
// 			// }
// 			for (auto &deviceValue : deviceList)
// 			{
// 				if (deviceValue.isObject() &&
// 						deviceValue.isMember("id") && deviceValue["id"].isString() &&
// 						deviceValue.isMember("data") && deviceValue["data"].isObject())
// 				{
// 					Json::Value deviceProperties = deviceValue["data"];
// 					string deviceId = deviceValue["id"].asString();
// 					Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 					if (device)
// 					{
// 						int prositionDevInScene = scene->GetPositionDevice(device);
// 						if (prositionDevInScene > -1)
// 						{
// 							if (device->RemoveFromScene(scene->getAddr(), scene->deviceList[prositionDevInScene]->data, roomAddr) == CODE_OK)
// 							{
// 								scene->DelDevice(device);
// 								Database::GetInstance()->DeviceInSceneDel(scene, device);
// 								successList.append(deviceId);
// 							}
// 							else
// 							{
// 								failedList.append(deviceId);
// 							}
// 						}
// 						else
// 						{
// 							LOGW("Device %s not in scene %s", deviceId.c_str(), sceneId.c_str());
// 							failedList.append(deviceId);
// 						}
// 					}
// 					SLEEP_MS(100);
// 				}
// 			}

// 			respValue["params"]["code"] = CODE_OK;
// 			respValue["params"]["id"] = scene->getId();
// 			respValue["params"]["success"] = successList;
// 			respValue["params"]["failed"] = failedList;

// 			SceneManager::GetInstance()->PrintScene();
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_SCENE;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "delDevFromSceneRspComplete";
// 	return CODE_OK;
// }

// int Gateway::OnEditScene(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("EditScene");
// 	if (reqValue.isMember("id") && reqValue["id"].isString() &&
// 			reqValue.isMember("name") && reqValue["name"].isString() &&
// 			reqValue.isMember("devices") && reqValue["devices"].isArray())
// 	{
// 		Json::Value dataRsp;
// 		dataRsp["method"] = "editSceneRsp";
// 		dataRsp["params"]["code"] = CODE_OK;
// 		// CloudProtocol::Publish(topicRsp, dataRsp.toString());

// 		int msgId = CloudProtocol::GetMsgId();
// 		CloudProtocol::SetMsgId(++msgId);
// 		// topicRsp = "v1/devices/me/rpc/request/" + to_string(CloudProtocol::GetMsgId());

// 		Json::Value successList = Json::arrayValue;
// 		Json::Value failedList = Json::arrayValue;
// 		string sceneId = reqValue["id"].asString();
// 		string sceneName = reqValue["name"].asString();
// 		uint16_t roomAddr = 0;
// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 		if (!scene)
// 		{
// 			scene = new Scene(sceneId, sceneName, SceneManager::GetInstance()->GetNextSceneAddr());
// 			if (scene)
// 			{
// 				SceneManager::GetInstance()->AddScene(scene);
// 				Database::GetInstance()->SceneAdd(scene);
// 			}
// 		}
// 		if (scene)
// 		{
// 			if (reqValue.isMember("room") && reqValue["room"].isString())
// 			{
// 				string roomId = reqValue["room"].asString();
// 				// Room *room = RoomManager::GetInstance()->GetRoomFromId(roomId);
// 				// if (room)
// 				// {
// 				// 	respValue["params"]["room"] = room->getId();
// 				// 	roomAddr = room->getAddr();
// 				// 	room->AddScene(scene);
// 				// 	Database::GetInstance()->SceneUpdateRoom(scene, roomId);
// 				// }
// 			}

// 			vector<Device *> devsInScene; // list dev in scene
// 			map<Device *, Json::Value> dataDevsInScene;
// 			for (auto &devs : scene->deviceList)
// 			{
// 				devsInScene.push_back(devs->device);
// 				dataDevsInScene[devs->device] = devs->data;
// 			}
// 			// vector<Device *> listDevicesDel; // list old dev del scene
// 			vector<Device *> devsDelFromScene;
// 			// vector<Device *> devsEditScene;	 // list dev edit scene
// 			vector<Device *> devsEditScene;

// 			Json::Value deviceList = reqValue["devices"];
// 			map<Device *, Json::Value> listData;
// 			for (auto &deviceValue : deviceList)
// 			{
// 				if (deviceValue.isObject() &&
// 						deviceValue.isMember("id") && deviceValue["id"].isString() &&
// 						deviceValue.isMember("data") && deviceValue["data"].isObject())
// 				{
// 					Json::Value deviceProperties = deviceValue["data"];
// 					string deviceId = deviceValue["id"].asString();
// 					Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 					if (device)
// 					{
// 						listData[device] = deviceProperties;
// 						devsEditScene.push_back(device);
// 					}
// 				}
// 			}

// 			// Find list device del scene
// 			for (auto &item : devsInScene)
// 			{
// 				if (find(devsEditScene.begin(), devsEditScene.end(), item) == devsEditScene.end())
// 				{
// 					devsDelFromScene.push_back(item);
// 				}
// 			}

// 			// Phai xoa list device delscene truoc
// 			for (auto &item : devsDelFromScene)
// 			{
// 				if (item->RemoveFromScene(scene->getAddr(), dataDevsInScene[item]) == CODE_OK)
// 				{
// 					scene->DelDevice(item);
// 					Database::GetInstance()->DeviceInSceneDel(scene, item);
// 					Json::Value sceneRs = Json::objectValue;
// 					sceneRs["id"] = item->getId();
// 					sceneRs["data"] = dataDevsInScene[item];
// 					successList.append(sceneRs);
// 				}
// 				else
// 				{
// 					failedList.append(item->getId());
// 				}
// 				SLEEP_MS(100);
// 			}

// 			for (auto &item : devsEditScene)
// 			{
// 				// Check edit scene nếu là công tắc ble phải xóa data trước
// 				if (item->EditInScene(scene->getAddr(), dataDevsInScene[item]) == CODE_OK)
// 				{
// 					scene->DelDevice(item);
// 					Database::GetInstance()->DeviceInSceneDel(scene, item);
// 				}
// 				if (item->AddToScene(scene->getAddr(), listData[item], roomAddr) == CODE_OK)
// 				{
// 					scene->AddDevice(item, listData[item]);
// 					Database::GetInstance()->DeviceInSceneAdd(scene, item, listData[item].toString());
// 					Json::Value sceneRs = Json::objectValue;
// 					sceneRs["id"] = item->getId();
// 					sceneRs["data"] = listData[item];
// 					successList.append(sceneRs);
// 				}
// 				else
// 				{
// 					item->RemoveFromScene(scene->getAddr(), listData[item], roomAddr);
// 					failedList.append(item->getId());
// 				}
// 				SLEEP_MS(100);
// 			}

// 			respValue["params"]["code"] = CODE_OK;
// 			respValue["params"]["id"] = scene->getId();
// 			respValue["params"]["name"] = scene->getName();
// 			respValue["params"]["success"] = successList;
// 			respValue["params"]["failed"] = failedList;

// 			SceneManager::GetInstance()->PrintScene();
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_SCENE;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "editSceneRspComplete";
// 	return CODE_OK;
// }

// int Gateway::OnDeleteScene(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		Json::Value successList = Json::arrayValue;
// 		Json::Value failedList = Json::arrayValue;
// 		string sceneId = reqValue["id"].asString();
// 		mtxDelScene.lock();
// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 		if (scene)
// 		{
// 			uint16_t roomAddr = 0;
// 			// Room *room = RoomManager::GetInstance()->GetRoomSceneIn(scene);
// 			// if (room)
// 			// {
// 			// 	roomAddr = room->getAddr();
// 			// }
// 			vector<DeviceInScene *> devicesInScene = scene->deviceList;
// 			for (auto &deviceInScene : devicesInScene)
// 			{
// 				if (deviceInScene->device->RemoveFromScene(scene->getAddr(), deviceInScene->data, roomAddr) == CODE_OK)
// 				{
// 					scene->DelDevice(deviceInScene->device);
// 					Database::GetInstance()->DeviceInSceneDel(scene, deviceInScene->device);
// 					successList.append(deviceInScene->device->getId());
// 				}
// 				else
// 				{
// 					failedList.append(deviceInScene->device->getId());
// 				}
// 				SLEEP_MS(100);
// 			}

// 			respValue["params"]["code"] = CODE_OK;
// 			respValue["params"]["id"] = sceneId;
// 			respValue["params"]["success"] = successList;
// 			respValue["params"]["failed"] = failedList;

// 			if (scene->deviceList.size() <= 0)
// 			{
// 				// vector<Room *> listRoom;
// 				// RoomManager::GetInstance()->ForEach([&](Room *room)
// 				// 																		{ listRoom.push_back(room); });
// 				// for (auto &room : listRoom)
// 				// {
// 				// 	if (room && room->isSceneInRoom(scene))
// 				// 	{
// 				// 		room->SceneDelDevice(scene);
// 				// 	}
// 				// }

// 				Database::GetInstance()->SceneDel(scene);
// 				SceneManager::GetInstance()->SceneDelDevice(scene);
// 			}
// 			SceneManager::GetInstance()->PrintScene();
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_SCENE;
// 		}
// 		mtxDelScene.unlock();
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "delSceneRsp";
// 	return CODE_OK;
// }

// int Gateway::OnUpdateSceneName(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("name") && reqValue["name"].isString() && reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string id = reqValue["id"].asString();
// 		string name = reqValue["name"].asString();
// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(id);
// 		if (scene)
// 		{
// 			scene->setName(name);
// 			Database::GetInstance()->SceneUpdate(scene);
// 		}
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "updateSceneNameRsp";
// 	return CODE_OK;
// }

// int Gateway::OnGetSceneList(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGW("OnGetSceneList");
// 	// Json::Value sceneData = Json::arrayValue;
// 	// sceneListMtx.lock();
// 	// for (const auto &[id, scene] : sceneList)
// 	// {
// 	// 	Json::Value sceneValue;
// 	// 	sceneValue["id"] = scene->getId();
// 	// 	sceneValue["name"] = scene->getName();
// 	// 	sceneData.append(sceneValue);
// 	// }
// 	// sceneListMtx.unlock();
// 	// respValue["data"]["scenes"] = sceneData;
// 	// respValue["data"]["code"] = CODE_OK;
// 	// respValue["cmd"] = "getSceneListRsp";
// 	return CODE_OK;
// }

// int Gateway::OnGetDevListInScene(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnGetDevListInScene");
// 	if (reqValue.isMember("scenes") && reqValue["scenes"].isArray() && reqValue["scenes"].size() > 0)
// 	{
// 		Json::Value scenesList = Json::arrayValue;
// 		Json::Value scenes = reqValue["scenes"];
// 		for (auto &sceneValue : scenes)
// 		{
// 			Json::Value scenesData = Json::objectValue;
// 			if (sceneValue.isString())
// 			{
// 				string sceneId = sceneValue.asString();
// 				scenesData["id"] = sceneId;
// 				Scene *temp_scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 				if (temp_scene)
// 				{
// 					Json::Value temp_devicesList = Json::arrayValue;
// 					for (unsigned int i = 0; i < temp_scene->deviceList.size(); i++)
// 					{
// 						DeviceInScene *deviceInScene = temp_scene->deviceList[i];
// 						string deviceId = deviceInScene->device->getId();
// 						temp_devicesList.append(deviceId);
// 					}
// 					scenesData["devices"] = temp_devicesList;
// 				}
// 				else
// 					respValue["params"]["code"] = CODE_NOT_FOUND_ROOM;
// 			}
// 			else
// 				respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 			scenesList.append(scenesData);
// 		}
// 		respValue["params"]["scenes"] = scenesList;
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "getDevListInScene";
// 	return CODE_OK;
// }

// int Gateway::OnCallScene(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string sceneId = reqValue["id"].asString();
// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 		if (scene)
// 		{
// 			uint16_t roomAddr = 0;
// 			// Room *room = RoomManager::GetInstance()->GetRoomSceneIn(scene);
// 			// if (room)
// 			// {
// 			// 	roomAddr = room->getAddr();
// 			// }

// 			scene->Do(true, roomAddr);
// 			respValue["params"]["code"] = CODE_OK;
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_SCENE;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "callSceneRsp";
// 	return CODE_OK;
// }

// int Gateway::ConfigSceneForRemote(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene)
// {
// #ifdef CONFIG_ENABLE_BLE
// 	if (!device)
// 		return CODE_ERROR;

// 	if (data.isArray() && scene.isObject())
// 	{
// 		int result = CODE_OK;

// 		if (scene.isMember("id") && scene["id"].isString())
// 		{
// 			string sceneId = scene["id"].asString();
// 			Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 			if (scene)
// 			{
// 				uint16_t convertButton = 0;
// 				uint8_t bt1 = 0;
// 				uint8_t bt2 = 0;
// 				uint8_t bt3 = 0;
// 				uint8_t bt4 = 0;
// 				uint8_t bt5 = 0;
// 				uint8_t bt6 = 0;
// 				int sizeData = data.size();
// 				for (auto &dt : data)
// 				{
// 					if (dt.isObject())
// 					{
// 						if (device->getType() == BLE_DC_SCENE_CONTACT || device->getType() == BLE_REMOTE_M3 || device->getType() == BLE_REMOTE_M3_V2 || device->getType() == BLE_REMOTE_M4)
// 						{
// 							if (isAddScene)
// 							{
// 								if (dt.isMember("bt") && dt["bt"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 1, dt["bt"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt2") && dt["bt2"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 2, dt["bt2"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt3") && dt["bt3"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 3, dt["bt3"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt4") && dt["bt4"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 4, dt["bt4"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt5") && dt["bt5"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 5, dt["bt5"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt6") && dt["bt6"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 6, dt["bt6"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 							}
// 							else
// 							{
// 								if (dt.isMember("bt") && dt["bt"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 1, dt["bt"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt2") && dt["bt2"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 2, dt["bt2"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt3") && dt["bt3"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 3, dt["bt3"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt4") && dt["bt4"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 4, dt["bt4"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt5") && dt["bt5"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 5, dt["bt5"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt6") && dt["bt6"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneDC(device->getAddr(), 6, dt["bt6"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 							}
// 						}

// 						if (device->getType() == BLE_AC_SCENE_CONTACT || device->getType() == BLE_AC_SCENE_CONTACT_RGB || device->getType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE)
// 						{
// 							if (isAddScene)
// 							{
// 								if (dt.isMember("bt") && dt["bt"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 1, dt["bt"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt2") && dt["bt2"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 2, dt["bt2"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt3") && dt["bt3"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 3, dt["bt3"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt4") && dt["bt4"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 4, dt["bt4"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt5") && dt["bt5"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 5, dt["bt5"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt6") && dt["bt6"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 6, dt["bt6"].asInt(), scene->getAddr(), 0) != CODE_OK)
// 										result = CODE_ERROR;
// 							}
// 							else
// 							{
// 								if (dt.isMember("bt") && dt["bt"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 1, dt["bt"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt2") && dt["bt2"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 2, dt["bt2"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt3") && dt["bt3"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 3, dt["bt3"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt4") && dt["bt4"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 4, dt["bt4"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt5") && dt["bt5"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 5, dt["bt5"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 								if (dt.isMember("bt6") && dt["bt6"].isInt())
// 									if (BleProtocol::GetInstance()->SetSceneSwitchSceneAC(device->getAddr(), 6, dt["bt6"].asInt(), 0, 0) != CODE_OK)
// 										result = CODE_ERROR;
// 							}
// 						}
// 						if (device->getType() == BLE_SEFTPOWER_REMOTE_1 || device->getType() == BLE_SEFTPOWER_REMOTE_2 || device->getType() == BLE_SEFTPOWER_REMOTE_3 || device->getType() == BLE_SEFTPOWER_REMOTE_6)
// 						{
// 							if (dt.isMember("bt") && dt["bt"].isInt())
// 								bt1 = 1;
// 							else if (dt.isMember("bt2") && dt["bt2"].isInt())
// 								bt2 = 1;
// 							else if (dt.isMember("bt3") && dt["bt3"].isInt())
// 								bt3 = 1;
// 							else if (dt.isMember("bt4") && dt["bt4"].isInt())
// 								bt4 = 1;
// 							else if (dt.isMember("bt5") && dt["bt5"].isInt())
// 								bt5 = 1;
// 							else if (dt.isMember("bt6") && dt["bt6"].isInt())
// 								bt6 = 1;
// 							if (sizeData == 1)
// 							{
// 								convertButton = bt1 + bt2 * 2 + bt3 * 4 + bt4 * 8 + bt5 * 16 + bt6 * 32;
// 								// DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(device);
// 								// if (deviceBleSeftPowerRemote)
// 								// {
// 								// 	Device *parent = deviceBleSeftPowerRemote->GetParent();
// 								// 	if (parent)
// 								// 	{
// 								// 		if (isAddScene)
// 								// 		{
// 								// 			if (BleProtocol::GetInstance()->SetSceneSeftPowerRemote(parent->getAddr(), deviceBleSeftPowerRemote->getAddr(), convertButton, 1, scene->getAddr()) != CODE_OK)
// 								// 				result = CODE_ERROR;
// 								// 		}
// 								// 		else
// 								// 		{
// 								// 			if (BleProtocol::GetInstance()->DelSceneSeftPowerRemote(parent->getAddr(), deviceBleSeftPowerRemote->getAddr(), convertButton, 1) != CODE_OK)
// 								// 				result = CODE_ERROR;
// 								// 		}
// 								// 	}
// 								// 	else
// 								// 		LOGW("Parent is null");
// 								// }
// 								// else
// 								// 	LOGW("device seftpower remote is null");
// 							}
// 							sizeData--;
// 						}
// 					}
// 				}
// 			}
// 			else
// 			{
// 				LOGW("Scene %s not found", sceneId.c_str());
// 				result = CODE_ERROR;
// 			}
// 		}
// 		else
// 			result = CODE_ERROR;
// 		return result;
// 	}
// 	else
// 	{
// 		LOGW("Data error format");
// 	}
// #endif
// 	return CODE_ERROR;
// }

// int Gateway::ConfigSceneForPirSensor(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene)
// {
// #ifdef CONFIG_ENABLE_BLE
// 	if (!device)
// 		return CODE_ERROR;
// 	if (data.isArray() && scene.isObject())
// 	{
// 		int result = CODE_OK;
// 		if (scene.isMember("id") && scene["id"].isString())
// 		{
// 			string sceneId = scene["id"].asString();
// 			Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 			if (scene)
// 			{
// 				if (isAddScene)
// 				{
// 					int pir = 0;
// 					uint16_t luxHigh = 0;
// 					uint16_t luxLow = 0;
// 					bool isPir = false;
// 					bool isLux = false;
// 					for (auto &dt : data)
// 					{
// 						if (dt.isObject())
// 						{
// 							if (dt.isMember("pir") && dt["pir"].isInt())
// 							{
// 								pir = dt["pir"].asInt();
// 								isPir = true;
// 							}
// 							if (dt.isMember("lux") && dt["lux"].isArray())
// 							{
// 								if (dt["lux"][0].isInt() && dt["lux"][1].isInt())
// 								{
// 									luxLow = dt["lux"][0].asInt();
// 									luxHigh = dt["lux"][1].asInt();
// 									isLux = true;
// 								}
// 							}
// 							if (isLux && isPir)
// 								if (BleProtocol::GetInstance()->SetScenePirLightSensor(device->getAddr(), 2, pir, luxLow / 10, luxHigh / 10, scene->getAddr(), 0) != CODE_OK)
// 									result = CODE_ERROR;
// 						}
// 					}
// 				}
// 				else
// 				{
// 					for (auto &dt : data)
// 					{
// 						if (dt.isMember("pir") && dt["pir"].isInt())
// 						{
// 							if (BleProtocol::GetInstance()->SetScenePirLightSensor(device->getAddr(), 2, dt["pir"].asInt(), 0, 0, 0, 0) != CODE_OK)
// 								result = CODE_ERROR;
// 						}
// 					}
// 				}
// 			}
// 		}
// 		return result;
// 	}
// 	else
// 	{
// 		LOGW("Data error format");
// 		return CODE_ERROR;
// 	}
// #endif
// 	return CODE_OK;
// }

// int Gateway::ConfigSceneForScreenTouch(Device *device, Json::Value &data, Json::Value &sceneValue, bool isAddScene)
// {
// #ifdef CONFIG_ENABLE_BLE
// 	if (!device)
// 		return CODE_ERROR;

// 	if (sceneValue.isObject() && sceneValue.isMember("id") && sceneValue["id"].isString())
// 	{
// 		string sceneId = sceneValue["id"].asString();
// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 		if (scene)
// 		{
// 			if (isAddScene)
// 			{
// 				if (sceneValue.isMember("icon") && sceneValue["icon"].isInt())
// 				{
// 					int iconId = sceneValue["icon"].asInt();
// 					if (BleProtocol::GetInstance()->SceneForScreenTouch(device->getAddr(), scene->getAddr(), iconId, 0) == CODE_OK)
// 						return CODE_OK;
// 				}
// 			}
// 			else
// 			{
// 				if (BleProtocol::GetInstance()->DelSceneScreenTouch(device->getAddr(), scene->getAddr()) == CODE_OK)
// 					return CODE_OK;
// 			}
// 		}
// 	}
// 	else
// 	{
// 		LOGW("Data error");
// 	}
// #endif
// 	return CODE_ERROR;
// }

// int Gateway::OnCreateSceneController(Json::Value &reqValue, Json::Value &respValue)
// {
// 	respValue["method"] = "createSceneControllerRsp";
// 	if (reqValue.isMember("devId") && reqValue["devId"].isString() &&
// 			reqValue.isMember("data") && reqValue["data"].isArray())
// 	{
// 		string deviceId = reqValue["devId"].asString();
// 		Json::Value dataJson = reqValue["data"];
// 		int result = CODE_OK;

// 		Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 		if (device)
// 		{
// 			for (auto &dt : dataJson)
// 			{
// 				if (dt.isObject() && dt.isMember("properties") && dt["properties"].isArray() && dt.isMember("scene") && dt["scene"].isObject())
// 				{
// 					Json::Value propertiesJson = dt["properties"];
// 					Json::Value sceneJson = dt["scene"];
// 					int indexType1 = (device->getType() >> 16) & 0xFF;
// 					int indexType2 = (device->getType() >> 8) & 0xFF;
// 					if (device->getType() == BLE_REMOTE_M3 || device->getType() == BLE_REMOTE_M3_V2 || device->getType() == BLE_REMOTE_M4 || device->getType() == BLE_DC_SCENE_CONTACT ||
// 							device->getType() == BLE_AC_SCENE_CONTACT || device->getType() == BLE_AC_SCENE_CONTACT_RGB || device->getType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE ||
// 							(indexType1 == 2 && indexType2 == 7)) // seftpower remote
// 					{
// 						result = ConfigSceneForRemote(device, propertiesJson, sceneJson, true);
// 					}
// 					else if (indexType1 == 3 && indexType2 == 2)
// 					{
// 						result = ConfigSceneForPirSensor(device, propertiesJson, sceneJson, true);
// 					}
// 					else if (device->getType() == BLE_AC_SCENE_SCREEN_TOUCH || device->getType() == BLE_SWITCH_KNOB)
// 					{
// 						result = ConfigSceneForScreenTouch(device, propertiesJson, sceneJson, true);
// 					}
// 				}
// 				else
// 				{
// 					LOGW("Data error");
// 				}
// 			}
// 			respValue["params"]["code"] = result;
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_DEVICE;
// 		}
// 	}
// 	return CODE_OK;
// }

// int Gateway::OnDelSceneController(Json::Value &reqValue, Json::Value &respValue)
// {
// 	respValue["method"] = "delSceneControllerRsp";
// 	if (reqValue.isMember("devId") && reqValue["devId"].isString() &&
// 			reqValue.isMember("data") && reqValue["data"].isArray())
// 	{
// 		string deviceId = reqValue["devId"].asString();
// 		Json::Value dataJson = reqValue["data"];
// 		int result = CODE_OK;

// 		Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 		if (device)
// 		{
// 			for (auto &dt : dataJson)
// 			{
// 				if (dt.isObject() && dt.isMember("properties") && dt["properties"].isArray() && dt.isMember("scene") && dt["scene"].isObject())
// 				{
// 					Json::Value propertiesJson = dt["properties"];
// 					Json::Value sceneJson = dt["scene"];
// 					int indexType1 = (device->getType() >> 16) & 0xFF;
// 					int indexType2 = (device->getType() >> 8) & 0xFF;
// 					if (device->getType() == BLE_REMOTE_M3 || device->getType() == BLE_REMOTE_M3_V2 || device->getType() == BLE_REMOTE_M4 || device->getType() == BLE_DC_SCENE_CONTACT ||
// 							device->getType() == BLE_AC_SCENE_CONTACT || device->getType() == BLE_AC_SCENE_CONTACT_RGB || device->getType() == BLE_AC_SCENE_CONTACT_RGB_SQUARE ||
// 							(indexType1 == 2 && indexType2 == 7)) // seftpower remote
// 					{
// 						result = ConfigSceneForRemote(device, propertiesJson, sceneJson, false);
// 					}
// 					else if (indexType1 == 3 && indexType2 == 2)
// 					{
// 						result = ConfigSceneForPirSensor(device, propertiesJson, sceneJson, false);
// 					}
// 					else if (device->getType() == BLE_AC_SCENE_SCREEN_TOUCH || device->getType() == BLE_SWITCH_KNOB)
// 					{
// 						result = ConfigSceneForScreenTouch(device, propertiesJson, sceneJson, false);
// 					}
// 				}
// 			}
// 			respValue["params"]["code"] = result;
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_DEVICE;
// 		}
// 	}
// 	return CODE_OK;
// }

// int Gateway::OnScenesSync(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnScenesSync");

// 	std::unordered_set<std::string> syncSceneIds;

// 	if (reqValue.isArray())
// 	{
// 		for (const auto &value : reqValue)
// 		{
// 			if (value.isObject() &&
// 					value.isMember("id") && value["id"].isString() &&
// 					value.isMember("name") && value["name"].isString() &&
// 					value.isMember("listDevices") && value["listDevices"].isArray())
// 			{
// 				std::string sceneId = value["id"].asString();
// 				syncSceneIds.insert(sceneId);
// 			}
// 		}
// 	}

// 	std::vector<std::string> scenesToDelete;
// 	mtxDelScene.lock();
// 	SceneManager::GetInstance()->ForEach([&](Scene *scene)
// 																			 {
// 		if (scene && syncSceneIds.find(scene->getId()) == syncSceneIds.end())
// 		{
// 			scenesToDelete.push_back(scene->getId());
// 		} });

// 	for (const auto &sceneId : scenesToDelete)
// 	{
// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
// 		if (!scene)
// 			continue;

// 		for (auto &devInScene : scene->deviceList)
// 		{
// 			if (devInScene && devInScene->device)
// 			{
// 				devInScene->device->RemoveFromScene(scene->getAddr(), devInScene->data);
// 				Database::GetInstance()->DeviceInSceneDel(scene, devInScene->device);
// 			}
// 		}

// 		// vector<Room *> listRoom;
// 		// RoomManager::GetInstance()->ForEach([&](Room *room)
// 		// 																		{ listRoom.push_back(room); });
// 		// for (auto &room : listRoom)
// 		// {
// 		// 	if (room && room->isSceneInRoom(scene))
// 		// 	{
// 		// 		room->SceneDelDevice(scene);
// 		// 	}
// 		// }

// 		Database::GetInstance()->SceneDel(scene->getId());
// 		SceneManager::GetInstance()->SceneDelDevice(scene);
// 	}
// 	mtxDelScene.unlock();

// 	return CODE_NOT_RESPONSE;
// }