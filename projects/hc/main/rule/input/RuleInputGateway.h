#pragma once

#include "RuleInput.h"
#include "Rule.h"
#include <functional>
#include "json.h"

using namespace std;

class RuleInputGateway : public RuleInput
{
private:
public:
	RuleInputGateway(Rule *rule, Json::Value &data);
	~RuleInputGateway();
	Json::Value *getData();
	void Trigger(bool value);
	void UpdateStatus(bool status);
};
