// #include "Gateway.h"
// #include "Log.h"
// #include "Database.h"
// #include <unordered_set>
// #include <algorithm>

// void Gateway::InitRpcRule()
// {
// 	OnRpcCallbackRegister("createRule", bind(&Gateway::OnCreateRule, this, placeholders::_1, placeholders::_2));
// 	OnRpcCallbackRegister("editRule", bind(&Gateway::OnEditRule, this, placeholders::_1, placeholders::_2));
// 	OnRpcCallbackRegister("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));
// 	OnRpcCallbackRegister("getRuleList", bind(&Gateway::OnGetRuleList, this, placeholders::_1, placeholders::_2));
// 	OnRpcCallbackRegister("getRuleInfo", bind(&Gateway::OnGetRuleInfo, this, placeholders::_1, placeholders::_2));
// 	OnRpcCallbackRegister("activeRule", bind(&Gateway::OnActiveRule, this, placeholders::_1, placeholders::_2));
// 	OnRpcCallbackRegister("actionRule", bind(&Gateway::OnActionRule, this, placeholders::_1, placeholders::_2));
// 	OnRpcCallbackRegister("actionRuleCloud", bind(&Gateway::OnActionRuleCloud, this, placeholders::_1, placeholders::_2));
// 	OnRpcCallbackRegister("rules", bind(&Gateway::OnRulesSync, this, placeholders::_1, placeholders::_2));
// }

// int Gateway::OnGetRuleList(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGW("OnGetRuleList");
// 	// Json::Value ruleData = Json::arrayValue;
// 	// ruleListMtx.lock();
// 	// for (const auto &[id, rule] : ruleList)
// 	// {
// 	// 	Json::Value ruleValue;
// 	// 	ruleValue["id"] = rule->getId();
// 	// 	ruleValue["name"] = rule->getName();
// 	// 	ruleData.append(ruleValue);
// 	// }
// 	// ruleListMtx.unlock();
// 	// respValue["data"]["rules"] = ruleData;
// 	// respValue["data"]["code"] = CODE_OK;
// 	// respValue["cmd"] = "getRuleListRsp";
// 	return CODE_OK;
// }

// int Gateway::OnGetRuleInfo(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnOnGetRuleInfo");
// 	if (reqValue.isMember("rules") && reqValue["rules"].isArray() && reqValue["rules"].size() > 0)
// 	{
// 		Json::Value rules = reqValue["rules"];
// 		Json::Value ruleData = Json::arrayValue;
// 		for (auto &ruleValue : rules)
// 		{
// 			if (ruleValue.isString())
// 			{
// 				string ruleId = ruleValue.asString();
// 				Rule *temp_rule = RuleManager::GetInstance()->GetRuleFromId(ruleId);
// 				if (temp_rule)
// 				{
// 					ruleData.append(temp_rule->GetRuleData());
// 					respValue["params"]["code"] = CODE_OK;
// 				}
// 			}
// 			else
// 				respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 		}
// 		respValue["params"]["rules"] = ruleData;
// 	}
// 	respValue["method"] = "getRuleInfoRsp";
// 	return CODE_OK;
// }

// int Gateway::OnCreateRule(Json::Value &reqValue, Json::Value &respValue)
// {
// 	// Rule *rule = RuleManager::GetInstance()->AddRule(reqValue);
// 	// if (rule)
// 	// {
// 	// 	LOGI("Add Rule %s", rule->getId().c_str());
// 	// 	string ruleStr = reqValue.toString();
// 	// 	ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
// 	// 	Database::GetInstance()->RuleAdd(rule, ruleStr, 0);
// 	// 	Database::GetInstance()->RuleUpdateFirstRun(rule, rule->GetFirstRun());
// 	// 	respValue["params"]["code"] = CODE_OK;
// 	// 	respValue["params"]["id"] = rule->getId();
// 	// 	respValue["params"]["data"] = reqValue;
// 	// }
// 	// else
// 	// {
// 	// 	respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	// }
// 	// respValue["method"] = "createRuleRsp";
// 	return CODE_OK;
// }

// int Gateway::OnEditRule(Json::Value &reqValue, Json::Value &respValue)
// {
// 	int rs = CODE_ERROR;
// 	// if (reqValue.isMember("id") && reqValue["id"].isString())
// 	// {
// 	// 	string ruleId = reqValue["id"].asString();
// 	// 	Rule *rule = RuleManager::GetInstance()->GetRuleFromId(ruleId);
// 	// 	if (!rule)
// 	// 	{
// 	// 		Scene *scene = SceneManager::GetInstance()->GetSceneFromId(ruleId);
// 	// 		if (scene)
// 	// 		{
// 	// 			vector<DeviceInScene *> devicesInScene = scene->deviceList;
// 	// 			for (auto &deviceInScene : devicesInScene)
// 	// 			{
// 	// 				deviceInScene->device->RemoveFromScene(scene->getAddr(), deviceInScene->data);
// 	// 				scene->DelDevice(deviceInScene->device);
// 	// 				Database::GetInstance()->DeviceInSceneDel(scene, deviceInScene->device);
// 	// 			}
// 	// 			SceneManager::GetInstance()->SceneDelDevice(scene);
// 	// 			Database::GetInstance()->SceneDel(scene);
// 	// 		}
// 	// 	}
// 	// 	if (rule)
// 	// 	{
// 	// 		RuleManager::GetInstance()->DelRule(rule);
// 	// 	}
// 	// 	rule = RuleManager::GetInstance()->AddRule(reqValue);
// 	// 	if (rule)
// 	// 	{
// 	// 		LOGI("Edit Rule %s", rule->getId().c_str());
// 	// 		string ruleStr = reqValue.toString();
// 	// 		ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
// 	// 		Database::GetInstance()->RuleAdd(rule, ruleStr, 0);
// 	// 		Database::GetInstance()->RuleUpdateFirstRun(rule, rule->GetFirstRun());
// 	// 		rs = CODE_OK;
// 	// 		respValue["params"]["data"] = reqValue;
// 	// 	}
// 	// 	else
// 	// 	{
// 	// 		rs = CODE_FORMAT_ERROR;
// 	// 	}
// 	// 	respValue["params"]["id"] = ruleId;
// 	// }
// 	// else
// 	// {
// 	// 	rs = CODE_FORMAT_ERROR;
// 	// }
// 	// respValue["params"]["code"] = rs;
// 	// respValue["method"] = "editRuleRsp";
// 	return CODE_OK;
// }

// int Gateway::OnDeleteRule(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string ruleId = reqValue["id"].asString();
// 		mtxDelRule.lock();
// 		Rule *rule = RuleManager::GetInstance()->GetRuleFromId(ruleId);
// 		if (rule)
// 		{
// 			RuleManager::GetInstance()->DelRule(rule);
// 			Database::GetInstance()->RuleDel(ruleId);
// 			respValue["params"]["code"] = CODE_OK;
// 		}
// 		else
// 		{
// 			respValue["params"]["code"] = CODE_NOT_FOUND_RULE;
// 		}
// 		respValue["params"]["id"] = ruleId;
// 		mtxDelRule.unlock();
// 	}
// 	else
// 	{
// 		respValue["params"]["code"] = CODE_FORMAT_ERROR;
// 	}
// 	respValue["method"] = "delRuleRsp";
// 	return CODE_OK;
// }

// int Gateway::OnActiveRule(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString() &&
// 			reqValue.isMember("status") && reqValue["status"].isInt())
// 	{
// 		string id = reqValue["id"].asString();
// 		int status = reqValue["status"].asInt();
// 		Rule *rule = RuleManager::GetInstance()->GetRuleFromId(id);
// 		if (rule)
// 		{
// 			rule->setEnable(status);
// 			Database::GetInstance()->RuleUpdateStatus(rule);
// 		}
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "activeRuleRsp";
// 	return CODE_OK;
// }

// int Gateway::OnActionRule(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string id = reqValue["id"].asString();
// 		Rule *rule = RuleManager::GetInstance()->GetRuleFromId(id);
// 		if (rule)
// 		{
// 			rule->RunOutput();
// 			respValue["data"]["id"] = rule->getId();
// 			Json::Value deviceList = Json::arrayValue;
// 		}
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "actionRuleRsp";
// 	return CODE_OK;
// }

// int Gateway::OnActionRuleCloud(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnActionRuleCloud");
// 	if (reqValue.isMember("output") && reqValue["output"].isArray())
// 	{
// 		Json::Value outputList = reqValue["output"];
// 		for (auto &item : outputList)
// 		{
// 			if (item.isMember("deviceOutput") && item["deviceOutput"].isArray())
// 			{
// 				Json::Value deviceOutputList = item["deviceOutput"];
// 				for (auto &dev : deviceOutputList)
// 				{
// 					if (dev.isObject() && dev.isMember("id") && dev["id"].isString())
// 					{
// 						string idDev = dev["id"].asString();
// 						Device *device = DeviceManager::GetInstance()->GetDeviceFromId(idDev);
// 						if (device)
// 						{
// 							device->DoJsonArray(dev);
// 						}
// 					}
// 				}
// 			}

// 			if (item.isMember("sceneOutput") && item["sceneOutput"].isArray())
// 			{
// 				Json::Value sceneOutputList = item["sceneOutput"];
// 				for (auto &sceneOutput : sceneOutputList)
// 				{
// 					if (sceneOutput.isObject() && sceneOutput.isMember("id") && sceneOutput["id"].isString())
// 					{
// 						string idScene = sceneOutput["id"].asString();
// 						Scene *scene = SceneManager::GetInstance()->GetSceneFromId(idScene);
// 						if (scene)
// 						{
// 							if (sceneOutput.isMember("delay") && sceneOutput["delay"].isInt())
// 							{
// 								int delay = sceneOutput["delay"].asInt();
// 								sleep(delay);
// 							}
// 							scene->Do(false);
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "actionRuleCloudRsp";
// 	return CODE_OK;
// }

// int Gateway::OnRulesSync(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnRulesSync");

// 	std::unordered_set<std::string> syncRuleIds;
// 	if (reqValue.isArray())
// 	{
// 		for (const auto &value : reqValue)
// 		{
// 			if (value.isObject() &&
// 					value.isMember("id") && value["id"].isString() &&
// 					value.isMember("data") && value["data"].isObject())
// 			{
// 				std::string ruleId = value["id"].asString();
// 				syncRuleIds.insert(ruleId);
// 			}
// 		}
// 	}
// 	std::vector<std::string> rulesToDelete;
// 	mtxDelRule.lock();
// 	RuleManager::GetInstance()->ForEach([&](Rule *rule)
// 																			{
// 		if (rule && syncRuleIds.find(rule->getId()) == syncRuleIds.end())
// 		{
// 			rulesToDelete.push_back(rule->getId());
// 		} });
// 	for (const auto &ruleId : rulesToDelete)
// 	{
// 		Database::GetInstance()->RuleDel(ruleId);
// 		Rule *rule = RuleManager::GetInstance()->GetRuleFromId(ruleId);
// 		if (rule)
// 		{
// 			RuleManager::GetInstance()->DelRule(rule);
// 		}
// 	}
// 	mtxDelRule.unlock();
// 	return CODE_NOT_RESPONSE;
// }
