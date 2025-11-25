#include "RuleInput.h"
#include "RuleManager.h"

void RuleInput::Trigger(bool value)
{
	// LOGD("Trigger");
	isAvailable = value;
	if (isAvailable && rule)
	{
		RuleManager::GetInstance()->CheckRule(rule);
	}
}