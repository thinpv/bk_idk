#pragma once
#include <vector>
#include <mutex>
#include "RuleInput.h"
#include "json.h"

using namespace std;

class RuleInputRegister
{
protected:
	mutex ruleInputListMtx;

public:
	vector<RuleInput *> ruleInputList;

	void RegisterTrigger(RuleInput *ruleInput);
	void UnregisterTrigger(RuleInput *ruleInput);
	void CheckTrigger();
	virtual bool CheckData(Json::Value &dataValue, bool &rs) { return false; }
};