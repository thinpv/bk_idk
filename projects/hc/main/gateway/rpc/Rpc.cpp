#include "Gateway.h"
#include "Log.h"
#include "Database.h"
#include <algorithm>

void Gateway::InitRpc()
{
	OnRpcCallbackRegister("gateway_device_deleted", bind(&Gateway::OnGatewayDeviceDeleted, this, placeholders::_1, placeholders::_2));
	OnRpcCallbackRegister("control", bind(&Gateway::OnGatewayControl, this, placeholders::_1, placeholders::_2));
	OnRpcCallbackRegister("control_device", bind(&Gateway::OnGatewayControlDevice, this, placeholders::_1, placeholders::_2));
	OnRpcCallbackRegister("control_group", bind(&Gateway::OnGatewayControlGroup, this, placeholders::_1, placeholders::_2));
	OnRpcCallbackRegister("control_scene", bind(&Gateway::OnGatewayControlScene, this, placeholders::_1, placeholders::_2));
	OnRpcCallbackRegister("control_rule", bind(&Gateway::OnGatewayControlRule, this, placeholders::_1, placeholders::_2));

	OnRpcCallbackRegister("getTermInfo", bind(&Gateway::OnGetTermInfo, this, placeholders::_1, placeholders::_2));
	OnRpcCallbackRegister("sendCommand", bind(&Gateway::OnSendCommand, this, placeholders::_1, placeholders::_2));
	OnRpcCallbackRegister("getCommandStatus", bind(&Gateway::OnGetCommandStatus, this, placeholders::_1, placeholders::_2));
	OnRpcCallbackRegister("terminateCommand", bind(&Gateway::OnTerminateCommand, this, placeholders::_1, placeholders::_2));

	// OnRpcCallbackRegister("controlDev", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("controlAllDev", bind(&Gateway::OnControlAllDevice, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("getDevStt", bind(&Gateway::OnGetDeviceStatus, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("getAllDevStt", bind(&Gateway::OnGetAllDeviceStatus, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("getDevList", bind(&Gateway::OnGetDeviceList, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("getCamListInRoom", bind(&Gateway::OnGetCamList, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("getAllCam", bind(&Gateway::OnGetAllCam, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("registerNewDev", bind(&Gateway::OnNewDevice, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("delDev", bind(&Gateway::OnDeleteDevice, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("updateDeviceName", bind(&Gateway::OnUpdateDeviceName, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("createSwitchLink", bind(&Gateway::OnCreateSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("addBtToSwitchLink", bind(&Gateway::OnAddBtToSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("delBtFromSwitchLink", bind(&Gateway::OnDelBtFromSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("delSwitchLink", bind(&Gateway::OnDelSwitchLink, this, placeholders::_1, placeholders::_2));
	// OnRpcCallbackRegister("devices", bind(&Gateway::OnDevicesSync, this, placeholders::_1, placeholders::_2));

	// OnRpcCallbackRegister("controlDevMatter", bind(&Gateway::OnControlDevice, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnGatewayDeviceDeleted(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGatewayDeviceDeleted id: %s", reqValue.toString().c_str());
	if (reqValue.isString())
	{
		string deviceId = reqValue.asString();
		Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
		if (device)
		{
#ifdef CONFIG_ENABLE_BLE
			if (device->getProtocol() == BLE_DEVICE)
			{
				BleProtocol::GetInstance()->ResetDev(device->getAddr());
			}
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
			if (device->getProtocol() == ZIGBEE_DEVICE)
			{
				string mac = device->getMac();
				ZigbeeProtocol::GetInstance()->ResetDev(device->getAddr(), mac, false, false);
			}
#endif
			LOGD("Delete device id %s", deviceId.c_str());

			GroupManager::GetInstance()->ForEach(
					[&](Group *group)
					{
						group->RemoveDevice(device, false);
					});
			SceneManager::GetInstance()->ForEach(
					[&](Scene *scene)
					{
						scene->RemoveDevice(device, false);
					});
			DeviceManager::GetInstance()->RemoveDevice(device);
		}
	}
	return CODE_OK;
}

int Gateway::OnGatewayControl(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGatewayControl: %s", reqValue.toString().c_str());
	int rs = Do(reqValue);
	respValue["params"]["code"] = rs;
	return CODE_OK;
}

int Gateway::OnGatewayControlDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGatewayControlDevice: %s", reqValue.toString().c_str());
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("data") && reqValue["data"].isObject())
	{
		string deviceId = reqValue["id"].asString();
		return OnDeviceControl(deviceId, reqValue["data"], respValue);
	}
	else
	{
		respValue["params"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlGroup %s format error", reqValue.toString().c_str());
	}
	return CODE_OK;
}

int Gateway::OnGatewayControlGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGatewayControlGroup: %s", reqValue.toString().c_str());
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
			reqValue.isMember("data") && reqValue["data"].isObject())
	{
		string groupId = reqValue["id"].asString();
		Json::Value devData = reqValue["data"];
		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
		if (group)
		{
			int rs = CODE_OK;
			if (group->deviceList.size() <= 20)
				rs = group->Do(devData, true);
			else
			{
				rs = group->Do(devData, false);
				Json::Value devicesData = Json::arrayValue;
				for (auto &devInGroup : group->deviceList)
				{
					if (devInGroup->device->isOnline())
					{
						Json::Value deviceData;
						deviceData["id"] = devInGroup->device->getId();
						deviceData["data"] = devData;
						devicesData.append(deviceData);
					}
				}
				Json::Value dataPush;
				dataPush["device"] = devicesData;
				Gateway::GetInstance()->pushDeviceUpdateCloud(dataPush);
			}
			respValue["params"]["code"] = rs;
		}
		else
		{
			respValue["params"]["code"] = CODE_NOT_FOUND;
			LOGW("Group id %s not found", groupId.c_str());
		}
	}
	else
	{
		respValue["params"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlGroup %s format error", reqValue.toString().c_str());
	}
	return CODE_OK;
}

int Gateway::OnGatewayControlScene(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGatewayControlScene: %s", reqValue.toString().c_str());
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string sceneId = reqValue["id"].asString();
		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(sceneId);
		if (scene)
		{
			scene->Do();
			respValue["params"]["code"] = CODE_OK;
		}
		else
		{
			respValue["params"]["code"] = CODE_NOT_FOUND;
			LOGW("Scene id %s not found", sceneId.c_str());
		}
	}
	else
	{
		respValue["params"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlScene %s format error", reqValue.toString().c_str());
	}
	return CODE_OK;
}

int Gateway::OnGatewayControlRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGatewayControlRule: %s", reqValue.toString().c_str());
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string ruleId = reqValue["id"].asString();
		Rule *rule = RuleManager::GetInstance()->GetRuleFromId(ruleId);
		if (rule)
		{
			rule->Do();
			respValue["params"]["code"] = CODE_OK;
		}
		else
		{
			respValue["params"]["code"] = CODE_NOT_FOUND;
			LOGW("Rule id %s not found", ruleId.c_str());
		}
	}
	else
	{
		respValue["params"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnControlRule %s format error", reqValue.toString().c_str());
	}
	return CODE_OK;
}

#ifndef ESP_PLATFORM
int Gateway::OnGetTermInfo(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("OnGetTermInfo: %s", reqValue.toString().c_str());
	return CODE_OK;
}

int Gateway::OnSendCommand(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("OnSendCommand: %s", reqValue.toString().c_str());
	return CODE_OK;
}

int Gateway::OnGetCommandStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("OnGetCommandStatus: %s", reqValue.toString().c_str());
	return CODE_OK;
}

int Gateway::OnTerminateCommand(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("OnTerminateCommand: %s", reqValue.toString().c_str());
	return CODE_OK;
}
#endif

// int Gateway::OnControlDevice(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnControlDevice");
// 	if (reqValue.isObject() &&
// 			reqValue.isMember("id") && reqValue["id"].isString() &&
// 			reqValue.isMember("data") && reqValue["data"].isObject())
// 	{
// 		string deviceId = reqValue["id"].asString();
// 		Json::Value devData = reqValue["data"];
// 		Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 		if (device)
// 		{
// 			int rs = device->Do(devData);
// 			respValue["params"]["code"] = rs;
// 		}
// 		else
// 		{
// 			LOGW("Device id %s not found", deviceId.c_str());
// 			respValue["params"]["code"] = CODE_NOT_FOUND_DEVICE;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 		LOGW("OnControlDevice %s format error", reqValue.toString().c_str());
// 	}
// 	respValue["method"] = "controlDevRsp";
// 	return CODE_OK;
// }

// int Gateway::OnControlAllDevice(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnControlAllDevice");
// 	int rs = CODE_ERROR;
// #ifdef CONFIG_ENABLE_BLE
// 	if (reqValue.isMember(KEY_ATTRIBUTE_ONOFF) && reqValue[KEY_ATTRIBUTE_ONOFF].isInt())
// 	{
// 		int value = reqValue[KEY_ATTRIBUTE_ONOFF].asInt();
// 		BleProtocol::GetInstance()->SetOnOffLight(0xFFFF, value, TRANSITION_DEFAULT, true);
// 	}
// 	if (reqValue.isMember(KEY_ATTRIBUTE_DIM) && reqValue[KEY_ATTRIBUTE_DIM].isInt())
// 	{
// 		int value = reqValue[KEY_ATTRIBUTE_DIM].asInt();
// 		uint16_t dim = (value * 65535) / 100;
// 		BleProtocol::GetInstance()->SetDimmingLight(0xFFFF, dim, TRANSITION_DEFAULT, true);
// 	}
// 	if (reqValue.isMember(KEY_ATTRIBUTE_CCT) && reqValue[KEY_ATTRIBUTE_CCT].isInt())
// 	{
// 		int value = reqValue[KEY_ATTRIBUTE_CCT].asInt();
// 		uint16_t cct = (value * 192) + 800;
// 		BleProtocol::GetInstance()->SetCctLight(0xFFFF, cct, TRANSITION_DEFAULT, true);
// 	}
// 	if (reqValue.isMember(KEY_ATTRIBUTE_HUE) && reqValue[KEY_ATTRIBUTE_HUE].isInt() &&
// 			reqValue.isMember(KEY_ATTRIBUTE_SATURATION) && reqValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
// 			reqValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && reqValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
// 	{
// 		int h = reqValue[KEY_ATTRIBUTE_HUE].asInt();
// 		int s = reqValue[KEY_ATTRIBUTE_SATURATION].asInt();
// 		int l = reqValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
// 		BleProtocol::GetInstance()->SetHSLLight(0xFFFF, h, s, l, TRANSITION_DEFAULT, true);
// 	}
// 	if (reqValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && reqValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
// 	{
// 		int value = reqValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
// 		BleProtocol::GetInstance()->CallModeRgb(0xFFFF, value);
// 	}
// 	rs = CODE_OK;
// #endif
// 	respValue["method"] = "controlAllDevRsp";
// 	respValue["params"]["code"] = rs;
// 	return CODE_OK;
// }

// int Gateway::OnGetDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnGetDeviceStatus");
// 	if (reqValue.isMember("devices") && reqValue["devices"].isArray())
// 	{
// 		Json::Value devicesValueRsp = Json::arrayValue;
// 		Json::Value devicesValue = reqValue["devices"];
// 		for (auto &deviceValue : devicesValue)
// 		{
// 			if (deviceValue.isString())
// 			{
// 				string deviceId = deviceValue.asString();
// 				Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 				if (device)
// 				{
// 					Json::Value deviceValue = Json::objectValue;
// 					if (device->getType() == BLE_PM_SENSOR)
// 					{
// #ifdef CONFIG_ENABLE_BLE
// 						BleProtocol::GetInstance()->UpdateStatusSensorsPm(device->getAddr());
// #endif
// 					}
// 					// else
// 					// {
// 					deviceValue["id"] = device->getId();
// 					Json::Value deviceAttbute = Json::objectValue;
// 					device->BuildTelemetryValue(deviceAttbute);
// 					deviceValue["data"] = deviceAttbute;
// 					devicesValueRsp.append(deviceValue);
// 					// }
// 				}
// 			}
// 		}
// 		respValue["params"]["code"] = CODE_OK;
// 		respValue["params"]["devices"] = devicesValueRsp;
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "getDevSttRsp";
// 	return CODE_OK;
// }

// int Gateway::OnGetAllDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGW("OnGetAllDeviceStatus");
// 	// Json::Value devicesValueRsp = Json::arrayValue;
// 	// deviceListMtx.lock();
// 	// for (const auto &[id, device] : deviceList)
// 	// {
// 	// 	Json::Value deviceValue;
// 	// 	deviceValue["id"] = device->getId();
// 	// 	Json::Value deviceAttbute = Json::objectValue;
// 	// 	device->BuildTelemetryValue(deviceAttbute);
// 	// 	deviceValue["data"] = deviceAttbute;
// 	// 	devicesValueRsp.append(deviceValue);
// 	// }
// 	// deviceListMtx.unlock();
// 	// respValue["data"]["code"] = CODE_OK;
// 	// respValue["data"]["device"] = devicesValueRsp;
// 	// respValue["cmd"] = "deviceUpdate";
// 	return CODE_OK;
// }

// int Gateway::OnGetDeviceList(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGW("OnGetDeviceList");
// 	vector<Device *> idDevCurrentList;
// 	DeviceManager::GetInstance()->ForEach([&](Device *device)
// 																				{ idDevCurrentList.push_back(device); });

// 	Json::Value devicesValueRsp = Json::arrayValue;
// 	for (const auto &device : idDevCurrentList)
// 	{
// 		Json::Value deviceValue;
// 		deviceValue["id"] = device->getId();
// 		deviceValue["addr"] = (Json::UInt)device->getAddr();
// 		deviceValue["type"] = (Json::UInt)device->getType();
// 		deviceValue["mac"] = device->getMac();
// 		deviceValue["ver"] = device->GetVersionStr();
// 		devicesValueRsp.append(deviceValue);
// 	}
// 	respValue["params"]["devices"] = devicesValueRsp;
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "getDevListRsp";
// 	return CODE_OK;
// }

// int Gateway::OnGetCamList(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGW("OnGetCamList");
// 	// Json::Value roomData = Json::arrayValue;
// 	// for (const auto &[id, room] : roomList)
// 	// {
// 	// 	Json::Value temp_devicesList;
// 	// 	temp_devicesList["camList"] = Json::arrayValue;
// 	// 	for (unsigned int i = 0; i < room->deviceList.size(); i++)
// 	// 	{
// 	// 		DeviceInGroup *deviceInRoom = room->deviceList[i];
// 	// 		int type = deviceInRoom->device->getType();
// 	// 		if (type / 10000 == 6)
// 	// 		{
// 	// 			Json::Value deviceValue;
// 	// 			deviceValue["id"] = deviceInRoom->device->getId();
// 	// 			deviceValue["mac"] = deviceInRoom->device->getMac();
// 	// 			deviceValue["name"] = deviceInRoom->device->getName();
// 	// 			deviceValue["data"] = deviceInRoom->device->getData();
// 	// 			temp_devicesList["camList"].append(deviceValue);
// 	// 		}
// 	// 	}
// 	// 	temp_devicesList["id"] = id;
// 	// 	roomData.append(temp_devicesList);
// 	// }
// 	// respValue["data"]["room"] = roomData;
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "getCamListInRoomRsp";
// 	return CODE_OK;
// }

// int Gateway::OnGetAllCam(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGW("OnGetAllCam");
// 	// Json::Value camData = Json::arrayValue;
// 	// for (const auto &[id, device] : deviceList)
// 	// {
// 	// 	int type = device->getType();
// 	// 	if (type / 10000 == 6)
// 	// 	{
// 	// 		Json::Value deviceValue;
// 	// 		deviceValue["id"] = device->getId();
// 	// 		deviceValue["mac"] = device->getMac();
// 	// 		deviceValue["name"] = device->getName();
// 	// 		deviceValue["data"] = device->getData();
// 	// 		camData.append(deviceValue);
// 	// 	}
// 	// }
// 	// respValue["data"]["devices"] = camData;
// 	// respValue["data"]["code"] = CODE_OK;
// 	// respValue["cmd"] = "getAllCamRsp";
// 	return CODE_OK;
// }

// int Gateway::OnNewDevice(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("device") && reqValue["device"].isArray())
// 	{
// 		Json::Value device = reqValue["device"];
// 		for (auto &temp : device)
// 		{
// 			if (temp.isMember("id") && temp["id"].isString() &&
// 					temp.isMember("name") && temp["name"].isString() &&
// 					temp.isMember("mac") && temp["mac"].isString() &&
// 					temp.isMember("type") && temp["type"].isInt() &&
// 					temp.isMember("data") && temp["data"].isObject())
// 			{
// 				string id = temp["id"].asString();
// 				string name = temp["name"].asString();
// 				string mac = temp["mac"].asString();
// 				int type = temp["type"].asInt();
// 				Device *device = DeviceManager::GetInstance()->AddDevice(mac, type, 0, 0, &temp["data"]);
// 				if (device)
// 				{
// 					Database::GetInstance()->DeviceAdd(device);
// 					Gateway::GetInstance()->DeviceAddNew(device);
// 				}
// 			}
// 		}
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "registerNewDevRsp";
// 	return CODE_OK;
// }

// // TODO: delete device from room, group, scene,...
// int Gateway::OnDeleteDevice(Json::Value &reqValue, Json::Value &respValue)
// {
// 	// 	if (reqValue.isMember("device") && reqValue["device"].isArray())
// 	// 	{
// 	// 		Json::Value successList = Json::arrayValue;
// 	// 		Json::Value failedList = Json::arrayValue;
// 	// 		Json::Value devicesValue = reqValue["device"];
// 	// 		for (auto &deviceValue : devicesValue)
// 	// 		{
// 	// 			if (deviceValue.isString())
// 	// 			{
// 	// 				string deviceId = deviceValue.asString();
// 	// 				Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 	// 				if (device)
// 	// 				{
// 	// 					// if (device->getType() == BLE_SEFTPOWER_REMOTE_1 || device->getType() == BLE_SEFTPOWER_REMOTE_2 || device->getType() == BLE_SEFTPOWER_REMOTE_3 || device->getType() == BLE_SEFTPOWER_REMOTE_6)
// 	// 					// {
// 	// 					// 	DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(device);
// 	// 					// 	if (deviceBleSeftPowerRemote)
// 	// 					// 	{
// 	// 					// 		Device *parent = deviceBleSeftPowerRemote->GetParent();
// 	// 					// 		if (parent)
// 	// 					// 		{
// 	// 					// 			Database::GetInstance()->DeviceBleChildDel(deviceBleSeftPowerRemote, parent);
// 	// 					// 			BleProtocol::GetInstance()->ResetSeftPowerRemote(parent->getAddr(), deviceBleSeftPowerRemote->getAddr());
// 	// 					// 		}
// 	// 					// 		else
// 	// 					// 			LOGW("parent device null");
// 	// 					// 	}
// 	// 					// }
// 	// 					// else
// 	// 					// 	BleProtocol::GetInstance()->ResetDev(device->getAddr());
// 	// #ifdef CONFIG_ENABLE_BLE
// 	// 					if (device->getProtocol() == BLE_DEVICE)
// 	// 					{
// 	// 						BleProtocol::GetInstance()->ResetDev(device->getAddr());
// 	// 					}
// 	// #endif
// 	// #ifdef CONFIG_ENABLE_ZIGBEE
// 	// 					if (device->getProtocol() == ZIGBEE_DEVICE)
// 	// 					{
// 	// 						string mac = device->getMac();
// 	// 						ZigbeeProtocol::GetInstance()->ResetDev(device->getAddr(), mac, false, false);
// 	// 					}
// 	// #endif
// 	// 					LOGD("Delete device id %s", deviceId.c_str());
// 	// 					// DeviceMatterDelete(device);

// 	// 					vector<Group *> groupList;
// 	// 					GroupManager::GetInstance()->ForEach([&](Group *group)
// 	// 																							 { groupList.push_back(group); });
// 	// 					for (auto &group : groupList)
// 	// 					{
// 	// 						if (group->RemoveDevice(device) == CODE_OK)
// 	// 							Database::GetInstance()->DeviceInGroupDelDev(device);
// 	// 					}

// 	// 					vector<Room *> roomList;
// 	// 					RoomManager::GetInstance()->ForEach([&](Room *room)
// 	// 																							{ roomList.push_back(room); });
// 	// 					for (auto &room : roomList)
// 	// 					{
// 	// 						if (room->RemoveDevice(device) == CODE_OK)
// 	// 							Database::GetInstance()->DeviceInRoomDelDev(device);
// 	// 					}

// 	// 					vector<Scene *> sceneList;
// 	// 					SceneManager::GetInstance()->ForEach([&](Scene *scene)
// 	// 																							 { sceneList.push_back(scene); });
// 	// 					for (auto &scene : sceneList)
// 	// 					{
// 	// 						if (scene->DelDevice(device) == CODE_OK)
// 	// 							Database::GetInstance()->DeviceInSceneDelDev(device);
// 	// 					}

// 	// 					Database::GetInstance()->DeviceDel(device);
// 	// 					DeviceManager::GetInstance()->DelDevice(device);
// 	// 					successList.append(deviceId);
// 	// 				}
// 	// 				else
// 	// 				{
// 	// 					LOGD("deviceId %s dose not exist", deviceId.c_str());
// 	// 					failedList.append(deviceId);
// 	// 				}
// 	// 			}
// 	// 		}
// 	// 		respValue["params"]["code"] = 0;
// 	// 		respValue["params"]["success"] = successList;
// 	// 		respValue["params"]["failed"] = failedList;
// 	// 	}
// 	// 	else
// 	// 	{
// 	// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	// 	}
// 	respValue["method"] = "delDevRsp";
// 	return CODE_OK;
// }

// int Gateway::OnUpdateDeviceName(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("name") && reqValue["name"].isString() && reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string id = reqValue["id"].asString();
// 		string name = reqValue["name"].asString();
// 		Device *device = DeviceManager::GetInstance()->GetDeviceFromId(id);
// 		if (device)
// 		{
// 			device->setName(name);
// 			Database::GetInstance()->DeviceUpdateNameAndAddr(device);
// 		}
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "updateDeviceNameRsp";
// 	return CODE_OK;
// }

// static int indexBt(string bt)
// {
// 	string a[] = {"bt", "bt2", "bt3", "bt4", "bt5", "bt6"};
// 	for (int i = 0; i < 6; i++)
// 	{
// 		if (a[i] == bt)
// 		{
// 			return i;
// 		}
// 	}
// 	return -1;
// }

// int Gateway::OnCreateSwitchLink(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnCreateSwitchLink");
// 	respValue["method"] = "createSwitchLinkRsp";
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string groupId = reqValue["id"].asString();
// 		// Group *group = GetGroupFromId(groupId);
// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
// 		if (!group)
// 		{
// 			group = new Group(groupId, groupId, GroupManager::GetInstance()->GetNextGroupAddr());
// 			if (group)
// 				GroupManager::GetInstance()->AddGroup(group); // true
// 		}

// 		if (group)
// 		{
// 			Json::Value listBtSuccess = Json::arrayValue;
// 			Json::Value listBtFailure = Json::arrayValue;
// 			Json::Value listSuccess = Json::arrayValue;
// 			Json::Value listFailure = Json::arrayValue;
// 			if (reqValue.isMember("lstBt") && reqValue["lstBt"].isArray())
// 			{
// 				Json::Value lstBt = reqValue["lstBt"];
// 				for (auto &btn : lstBt)
// 				{
// 					if (btn.isObject() && btn.isMember("id") && btn["id"].isString() && btn.isMember("bt") && btn["bt"].isArray())
// 					{
// 						string devId = btn["id"].asString();
// 						Device *device = DeviceManager::GetInstance()->GetDeviceFromId(devId);
// 						if (device)
// 						{
// 							if (device->getType() == BLE_SWITCH_1 ||
// 									device->getType() == BLE_SWITCH_2 ||
// 									device->getType() == BLE_SWITCH_3 ||
// 									device->getType() == BLE_SWITCH_4 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_1 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_2 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_3 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_4 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_1_V2 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_2_V2 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_3_V2 ||
// 									device->getType() == BLE_WIFI_SWITCH_ELECTRICAL_1 ||
// 									device->getType() == BLE_WIFI_SWITCH_ELECTRICAL_2 ||
// 									device->getType() == BLE_WIFI_SWITCH_ELECTRICAL_3 ||
// 									device->getType() == BLE_SWITCH_RGB_1 ||
// 									device->getType() == BLE_SWITCH_RGB_2 ||
// 									device->getType() == BLE_SWITCH_RGB_3 ||
// 									device->getType() == BLE_SWITCH_RGB_4 ||
// 									device->getType() == BLE_SWITCH_RGB_1_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_2_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_3_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_4_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_1_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_1_SQUARE_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_2_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_2_SQUARE_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_3_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_3_SQUARE_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_4_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_4_SQUARE_V2 ||
// 									device->getType() == BLE_WIFI_SWITCH_1 ||
// 									device->getType() == BLE_WIFI_SWITCH_2 ||
// 									device->getType() == BLE_WIFI_SWITCH_3 ||
// 									device->getType() == BLE_WIFI_SWITCH_4 ||
// 									device->getType() == BLE_WIFI_SWITCH_1_SQUARE ||
// 									device->getType() == BLE_WIFI_SWITCH_2_SQUARE ||
// 									device->getType() == BLE_WIFI_SWITCH_3_SQUARE ||
// 									device->getType() == BLE_WIFI_SWITCH_4_SQUARE ||
// 									device->getType() == BLE_SWITCH_KNOB)
// 							{
// 								for (auto &bt : btn["bt"])
// 								{
// 									if (bt.isString())
// 									{
// 										int idxBt = indexBt(bt.asString());
// 										if ((idxBt >= 0) && (idxBt < 6))
// 										{
// 											// TODO: check SetIdCombine
// 											if (device->AddToGroup(device->getAddr() + idxBt, group->getAddr()) == CODE_OK)
// 											{
// 												// if (Database::GetInstance()->DeviceInGroupAdd(group, device, device->getAddr() + idxBt) == CODE_OK)
// 												// {
// 												// 	group->AddDevice(device, device->getAddr() + idxBt);
// 												// 	listBtSuccess.append(bt.asString());
// 												// }
// 												// else
// 												// {
// 												// 	device->RemoveFromGroup(device->getAddr() + idxBt, group->getAddr());
// 												// 	listBtFailure.append(bt.asString());
// 												// }
// 											}
// 											else
// 											{
// 												listBtFailure.append(bt.asString());
// 											}
// 										}
// 									}
// 								}
// 								if (listBtSuccess.size() > 0)
// 								{
// 									Json::Value success;
// 									success["id"] = devId;
// 									success["bt"] = listBtSuccess;
// 									listSuccess.append(success);
// 								}
// 								if (listBtFailure.size() > 0)
// 								{
// 									Json::Value failed;
// 									failed["id"] = devId;
// 									failed["bt"] = listBtFailure;
// 									listFailure.append(failed);
// 								}
// 								respValue["params"]["code"] = CODE_OK;
// 							}
// 							else
// 							{
// 								respValue["params"]["code"] = CODE_ERROR;
// 							}
// 						}
// 						else
// 						{
// 							respValue["params"]["code"] = CODE_NOT_FOUND_DEVICE;
// 						}
// 						respValue["params"]["success"] = listSuccess;
// 						respValue["params"]["failed"] = listFailure;
// 					}
// 					else
// 					{
// 						respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 					}
// 				}
// 			}
// 			else
// 			{
// 				respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 			}
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
// 	return CODE_OK;
// }

// int Gateway::OnAddBtToSwitchLink(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnAddBtSwitchLink");
// 	respValue["method"] = "addBtToSwitchLinkRsp";
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string groupId = reqValue["id"].asString();
// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
// 		if (!group)
// 		{
// 			group = new Group(groupId, groupId, GroupManager::GetInstance()->GetNextGroupAddr());
// 			if (group)
// 				GroupManager::GetInstance()->AddGroup(group); // true
// 		}

// 		if (group)
// 		{
// 			Json::Value listBtSuccess = Json::arrayValue;
// 			Json::Value listBtFailure = Json::arrayValue;
// 			Json::Value listSuccess = Json::arrayValue;
// 			Json::Value listFailure = Json::arrayValue;
// 			if (reqValue.isMember("lstBt") && reqValue["lstBt"].isArray())
// 			{
// 				Json::Value lstBt = reqValue["lstBt"];
// 				for (auto &btn : lstBt)
// 				{
// 					if (btn.isObject() && btn.isMember("id") && btn["id"].isString() && btn.isMember("bt") && btn["bt"].isArray())
// 					{
// 						string devId = btn["id"].asString();
// 						Device *device = DeviceManager::GetInstance()->GetDeviceFromId(devId);
// 						if (device)
// 						{
// 							if (device->getType() == BLE_SWITCH_1 ||
// 									device->getType() == BLE_SWITCH_2 ||
// 									device->getType() == BLE_SWITCH_3 ||
// 									device->getType() == BLE_SWITCH_4 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_1 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_2 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_3 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_4 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_1_V2 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_2_V2 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_3_V2 ||
// 									device->getType() == BLE_WIFI_SWITCH_ELECTRICAL_1 ||
// 									device->getType() == BLE_WIFI_SWITCH_ELECTRICAL_2 ||
// 									device->getType() == BLE_WIFI_SWITCH_ELECTRICAL_3 ||
// 									device->getType() == BLE_SWITCH_RGB_1 ||
// 									device->getType() == BLE_SWITCH_RGB_2 ||
// 									device->getType() == BLE_SWITCH_RGB_3 ||
// 									device->getType() == BLE_SWITCH_RGB_4 ||
// 									device->getType() == BLE_SWITCH_RGB_1_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_2_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_3_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_4_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_1_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_1_SQUARE_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_2_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_2_SQUARE_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_3_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_3_SQUARE_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_4_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_4_SQUARE_V2 ||
// 									device->getType() == BLE_WIFI_SWITCH_1 ||
// 									device->getType() == BLE_WIFI_SWITCH_2 ||
// 									device->getType() == BLE_WIFI_SWITCH_3 ||
// 									device->getType() == BLE_WIFI_SWITCH_4 ||
// 									device->getType() == BLE_WIFI_SWITCH_1_SQUARE ||
// 									device->getType() == BLE_WIFI_SWITCH_2_SQUARE ||
// 									device->getType() == BLE_WIFI_SWITCH_3_SQUARE ||
// 									device->getType() == BLE_WIFI_SWITCH_4_SQUARE ||
// 									device->getType() == BLE_SWITCH_KNOB)
// 							{
// 								for (auto &bt : btn["bt"])
// 								{
// 									if (bt.isString())
// 									{
// 										int idxBt = indexBt(bt.asString());
// 										if ((idxBt >= 0) && (idxBt < 6))
// 										{
// 											// TODO: check SetIdCombine
// 											if (device->AddToGroup(device->getAddr() + idxBt, group->getAddr()) == CODE_OK)
// 											{
// 												// if (Database::GetInstance()->DeviceInGroupAdd(group, device, device->getAddr() + idxBt) == CODE_OK)
// 												// {
// 												// 	group->AddDevice(device, device->getAddr() + idxBt);
// 												// 	listBtSuccess.append(bt.asString());
// 												// }
// 												// else
// 												// {
// 												// 	device->RemoveFromGroup(device->getAddr() + idxBt, group->getAddr());
// 												// 	listBtFailure.append(bt.asString());
// 												// }
// 											}
// 											else
// 											{
// 												listBtFailure.append(bt.asString());
// 											}
// 										}
// 									}
// 								}
// 								if (listBtSuccess.size() > 0)
// 								{
// 									Json::Value success;
// 									success["id"] = devId;
// 									success["bt"] = listBtSuccess;
// 									listSuccess.append(success);
// 								}
// 								if (listBtFailure.size() > 0)
// 								{
// 									Json::Value failed;
// 									failed["id"] = devId;
// 									failed["bt"] = listBtFailure;
// 									listFailure.append(failed);
// 								}
// 								respValue["params"]["code"] = CODE_OK;
// 							}
// 							else
// 							{
// 								respValue["params"]["code"] = CODE_ERROR;
// 							}
// 						}
// 						else
// 						{
// 							respValue["params"]["code"] = CODE_NOT_FOUND_DEVICE;
// 						}
// 						respValue["params"]["success"] = listSuccess;
// 						respValue["params"]["failed"] = listFailure;
// 					}
// 					else
// 					{
// 						respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 					}
// 				}
// 			}
// 			else
// 			{
// 				respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 			}
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
// 	return CODE_OK;
// }

// int Gateway::OnDelBtFromSwitchLink(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("DelBtSwitchLink");
// 	respValue["method"] = "delBtFromSwitchLinkRsp";
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string groupId = reqValue["id"].asString();
// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
// 		if (group)
// 		{
// 			Json::Value listBtSuccess = Json::arrayValue;
// 			Json::Value listBtFailure = Json::arrayValue;
// 			Json::Value listSuccess = Json::arrayValue;
// 			Json::Value listFailure = Json::arrayValue;
// 			if (reqValue.isMember("lstBt") && reqValue["lstBt"].isArray())
// 			{
// 				Json::Value lstBt = reqValue["lstBt"];
// 				for (auto &btn : lstBt)
// 				{
// 					if (btn.isObject() && btn.isMember("id") && btn["id"].isString() && btn.isMember("bt") && btn["bt"].isArray())
// 					{
// 						string devId = btn["id"].asString();
// 						Device *device = DeviceManager::GetInstance()->GetDeviceFromId(devId);
// 						if (device)
// 						{
// 							if (device->getType() == BLE_SWITCH_1 ||
// 									device->getType() == BLE_SWITCH_2 ||
// 									device->getType() == BLE_SWITCH_3 ||
// 									device->getType() == BLE_SWITCH_4 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_1 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_2 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_3 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_4 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_1_V2 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_2_V2 ||
// 									device->getType() == BLE_SWITCH_ELECTRICAL_3_V2 ||
// 									device->getType() == BLE_WIFI_SWITCH_ELECTRICAL_1 ||
// 									device->getType() == BLE_WIFI_SWITCH_ELECTRICAL_2 ||
// 									device->getType() == BLE_WIFI_SWITCH_ELECTRICAL_3 ||
// 									device->getType() == BLE_SWITCH_RGB_1 ||
// 									device->getType() == BLE_SWITCH_RGB_2 ||
// 									device->getType() == BLE_SWITCH_RGB_3 ||
// 									device->getType() == BLE_SWITCH_RGB_4 ||
// 									device->getType() == BLE_SWITCH_RGB_1_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_2_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_3_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_4_SQUARE ||
// 									device->getType() == BLE_SWITCH_RGB_1_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_1_SQUARE_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_2_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_2_SQUARE_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_3_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_3_SQUARE_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_4_V2 ||
// 									device->getType() == BLE_SWITCH_RGB_4_SQUARE_V2 ||
// 									device->getType() == BLE_WIFI_SWITCH_1 ||
// 									device->getType() == BLE_WIFI_SWITCH_2 ||
// 									device->getType() == BLE_WIFI_SWITCH_3 ||
// 									device->getType() == BLE_WIFI_SWITCH_4 ||
// 									device->getType() == BLE_WIFI_SWITCH_1_SQUARE ||
// 									device->getType() == BLE_WIFI_SWITCH_2_SQUARE ||
// 									device->getType() == BLE_WIFI_SWITCH_3_SQUARE ||
// 									device->getType() == BLE_WIFI_SWITCH_4_SQUARE ||
// 									device->getType() == BLE_SWITCH_KNOB)
// 							{
// 								for (auto &bt : btn["bt"])
// 								{
// 									if (bt.isString())
// 									{
// 										int idxBt = indexBt(bt.asString());
// 										if ((idxBt >= 0) && (idxBt < 6))
// 										{
// 											// TODO: check SetIdCombine
// 											if (device->RemoveFromGroup(device->getAddr() + idxBt, group->getAddr()) == CODE_OK)
// 											{
// 												// if (Database::GetInstance()->DeviceInGroupDel(group, device, device->getAddr() + idxBt) == CODE_OK)
// 												// {
// 												// 	group->DelDevice(device, device->getAddr() + idxBt);
// 												// 	listBtSuccess.append(bt.asString());
// 												// }
// 												// else
// 												// {
// 												// 	device->AddToGroup(device->getAddr() + idxBt, group->getAddr());
// 												// 	listBtFailure.append(bt.asString());
// 												// }
// 											}
// 											else
// 											{
// 												listBtFailure.append(bt.asString());
// 											}
// 										}
// 									}
// 								}
// 								if (listBtSuccess.size() > 0)
// 								{
// 									Json::Value success;
// 									success["id"] = devId;
// 									success["bt"] = listBtSuccess;
// 									listSuccess.append(success);
// 								}
// 								if (listBtFailure.size() > 0)
// 								{
// 									Json::Value failed;
// 									failed["id"] = devId;
// 									failed["bt"] = listBtFailure;
// 									listFailure.append(failed);
// 								}
// 								respValue["method"]["code"] = CODE_OK;
// 							}
// 							else
// 							{
// 								respValue["method"]["code"] = CODE_ERROR;
// 							}
// 						}
// 						else
// 						{
// 							respValue["method"]["code"] = CODE_NOT_FOUND_DEVICE;
// 						}
// 						respValue["method"]["success"] = listSuccess;
// 						respValue["method"]["failed"] = listFailure;
// 					}
// 					else
// 					{
// 						respValue["method"]["code"] = CODE_FORMAT_ERROR;
// 					}
// 				}
// 			}
// 			else
// 			{
// 				respValue["method"]["code"] = CODE_FORMAT_ERROR;
// 			}
// 		}
// 		else
// 		{
// 			respValue["method"]["code"] = CODE_NOT_FOUND_GROUP;
// 		}
// 	}
// 	else
// 	{
// 		respValue["method"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	return CODE_OK;
// }

// int Gateway::OnDelSwitchLink(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("Del Switch Link");
// 	respValue["method"] = "delSwitchLinkRsp";
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string groupId = reqValue["id"].asString();
// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
// 		if (group)
// 		{
// 			Json::Value listBtSuccess = Json::arrayValue;
// 			Json::Value listBtFailure = Json::arrayValue;
// 			Json::Value listSuccess = Json::arrayValue;
// 			Json::Value listFailure = Json::arrayValue;
// 			vector<DeviceInGroup *> listDev = group->deviceList;
// 			for (auto &dev : listDev)
// 			{
// 				if (dev->device->RemoveFromGroup(group->getAddr(), dev->epId) == CODE_OK)
// 				{
// 					// if (Database::GetInstance()->DeviceInGroupDel(group, dev->device, dev->epId) == CODE_OK)
// 					// {
// 					// 	group->DelDevice(dev->device, dev->epId);
// 					// 	respValue["params"]["coode"] = CODE_OK;
// 					// }
// 					// else
// 					// {
// 					// 	dev->device->AddToGroup(group->getAddr(), dev->epId);
// 					// 	respValue["params"]["coode"] = CODE_ERROR;
// 					// }
// 				}
// 				else
// 				{
// 					respValue["params"]["coode"] = CODE_ERROR;
// 				}
// 			}
// 			GroupManager::GetInstance()->DelGroup(group);
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_GROUP;
// 		}
// 	}
// 	return CODE_OK;
// }

// int Gateway::OnDevicesSync(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnDevicesSync");
// 	vector<string> idDevList;
// 	if (reqValue.isArray())
// 	{
// 		for (auto &value : reqValue)
// 		{
// 			if (value.isObject() && value.isMember("id"))
// 			{
// 				idDevList.push_back(value["id"].asString());
// 			}
// 		}
// 	}

// 	vector<string> idDevCurrentList;
// 	DeviceManager::GetInstance()->ForEach([&](Device *device)
// 																				{ idDevCurrentList.push_back(device->getId()); });

// 	vector<string> result;
// 	for (const auto &id : idDevCurrentList)
// 	{
// 		if (find(idDevList.begin(), idDevList.end(), id) == idDevList.end())
// 		{
// 			result.push_back(id);
// 		}
// 	}

// 	for (auto &id : result)
// 	{
// 		Device *device = DeviceManager::GetInstance()->GetDeviceFromId(id);
// 		if (device)
// 		{
// 			// TODO: Del device
// 		}
// 	}
// 	return CODE_NOT_RESPONSE;
// }

// int Gateway::OnBridgeGetMatterCommission(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnBridgeGetMatterCommission");

// 	if (reqValue.isMember("qr") && reqValue["qr"].isString() && reqValue.isMember("manual") && reqValue["manual"].isString())
// 	{
// 		// string qrCode = "https://project-chip.github.io/connectedhomeip/qrcode.html?data=" + reqValue["qr"].asString();
// 		string qrCode = reqValue["qr"].asString();
// 		string manualCode = reqValue["manual"].asString();
// 		Json::Value data;
// 		Json::Value params;
// 		data["method"] = "saveLinkConnect";
// 		params["linkQR"] = qrCode;
// 		params["raw"] = manualCode;
// 		data["params"] = params;
// 		this->DeviceRpcRequest(data.toString());
// 		ListDeviceMatterInit();
// 		return CODE_OK;
// 	}
// 	else
// 	{
// 		return CODE_ERROR;
// 	}
// 	return CODE_OK;
// }

// int Gateway::OnBridgeGetMatterFabric(Json::Value &data, Json::Value &respValue)
// {
// 	LOGD("OnBridgeGetMatterFabric");
// 	Json::Value reqValue = data[0];
// 	if (reqValue.isMember("vendor") && reqValue["vendor"].isInt() && reqValue.isMember("fabric") && reqValue["fabric"].isInt())
// 	{
// 		int vendor = reqValue["vendor"].asInt();
// 		int fabric = reqValue["fabric"].asInt();
// 		Json::Value data;
// 		Json::Value params;
// 		data["method"] = "saveMatterInfo";
// 		params["matterInfo"].append(vendor);
// 		data["params"] = params;
// 		this->DeviceRpcRequest(data.toString());
// 		return CODE_OK;
// 	}
// 	else
// 	{
// 		return CODE_ERROR;
// 	}
// 	return CODE_OK;
// }

// int Gateway::OnDelMatterFabric(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnDelMatterFabric");
// 	LOGD("Del Switch Link");
// 	respValue["method"] = "delMatterFabricRsp";
// 	respValue["params"]["code"] = CODE_OK;
// 	return CODE_OK;
// }