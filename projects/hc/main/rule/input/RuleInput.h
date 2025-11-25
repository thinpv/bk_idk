#pragma once
#include "json.h"

using namespace std;

class Rule;
class RuleInput
{
protected:
	bool isAvailable;
	Rule *rule;
	Json::Value data;

public:
	virtual ~RuleInput() {}
	Json::Value *getData() { return &data; }
	virtual void Trigger(bool value);
	virtual bool Check() { return isAvailable; }
};