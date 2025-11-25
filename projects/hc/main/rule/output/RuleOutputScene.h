#pragma once

#include "json.h"
#include "Scene.h"
#include "RuleOutput.h"

using namespace std;

class RuleOutputScene : public RuleOutput
{
private:
	Scene *scene;
	Json::Value data;

public:
	RuleOutputScene(Scene *scene);
	~RuleOutputScene();

	void RunOutput();
};