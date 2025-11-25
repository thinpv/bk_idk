#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <functional>
#include "Rule.h"

using namespace std;

class RuleManager
{
private:
	map<string, Rule *> ruleList;
	mutex ruleListMtx;
	vector<Rule *> ruleNeedToCheckList;
	mutex ruleNeedToCheckListMtx;

	RuleManager();

public:
	static RuleManager *GetInstance();

	void CheckRuleTask();
	void ForEach(function<void(Rule *)> func);

	void AddRule(Json::Value &sharedValue);
	Rule *AddRule(string id, Json::Value &ruleValue, bool isSaveToDB = true);
	Rule *GetRuleFromId(string id);
	void DelRule(Rule *rule);
	void DelAllRule();

	// Add rule to check
	int CheckRule(Rule *rule);

	void startCheckRule();
	void stopCheckRule();
};