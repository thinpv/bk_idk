#pragma once

#include <map>
#include <mutex>
#include <functional>
#include "Scene.h"

using namespace std;

class SceneManager
{
private:
	map<string, Scene *> sceneList;
	mutex sceneListMtx;

	SceneManager();

public:
	static SceneManager *GetInstance();
	void ForEach(function<void(Scene *)> func);

	Scene *GetSceneFromId(string id);
	Scene *GetSceneFromAddr(uint16_t addr);
	uint16_t GetNextSceneAddr();
	void AddScene(Json::Value &sharedValue);
	Scene *AddScene(string id, Json::Value &sceneValue, bool isSaveToDB = true);
	void AddScene(Scene *scene);
	void DelScene(Scene *scene);
	void DelAllScene();
	void PrintScene();

	void startCheckScene();
	void stopCheckScene();
};