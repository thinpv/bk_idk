#include "RuleInputGateway.h"
#include "Feature.h"
#include "RuleManager.h"
#include "Log.h"

RuleInputGateway::RuleInputGateway(Rule *rule, Json::Value &data)
{
	this->rule = rule;
	this->data = data;
	Feature::GetInstance()->CheckData(data, isAvailable);
	Feature::GetInstance()->RegisterTrigger(this);
}

RuleInputGateway::~RuleInputGateway()
{
	LOGI("~RuleInputGateway");
	Feature::GetInstance()->UnregisterTrigger(this);
}

Json::Value *RuleInputGateway::getData()
{
	return &data;
}

void RuleInputGateway::Trigger(bool value)
{
	LOGD("Trigger");
	isAvailable = value;
	if (isAvailable && rule)
	{
		RuleManager::GetInstance()->CheckRule(rule);
	}
}

void RuleInputGateway::UpdateStatus(bool status)
{
	this->isAvailable = status;
}