#include "GroupManager.h"
#include "Log.h"
#include "DeviceManager.h"
#include "Database.h"
#include "Gateway.h"

GroupManager::GroupManager()
{
}

GroupManager *GroupManager::GetInstance()
{
    static GroupManager *groupManager = NULL;
    if (!groupManager)
    {
        groupManager = new GroupManager();
    }
    return groupManager;
}

void GroupManager::ForEach(function<void(Group *)> func)
{
    groupListMtx.lock();
    for (const auto &p : groupList)
    {
        const auto &group = p.second;
        func(group);
    }
    groupListMtx.unlock();
}

void GroupManager::AddGroup(Json::Value &sharedValue)
{
    startCheckGroup();
    if (sharedValue.isObject() &&
        sharedValue.isMember("groups") && sharedValue["groups"].isObject())
    {
        Json::Value &groupsValue = sharedValue["groups"];
        if (groupsValue.isObject())
        {
            for (const auto &id : groupsValue.getMemberNames())
            {
                Group *group = AddGroup(id, groupsValue[id]);
                group->setWaitingToCheck(false);
            }
        }
    }
    stopCheckGroup();
}

Group *GroupManager::AddGroup(string id, Json::Value &groupValue, bool isSaveToDB)
{
    LOGD("AddGroup id: %s value: %s", id.c_str(), groupValue.toString().c_str());
    if (groupValue.isObject() &&
        groupValue.isMember("name") && groupValue["name"].isString() &&
        groupValue.isMember("devices") && groupValue["devices"].isArray() &&
        groupValue.isMember("ts") && groupValue["ts"].isUInt64())
    {
        string name = groupValue["name"].asString();
        Json::Value devicesValues = groupValue["devices"];
        uint64_t updatedAt = groupValue["ts"].asUInt64();

        Group *group = GetGroupFromId(id);
        if (group)
        {
            if (group->getUpdatedAt() != updatedAt)
            {
                LOGD("Group %s already exists, updating...", id.c_str());
                group->setName(name);
                group->setUpdatedAt(updatedAt);
                Database::GetInstance()->GroupUpdateName(group);
            }
        }
        else
        {
            int groupAddr = 0;
            if (groupValue.isMember("addr") && groupValue["addr"].isUInt())
            {
                groupAddr = groupValue["addr"].asUInt();
            }
            else
            {
                groupAddr = Gateway::GetInstance()->getNextAndIncreaseGroupAddr();
            }

            LOGD("Creating new group %s addr: %d", id.c_str(), groupAddr);
            group = new Group(id, name, groupAddr, updatedAt);
            if (group)
            {
                AddGroup(group);
                if (isSaveToDB)
                    Database::GetInstance()->GroupAdd(group);
            }
        }

        if (group)
        {
            // Add all devices to group
            group->StartCheckDeviceInGroup();
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

                        DeviceInGroup *deviceInGroup = group->AddDeviceAndConfig(device, epId);
                        if (deviceInGroup)
                        {
                            deviceInGroup->setWaitingToCheck(false);
                        }
                    }
                    else
                    {
                        LOGW("Device %s not found in group", deviceId.c_str());
                    }
                }
                else
                {
                    LOGW("Device value error in group");
                }
            }
            group->StopCheckDeviceInGroup();

            return group;
        }
        else
        {
            LOGE("New group error, check format");
        }
    }
    else
    {
        LOGE("New group error, check format");
    }
    return NULL;
}

void GroupManager::AddGroup(Group *group)
{
    if (group)
    {
        groupListMtx.lock();
        groupList[group->getId()] = group;
        groupListMtx.unlock();
    }
}

Group *GroupManager::GetGroupFromId(string id)
{
    groupListMtx.lock();
    if (groupList.find(id) != groupList.end())
    {
        groupListMtx.unlock();
        return groupList[id];
    }
    groupListMtx.unlock();
    return NULL;
}

Group *GroupManager::GetGroupFromAddr(uint16_t addr)
{
    groupListMtx.lock();
    for (const auto &p : groupList)
    {
        const auto &group = p.second;
        if (group->getAddr() == addr)
        {
            groupListMtx.unlock();
            return group;
        }
    }
    groupListMtx.unlock();
    return NULL;
}

uint16_t GroupManager::GetNextGroupAddr()
{
    uint16_t groupAddr = 1; // start add of normal group
    groupListMtx.lock();
    for (const auto &p : groupList)
    {
        const auto &group = p.second;
        if (group->getAddr() >= groupAddr && group->getAddr() < 4096)
        {
            groupAddr = group->getAddr() + 1;
        }
    }
    groupListMtx.unlock();
    return groupAddr;
}

// Del dev in all group, scenBle, room
void GroupManager::DelGroup(Group *group)
{
    groupListMtx.lock();
    groupList.erase(group->getId());
    groupListMtx.unlock();
    delete group;
}

void GroupManager::DelAllGroup()
{
    groupListMtx.lock();
    for (const auto &p : groupList)
    {
        const auto &group = p.second;
        delete group;
    }
    groupList.clear();
    groupListMtx.unlock();
}

void GroupManager::PrintGroup()
{
    groupListMtx.lock();
    for (const auto &p : groupList)
    {
        const auto &id = p.first;
        const auto &group = p.second;
        LOGD("group: %s", id.c_str());
        for (auto &dev : group->deviceList)
        {
            LOGD("\tdev:%s: 0X%04X", dev->device->getId().c_str(), dev->epId);
        }
    }
    groupListMtx.unlock();
}

// đưa tất cả thiết bị trong nhóm vào trạng thái chờ kiểm tra, nếu không còn thuộc nhóm thì phải xóa khỏi danh sách
void GroupManager::startCheckGroup()
{
    groupListMtx.lock();
    for (const auto &p : groupList)
    {
        const auto &group = p.second;
        group->setWaitingToCheck(true);
    }
    groupListMtx.unlock();
}

// xóa tất cả các thiết bị không còn thuộc nhóm
void GroupManager::stopCheckGroup()
{
    groupListMtx.lock();
    for (auto it = groupList.begin(); it != groupList.end();)
    {
        auto group = it->second;
        if (group->isWaitingToCheck())
        {
            LOGI("Delete group %s", group->getName().c_str());
            Database::GetInstance()->GroupDel(group);
            delete group;
            it = groupList.erase(it);
        }
        else
        {
            ++it;
        }
    }
    groupListMtx.unlock();
}