#include "RuleInputRegister.h"
#include "Log.h"
#include <algorithm>

void RuleInputRegister::RegisterTrigger(RuleInput *ruleInput)
{
	LOGD("RegisterTrigger");
	ruleInputListMtx.lock();
	ruleInputList.push_back(ruleInput);
	ruleInputListMtx.unlock();
}

void RuleInputRegister::UnregisterTrigger(RuleInput *ruleInput)
{
	LOGD("UnregisterTrigger");
	ruleInputListMtx.lock();
	ruleInputList.erase(remove(ruleInputList.begin(), ruleInputList.end(), ruleInput), ruleInputList.end());
	ruleInputListMtx.unlock();
}

void RuleInputRegister::CheckTrigger()
{
	LOGD("CheckTrigger");
	bool rs;
	ruleInputListMtx.lock();
	for (auto &ruleInput : ruleInputList)
	{
		rs = false;
		if (CheckData(*ruleInput->getData(), rs))
			ruleInput->Trigger(rs);
	}
	ruleInputListMtx.unlock();
}