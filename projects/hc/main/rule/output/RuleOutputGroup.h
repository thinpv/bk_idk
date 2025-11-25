#pragma once

#include "json.h"
#include "Group.h"
#include "RuleOutput.h"

using namespace std;

class RuleOutputGroup : public RuleOutput
{
private:
	Group *group;
	Json::Value data;

public:
	RuleOutputGroup(Group *group, Json::Value &data);
	~RuleOutputGroup();

	void RunOutput();
};
