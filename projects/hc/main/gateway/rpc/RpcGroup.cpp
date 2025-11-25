#include "Gateway.h"
#include "Log.h"
#include "Database.h"
#include <unordered_set>

void Gateway::InitRpcGroup()
{
	// OnRpcCallbackRegister("controlGroup", bind(&Gateway::OnControlGroup, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("createGroup", bind(&Gateway::OnCreateGroup, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("addDevToGroup", bind(&Gateway::OnAddDeviceToGroup, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("delDevFromGroup", bind(&Gateway::OnDeleteDeviceFromGroup, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("delGroup", bind(&Gateway::OnDeleteGroup, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("updateGroupName", bind(&Gateway::OnUpdateGroupName, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("getGroupList", bind(&Gateway::OnGetGroupList, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("getDevListInGroup", bind(&Gateway::OnGetDevListInGroup, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("groups", bind(&Gateway::OnGroupsSync, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnControlGroup(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlGroup: %s", reqValue.toString().c_str());
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

// int Gateway::OnCreateGroup(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGI("OnCreateGroup");
// 	if (reqValue.isMember("id") && reqValue["id"].isString() &&
// 			reqValue.isMember("name") && reqValue["name"].isString() &&
// 			reqValue.isMember("devices") && reqValue["devices"].isArray())
// 	{
// 		Json::Value dataRsp;
// 		dataRsp["method"] = "createGroupRsp";
// 		dataRsp["params"]["code"] = CODE_OK;
// 		// CloudProtocol::Publish(topicRsp, dataRsp.toString());

// 		int msgId = CloudProtocol::GetMsgId();
// 		CloudProtocol::SetMsgId(++msgId);
// 		// topicRsp = "v1/devices/me/rpc/request/" + to_string(CloudProtocol::GetMsgId());

// 		Json::Value successList = Json::arrayValue;
// 		Json::Value failedList = Json::arrayValue;
// 		string groupId = reqValue["id"].asString();
// 		string groupName = reqValue["name"].asString();
// 		Json::Value devInGroup = reqValue["devices"];

// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
// 		if (!group)
// 		{
// 			group = new Group(groupId, groupName, GroupManager::GetInstance()->GetNextGroupAddr());
// 			if (group)
// 			{
// 				GroupManager::GetInstance()->AddGroup(group);
// 				Database::GetInstance()->GroupAdd(group);
// 			}
// 		}

// 		if (group)
// 		{
// 			if (reqValue.isMember("room") && reqValue["room"].isString())
// 			{
// 				string roomId = reqValue["room"].asString();
// 				// Room *room = RoomManager::GetInstance()->GetRoomFromId(roomId);
// 				// if (room)
// 				// {
// 				// 	respValue["params"]["room"] = room->getId();
// 				// 	room->AddGroup(group);
// 				// 	Database::GetInstance()->GroupUpdateRoom(group, roomId);
// 				// }
// 			}

// 			for (auto &deviceValue : devInGroup)
// 			{
// 				if (deviceValue.isObject() && deviceValue.isMember("id") && deviceValue["id"].isString() &&
// 						deviceValue.isMember("epId") && deviceValue["epId"].isArray())
// 				{
// 					Json::Value listEpSuccess = Json::arrayValue;
// 					Json::Value listEpFailed = Json::arrayValue;
// 					Json::Value epList = deviceValue["epId"];
// 					string deviceId = deviceValue["id"].asString();
// 					Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 					if (device)
// 					{
// 						for (int i = 0; i < epList.size(); i++)
// 						{
// 							if (epList[i].isInt())
// 							{
// 								// if (device->AddToGroup(group->getAddr(), epList[i].asInt()) == CODE_OK)
// 								// {
// 								// 	group->AddDevice(device, epList[i].asInt());
// 								// 	Database::GetInstance()->DeviceInGroupAdd(group, device, epList[i].asInt());
// 								// 	listEpSuccess.append(epList[i].asInt());
// 								// }
// 								// else
// 								// {
// 								// 	device->RemoveFromGroup(group->getAddr(), epList[i].asInt());
// 								// 	listEpFailed.append(epList[i].asInt());
// 								// }
// 							}
// 						}
// 						if (listEpSuccess.size() > 0)
// 						{
// 							Json::Value resultSuccess = Json::objectValue;
// 							resultSuccess["id"] = deviceId;
// 							resultSuccess["epId"] = listEpSuccess;
// 							successList.append(resultSuccess);
// 						}

// 						if (listEpFailed.size() > 0)
// 						{
// 							Json::Value resultFailed = Json::objectValue;
// 							resultFailed["id"] = deviceId;
// 							resultFailed["epId"] = listEpFailed;
// 							failedList.append(resultFailed);
// 						}
// 					}
// 					else
// 					{
// 						LOGW("device %s not found", deviceId.c_str());
// 					}
// 				}
// 			}

// 			respValue["params"]["code"] = CODE_OK;
// 			respValue["params"]["id"] = group->getId();
// 			respValue["params"]["name"] = group->getName();
// 			respValue["params"]["success"] = successList;
// 			respValue["params"]["failed"] = failedList;
// 			GroupManager::GetInstance()->PrintGroup();
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_GROUP;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "createGroupRspComplete";
// 	return CODE_OK;
// }

// int Gateway::OnAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString() &&
// 			reqValue.isMember("devices") && reqValue["devices"].isArray())
// 	{
// 		Json::Value dataRsp;
// 		dataRsp["method"] = "addDevToGroupRsp";
// 		dataRsp["params"]["code"] = CODE_OK;
// 		// CloudProtocol::Publish(topicRsp, dataRsp.toString());

// 		int msgId = CloudProtocol::GetMsgId();
// 		CloudProtocol::SetMsgId(++msgId);
// 		// topicRsp = "v1/devices/me/rpc/request/" + to_string(CloudProtocol::GetMsgId());

// 		Json::Value successList = Json::arrayValue;
// 		Json::Value failedList = Json::arrayValue;
// 		string groupId = reqValue["id"].asString();
// 		Json::Value devicesValue = reqValue["devices"];
// 		string groupName = groupId;
// 		if (reqValue.isMember("name") && reqValue["name"].isString())
// 			groupName = reqValue["name"].asString();
// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);

// 		if (!group)
// 		{
// 			group = new Group(groupId, groupName, GroupManager::GetInstance()->GetNextGroupAddr());
// 			if (group)
// 			{
// 				GroupManager::GetInstance()->AddGroup(group);
// 				Database::GetInstance()->GroupAdd(group);
// 			}
// 		}
// 		if (group)
// 		{
// 			if (reqValue.isMember("room") && reqValue["room"].isString())
// 			{
// 				string roomId = reqValue["room"].asString();
// 				// Room *room = RoomManager::GetInstance()->GetRoomFromId(roomId);
// 				// if (room)
// 				// {
// 				// 	respValue["params"]["room"] = room->getId();
// 				// 	room->AddGroup(group);
// 				// 	Database::GetInstance()->GroupUpdateRoom(group, roomId);
// 				// }
// 			}

// 			for (auto &deviceValue : devicesValue)
// 			{
// 				if (deviceValue.isObject() && deviceValue.isMember("id") && deviceValue["id"].isString() &&
// 						deviceValue.isMember("epId") && deviceValue["epId"].isArray())
// 				{
// 					Json::Value listEpSuccess = Json::arrayValue;
// 					Json::Value listEpFailed = Json::arrayValue;
// 					Json::Value epList = deviceValue["epId"];
// 					string deviceId = deviceValue["id"].asString();
// 					Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 					if (device)
// 					{
// 						for (int i = 0; i < epList.size(); i++)
// 						{
// 							if (epList[i].isInt())
// 							{
// 								// if (device->AddToGroup(group->getAddr(), epList[i].asInt()) == CODE_OK)
// 								// {
// 								// 	group->AddDevice(device, epList[i].asInt());
// 								// 	Database::GetInstance()->DeviceInGroupAdd(group, device, epList[i].asInt());
// 								// 	listEpSuccess.append(epList[i].asInt());
// 								// }
// 								// else
// 								// {
// 								// 	device->RemoveFromGroup(group->getAddr(), epList[i].asInt());
// 								// 	listEpFailed.append(epList[i].asInt());
// 								// }
// 							}
// 						}
// 						if (listEpSuccess.size() > 0)
// 						{
// 							Json::Value resultSuccess = Json::objectValue;
// 							resultSuccess["id"] = deviceId;
// 							resultSuccess["epId"] = listEpSuccess;
// 							successList.append(resultSuccess);
// 						}

// 						if (listEpFailed.size() > 0)
// 						{
// 							Json::Value resultFailed = Json::objectValue;
// 							resultFailed["id"] = deviceId;
// 							resultFailed["epId"] = listEpFailed;
// 							failedList.append(resultFailed);
// 						}
// 					}
// 					else
// 					{
// 						LOGW("device %s not found", deviceId.c_str());
// 					}
// 				}
// 			}

// 			respValue["params"]["code"] = CODE_OK;
// 			respValue["params"]["id"] = group->getId();
// 			respValue["params"]["name"] = group->getName();
// 			respValue["params"]["success"] = successList;
// 			respValue["params"]["failed"] = failedList;
// 			GroupManager::GetInstance()->PrintGroup();
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_GROUP;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "addDevToGroupRspComplete";
// 	return CODE_OK;
// }

// int Gateway::OnDeleteDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString() &&
// 			reqValue.isMember("devices") && reqValue["devices"].isArray())
// 	{
// 		Json::Value dataRsp;
// 		dataRsp["method"] = "delDevFromGroupRsp";
// 		dataRsp["params"]["code"] = CODE_OK;
// 		// CloudProtocol::Publish(topicRsp, dataRsp.toString());

// 		int msgId = CloudProtocol::GetMsgId();
// 		CloudProtocol::SetMsgId(++msgId);
// 		// topicRsp = "v1/devices/me/rpc/request/" + to_string(CloudProtocol::GetMsgId());

// 		Json::Value successList = Json::arrayValue;
// 		Json::Value failedList = Json::arrayValue;
// 		string groupId = reqValue["id"].asString();
// 		Json::Value devicesValue = reqValue["devices"];
// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
// 		if (group)
// 		{
// 			for (auto &deviceValue : devicesValue)
// 			{
// 				if (deviceValue.isObject() && deviceValue.isMember("id") && deviceValue["id"].isString() &&
// 						deviceValue.isMember("epId") && deviceValue["epId"].isArray())
// 				{

// 					Json::Value listEpSuccess = Json::arrayValue;
// 					Json::Value listEpFailed = Json::arrayValue;
// 					Json::Value epList = deviceValue["epId"];
// 					string deviceId = deviceValue["id"].asString();
// 					Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 					if (device)
// 					{
// 						for (int i = 0; i < epList.size(); i++)
// 						{
// 							// if (device->RemoveFromGroup(group->getAddr(), epList[i].asInt()) == CODE_OK)
// 							// {
// 							// 	group->DelDevice(device, epList[i].asInt());
// 							// 	Database::GetInstance()->DeviceInGroupDel(group, device, epList[i].asInt());
// 							// 	listEpSuccess.append(epList[i].asInt());
// 							// }
// 							// else
// 							// {
// 							// 	listEpFailed.append(epList[i].asInt());
// 							// }
// 						}

// 						if (listEpSuccess.size() > 0)
// 						{
// 							Json::Value resultSuccess = Json::objectValue;
// 							resultSuccess["id"] = deviceId;
// 							resultSuccess["epId"] = listEpSuccess;
// 							successList.append(resultSuccess);
// 						}
// 						if (listEpFailed.size() > 0)
// 						{
// 							Json::Value resultFailed = Json::objectValue;
// 							resultFailed["id"] = deviceId;
// 							resultFailed["epId"] = listEpFailed;
// 							failedList.append(resultFailed);
// 						}
// 					}
// 					else
// 					{
// 						LOGW("device %s not found", deviceId.c_str());
// 					}
// 				}
// 			}

// 			respValue["params"]["code"] = CODE_OK;
// 			respValue["params"]["id"] = group->getId();
// 			respValue["params"]["success"] = successList;
// 			respValue["params"]["failed"] = failedList;
// 			GroupManager::GetInstance()->PrintGroup();
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_GROUP;
// 		}
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "delDevFromGroupRspComplete";
// 	return CODE_OK;
// }

// int Gateway::OnDeleteGroup(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		Json::Value successList = Json::arrayValue;
// 		Json::Value failedList = Json::arrayValue;
// 		string groupId = reqValue["id"].asString();
// 		mtxDelGroup.lock();
// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
// 		if (group)
// 		{
// 			vector<DeviceInGroup *> devsInGroup = group->deviceList;
// 			for (auto &deviceInGroup : devsInGroup)
// 			{
// 				// if (deviceInGroup->device->RemoveFromGroup(group->getAddr(), deviceInGroup->epId) == CODE_OK)
// 				// {
// 				// 	Database::GetInstance()->DeviceInGroupDel(group, deviceInGroup->device, deviceInGroup->epId);
// 				// 	group->DelDevice(deviceInGroup->device, deviceInGroup->epId);

// 				// 	bool found = false;
// 				// 	for (auto &data : successList)
// 				// 	{
// 				// 		if (data["id"].asString() == deviceInGroup->device->getId())
// 				// 		{
// 				// 			data["epId"].append(deviceInGroup->epId);
// 				// 			found = true;
// 				// 			break;
// 				// 		}
// 				// 	}

// 				// 	if (!found)
// 				// 	{
// 				// 		Json::Value newObject;
// 				// 		newObject["id"] = deviceInGroup->device->getId();
// 				// 		newObject["epId"].append(deviceInGroup->epId);
// 				// 		successList.append(newObject);
// 				// 	}
// 				// }
// 				// else
// 				// {
// 				// 	LOGD("delete from group deviceId %s error", deviceInGroup->device->getId().c_str());
// 				// 	bool found = false;
// 				// 	for (auto &data : failedList)
// 				// 	{
// 				// 		if (data["id"].asString() == deviceInGroup->device->getId())
// 				// 		{
// 				// 			data["epId"].append(deviceInGroup->epId);
// 				// 			found = true;
// 				// 			break;
// 				// 		}
// 				// 	}

// 				// 	if (!found)
// 				// 	{
// 				// 		Json::Value newObject;
// 				// 		newObject["id"] = deviceInGroup->device->getId();
// 				// 		newObject["epId"].append(deviceInGroup->epId);
// 				// 		failedList.append(newObject);
// 				// 	}
// 				// }
// 				// SLEEP_MS(100);
// 			}

// 			if (group->deviceList.size() <= 0)
// 			{
// 				// vector<Room *> listRoom;
// 				// RoomManager::GetInstance()->ForEach([&](Room *room)
// 				// 																		{ listRoom.push_back(room); });
// 				// for (auto &room : listRoom)
// 				// {
// 				// 	if (room && room->isGroupInRoom(group))
// 				// 	{
// 				// 		room->DelGroup(group);
// 				// 	}
// 				// }
// 				Database::GetInstance()->GroupDel(group);
// 				GroupManager::GetInstance()->DelGroup(group);
// 			}
// 			GroupManager::GetInstance()->PrintGroup();

// 			respValue["params"]["code"] = CODE_OK;
// 			respValue["params"]["id"] = groupId;
// 			respValue["params"]["success"] = successList;
// 			respValue["params"]["failed"] = failedList;
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_GROUP;
// 		}
// 		mtxDelGroup.unlock();
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "delGroupRsp";
// 	return CODE_OK;
// }

// int Gateway::OnUpdateGroupName(Json::Value &reqValue, Json::Value &respValue)
// {
// 	string idGroup = "";
// 	if (reqValue.isMember("name") && reqValue["name"].isString() &&
// 			reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		idGroup = reqValue["id"].asString();
// 		string name = reqValue["name"].asString();
// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(idGroup);
// 		if (group)
// 		{
// 			group->setName(name);
// 			Database::GetInstance()->GroupUpdateName(group);
// 		}
// 		// Room *room = RoomManager::GetInstance()->GetRoomFromId(idGroup);
// 		// if (room)
// 		// {
// 		// 	room->setName(name);
// 		// 	Database::GetInstance()->RoomUpdateName(room);
// 		// }
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["params"]["id"] = idGroup;
// 	respValue["method"] = "updateGroupNameRsp";
// 	return CODE_OK;
// }

// int Gateway::OnGetGroupList(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGW("OnGetGroupList");
// 	// Json::Value groupData = Json::arrayValue;
// 	// groupListMtx.lock();
// 	// for (const auto &[id, group] : groupList)
// 	// {
// 	// 	Json::Value groupValue;
// 	// 	groupValue["id"] = group->getId();
// 	// 	groupValue["name"] = group->getName();
// 	// 	groupData.append(groupValue);
// 	// }
// 	// groupListMtx.unlock();
// 	// respValue["data"]["groups"] = groupData;
// 	// respValue["data"]["code"] = CODE_OK;
// 	// respValue["cmd"] = "getGroupListRsp";
// 	return CODE_OK;
// }

// int Gateway::OnGetDevListInGroup(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnGetDevListInGroup");
// 	if (reqValue.isMember("groups") && reqValue["groups"].isArray() && reqValue["groups"].size() > 0)
// 	{
// 		Json::Value groupList = Json::arrayValue;
// 		Json::Value groups = reqValue["groups"];
// 		for (auto &groupValue : groups)
// 		{
// 			Json::Value groupsData;
// 			if (groupValue.isString())
// 			{
// 				string id = groupValue.asString();
// 				groupsData["id"] = id;
// 				Group *temp = GroupManager::GetInstance()->GetGroupFromId(id);
// 				if (temp)
// 				{
// 					Json::Value temp_devicesList = Json::arrayValue;
// 					vector<string> listDevId;
// 					bool isExist;
// 					for (unsigned int i = 0; i < temp->deviceList.size(); i++)
// 					{
// 						DeviceInGroup *deviceInGroup = temp->deviceList[i];
// 						string deviceId = deviceInGroup->device->getId();
// 						isExist = false;
// 						for (auto &id : listDevId)
// 						{
// 							if (deviceId == id)
// 							{
// 								isExist = true;
// 								break;
// 							}
// 						}

// 						if (!isExist)
// 						{
// 							listDevId.push_back(deviceId);
// 							temp_devicesList.append(deviceId);
// 						}
// 					}
// 					groupsData["devices"] = temp_devicesList;
// 				}
// 				else
// 					respValue["params"]["code"] = CODE_NOT_FOUND_GROUP;
// 			}
// 			else
// 				respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 			groupList.append(groupsData);
// 		}
// 		respValue["params"]["groups"] = groupList;
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "getDevListInGroupRsp";
// 	return CODE_OK;
// }

// int Gateway::OnGroupsSync(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnGroupsSync");

// 	std::unordered_set<std::string> syncGroupIds;

// 	if (reqValue.isArray())
// 	{
// 		for (const auto &value : reqValue)
// 		{
// 			if (value.isObject() &&
// 					value.isMember("id") && value["id"].isString() &&
// 					value.isMember("name") && value["name"].isString() &&
// 					value.isMember("listDevices") && value["listDevices"].isArray())
// 			{
// 				std::string groupId = value["id"].asString();
// 				syncGroupIds.insert(groupId);
// 			}
// 		}
// 	}

// 	std::vector<std::string> groupsToDelete;

// 	mtxDelGroup.lock();
// 	GroupManager::GetInstance()->ForEach([&](Group *group)
// 																			 {
// 		if (group && syncGroupIds.find(group->getId()) == syncGroupIds.end())
// 		{
// 			groupsToDelete.push_back(group->getId());
// 		} });

// 	for (const auto &groupId : groupsToDelete)
// 	{
// 		Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
// 		if (!group)
// 			continue;

// 		// for (auto &devInGroup : group->deviceList)
// 		// {
// 		// 	if (devInGroup && devInGroup->device)
// 		// 	{
// 		// 		devInGroup->device->RemoveFromGroup(group->getAddr(), devInGroup->epId);
// 		// 		Database::GetInstance()->DeviceInGroupDel(group, devInGroup->device, devInGroup->epId);
// 		// 	}
// 		// }

// 		// vector<Room *> listRoom;
// 		// RoomManager::GetInstance()->ForEach([&](Room *room)
// 		// 																		{ listRoom.push_back(room); });
// 		// for (auto &room : listRoom)
// 		// {
// 		// 	if (room && room->isGroupInRoom(group))
// 		// 	{
// 		// 		room->DelGroup(group);
// 		// 	}
// 		// }

// 		Database::GetInstance()->GroupDel(group->getId());
// 		GroupManager::GetInstance()->DelGroup(group);
// 	}
// 	mtxDelGroup.unlock();

// 	return CODE_NOT_RESPONSE;
// }
