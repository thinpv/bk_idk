#pragma once

#include "json.h"
#include "Device.h"
#include "RuleOutput.h"

using namespace std;

class RuleOutputDelay : public RuleOutput
{
private:
	int delayTime;

public:
	RuleOutputDelay(int delayTime);
	~RuleOutputDelay();

	void RunOutput();
};
