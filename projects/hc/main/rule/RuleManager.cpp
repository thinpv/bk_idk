#include "RuleManager.h"
#include "Log.h"
#include "Util.h"
#include "DeviceManager.h"
#include "GroupManager.h"
#include "SceneManager.h"
#include "RuleInputDevice.h"
#include "RuleInputGateway.h"
#include "RuleInputTimer.h"
#include "RuleOutputDelay.h"
#include "RuleOutputDevice.h"
#include "RuleOutputGateway.h"
#include "RuleOutputGroup.h"
#include "RuleOutputScene.h"
#include "Define.h"
#include "Gateway.h"
#include <Database.h>

#ifdef ESP_PLATFORM
#include "app_task.h"
#endif

RuleManager *RuleManager::GetInstance()
{
    static RuleManager *ruleManager = NULL;
    if (!ruleManager)
    {
        ruleManager = new RuleManager();
    }
    return ruleManager;
}

RuleManager::RuleManager()
{
#ifdef ESP_PLATFORM
    LOGI("Free memory: %d bytes, internal: %d bytes", esp_get_free_heap_size(), esp_get_free_internal_heap_size());
    if (!app_new_task([](void *arg)
                      {
		LOGI("checkRuleThread Start");
		RuleManager *ruleManager = (RuleManager *)arg;
		ruleManager->CheckRuleTask();
		vTaskDelete(NULL); },               // Task function
                      "checkRuleThread", // Task name
                      20480,             // Stack size (words, not bytes)
                      this,              // Param
                      5                  // Priority
                      ))
    {
        LOGE("Failed to create checkRuleThread task");
    }
#else
    thread checkRuleThread(bind(&RuleManager::CheckRuleTask, this));
    checkRuleThread.detach();
#endif
}

void RuleManager::ForEach(function<void(Rule *)> func)
{
    ruleListMtx.lock();
    for (const auto &p : ruleList)
    {
        const auto &rule = p.second;
        func(rule);
    }
    ruleListMtx.unlock();
}

void RuleManager::AddRule(Json::Value &sharedValue)
{
    startCheckRule();
    if (sharedValue.isObject() &&
        sharedValue.isMember("rules") && sharedValue["rules"].isObject())
    {
        Json::Value &rulesValue = sharedValue["rules"];
        if (rulesValue.isObject())
        {
            for (const auto &id : rulesValue.getMemberNames())
            {
                Rule *rule = AddRule(id, rulesValue[id]);
                rule->setWaitingToCheck(false);
            }
        }
    }
    stopCheckRule();
}

Rule *RuleManager::AddRule(string id, Json::Value &ruleValue, bool isSaveToDB)
{
    LOGD("AddRule id: %s value: %s", id.c_str(), ruleValue.toString().c_str());
    if (ruleValue.isObject() &&
        ruleValue.isMember("name") && ruleValue["name"].isString() &&
        ruleValue.isMember("ts") && ruleValue["ts"].isUInt64() &&
        ruleValue.isMember("if") && ruleValue["if"].isArray() &&
        ruleValue.isMember("then") && ruleValue["then"].isArray())
    {
        string name = ruleValue["name"].asString();
        uint64_t updatedAt = ruleValue["ts"].asUInt64();
        Json::Value ifValues = ruleValue["if"];
        Json::Value thenValues = ruleValue["then"];
        bool active = true;
        string type = RULE_TYPE_OR;
        uint16_t addr = 0;
        // bool isFullDay = true;
        int repeat = 255;

        if (ruleValue.isMember("active") && ruleValue["active"].isBool())
        {
            active = ruleValue["active"].asBool();
        }

        if (ruleValue.isMember("type") && ruleValue["type"].isString())
        {
            type = ruleValue["type"].asString();
        }

        if (ruleValue.isMember("addr") && ruleValue["addr"].isUInt())
        {
            addr = ruleValue["addr"].asUInt();
        }

        Rule *rule = RuleManager::GetInstance()->GetRuleFromId(id);
        if (rule)
        {
            if (rule->getUpdatedAt() == updatedAt)
                return rule;
            delete rule;
        }
        rule = new Rule(id, name, addr, updatedAt, active, type, repeat, ruleValue);
        if (rule)
        {
            if (isSaveToDB)
                Database::GetInstance()->RuleAdd(rule, ruleValue.toString(), 0);

            // parse input
            for (Json::Value::ArrayIndex i = 0; i < ifValues.size(); i++)
            {
                Json::Value ifValue = ifValues[i];
                if (ifValue.isMember("type") && ifValue["type"].isString())
                {
                    string type = ifValue["type"].asString();
                    if (type == "device")
                    {
                        if (ifValue.isMember("id") && ifValue["id"].isString() &&
                            ifValue.isMember("condition") && ifValue["condition"].isObject())
                        {
                            string deviceId = ifValue["id"].asString();
                            Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
                            if (device)
                            {
                                LOGD("Device %s", device->getName().c_str());
                                Json::Value &condition = ifValue["condition"];
                                RuleInputDevice *ruleInputDevice = new RuleInputDevice(rule, device, condition);
                                rule->AddRuleInput(ruleInputDevice);
                            }
                            // else if (deviceId == Gateway::GetInstance()->getId())
                            // {
                            // 	Json::Value &condition = ifValue["condition"];
                            // 	LOGD("ruleInputGateway %s", condition.toString().c_str());
                            // 	RuleInputGateway *ruleInputGateway = new RuleInputGateway(rule, condition);
                            // 	rule->AddRuleInput(ruleInputGateway);
                            // }
                            else
                            {
                                LOGW("Device not found");
                            }
                        }
                    }
                    else
                    {
                        LOGW("Unknown rule input type: %s", type.c_str());
                    }
                }
            }

            // parse output
            for (Json::Value::ArrayIndex i = 0; i < thenValues.size(); i++)
            {
                Json::Value thenValue = thenValues[i];
                if (thenValue.isMember("type") && thenValue["type"].isString())
                {
                    string type = thenValue["type"].asString();
                    if (type == "device")
                    {
                        if (thenValue.isMember("id") && thenValue["id"].isString() &&
                            thenValue.isMember("action") && thenValue["action"].isObject())
                        {
                            string deviceId = thenValue["id"].asString();
                            Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
                            if (device)
                            {
                                LOGD("Device %s", device->getName().c_str());
                                Json::Value &action = thenValue["action"];
                                RuleOutputDevice *ruleOutputDevice = new RuleOutputDevice(device, action);
                                rule->AddRuleOutput(ruleOutputDevice);
                            }
                            // else if (deviceId == Gateway::GetInstance()->getId())
                            // {
                            // 	Json::Value &action = thenValue["action"];
                            // 	LOGD("ruleOutputGateway %s", action.toString().c_str());
                            // 	RuleOutputGateway *ruleOutputGateway = new RuleOutputGateway(action);
                            // 	rule->AddRuleOutput(ruleOutputGateway);
                            // }
                            else
                            {
                                LOGW("Device not found");
                            }
                        }
                    }
                    else if (type == "group")
                    {
                        if (thenValue.isMember("id") && thenValue["id"].isString() &&
                            thenValue.isMember("action") && thenValue["action"].isObject())
                        {
                            string groupId = thenValue["id"].asString();
                            Group *group = GroupManager::GetInstance()->GetGroupFromId(groupId);
                            if (group)
                            {
                                LOGD("Group %s", group->getName().c_str());
                                Json::Value &action = thenValue["action"];
                                RuleOutputGroup *ruleOutputGroup = new RuleOutputGroup(group, action);
                                rule->AddRuleOutput(ruleOutputGroup);
                            }
                            else
                            {
                                LOGW("Group not found");
                            }
                        }
                    }
                    else if (type == "delay")
                    {
                        if (thenValue.isMember("delay") && thenValue["delay"].isInt())
                        {
                            RuleOutputDelay *ruleOutputDelay = new RuleOutputDelay(thenValue["delay"].asInt());
                            rule->AddRuleOutput(ruleOutputDelay);
                        }
                    }
                    else
                    {
                        LOGW("Unknown rule input type: %s", type.c_str());
                    }
                }
            }
            ruleListMtx.lock();
            ruleList[id] = rule;
            ruleListMtx.unlock();
            return rule;
        }
        else
            LOGW("Rule null");
    }
    else
    {
        LOGE("New rule error, check format");
    }
    return NULL;
}

Rule *RuleManager::GetRuleFromId(string id)
{
    ruleListMtx.lock();
    if (ruleList.find(id) != ruleList.end())
    {
        ruleListMtx.unlock();
        return ruleList[id];
    }
    ruleListMtx.unlock();
    return NULL;
}

// Del dev in all group, scenBle, room
void RuleManager::DelRule(Rule *rule)
{
    ruleListMtx.lock();
    ruleList.erase(rule->getId());
    ruleListMtx.unlock();
    delete rule;
}

void RuleManager::DelAllRule()
{
    ruleListMtx.lock();
    for (const auto &p : ruleList)
    {
        const auto &rule = p.second;
        delete rule;
    }
    ruleList.clear();
    ruleListMtx.unlock();
}

void RuleManager::CheckRuleTask()
{
    LOGI("Start CheckRuleTask");
    Rule *rule = NULL;
    while (1)
    {
        ruleNeedToCheckListMtx.lock();
        while (ruleNeedToCheckList.size())
        {
            rule = ruleNeedToCheckList.back();
            ruleNeedToCheckList.pop_back();
            rule->Check();
        }
        ruleNeedToCheckListMtx.unlock();
        SLEEP_MS(100);
    }
}

int RuleManager::CheckRule(Rule *rule)
{
    LOGI("Add rule id %s to check", rule->getId().c_str());
    int rs = CODE_ERROR;
    ruleNeedToCheckListMtx.lock();
    if (ruleNeedToCheckList.size() < MAX_RULE_IN_QUEUE_TO_CHECK)
    {
        ruleNeedToCheckList.push_back(rule);
        rs = CODE_OK;
    }
    ruleNeedToCheckListMtx.unlock();
    return rs;
}

// đưa tất cả thiết bị trong nhóm vào trạng thái chờ kiểm tra, nếu không còn thuộc nhóm thì phải xóa khỏi danh sách
void RuleManager::startCheckRule()
{
    ruleListMtx.lock();
    for (const auto &p : ruleList)
    {
        const auto &rule = p.second;
        rule->setWaitingToCheck(true);
    }
    ruleListMtx.unlock();
}

// xóa tất cả các thiết bị không còn thuộc nhóm
void RuleManager::stopCheckRule()
{
    ruleListMtx.lock();
    for (auto it = ruleList.begin(); it != ruleList.end();)
    {
        auto rule = it->second;
        if (rule->isWaitingToCheck())
        {
            LOGI("Delete rule %s", rule->getName().c_str());
            Database::GetInstance()->RuleDel(rule);
            delete rule;
            it = ruleList.erase(it);
        }
        else
        {
            ++it;
        }
    }
    ruleListMtx.unlock();
}