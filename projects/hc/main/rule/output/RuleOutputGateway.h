#pragma once

#include "json.h"
#include "RuleOutput.h"

using namespace std;

class RuleOutputGateway : public RuleOutput
{
private:
	Json::Value data;

public:
	RuleOutputGateway(Json::Value &data);
	~RuleOutputGateway();

	void RunOutput();
};