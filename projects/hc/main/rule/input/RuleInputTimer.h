#pragma once

#include "RuleInput.h"
#include "Rule.h"
#include <functional>
#include "json.h"

using namespace std;

class RuleInputTimer : public RuleInput
{
private:
	int timer;
	int repeat;
	int timerRegisterIndex;

public:
	RuleInputTimer(Rule *rule, int timer, int repeat);
	~RuleInputTimer();
	bool Check();
};