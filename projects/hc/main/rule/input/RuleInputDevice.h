#pragma once

#include "RuleInput.h"
#include "Rule.h"
#include <functional>
#include "json.h"

using namespace std;

typedef function<void(bool)> DeviceRuleInputCallbackFunc;

class Device;
class RuleInputDevice : public RuleInput
{
private:
	Device *device;

public:
	RuleInputDevice(Rule *rule, Device *device, Json::Value &data);
	~RuleInputDevice();
	Json::Value *getData();
	void Trigger(bool value);
	void UpdateStatus(bool status);
};
