#pragma once
#include "json.h"
#include "RuleInputRegister.h"

using namespace std;

class Feature : public RuleInputRegister
{
private:
	Feature();

public:
	static Feature *GetInstance();
	void init();
	bool CheckData(Json::Value &dataValue, bool &rs);
	int Do(Json::Value &dataValue);
};
