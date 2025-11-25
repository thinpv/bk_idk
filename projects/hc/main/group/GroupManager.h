#pragma once

#include <map>
#include <mutex>
#include <functional>
#include "Group.h"

using namespace std;

class GroupManager
{
private:
	map<string, Group *> groupList;
	mutex groupListMtx;

	GroupManager();

public:
	static GroupManager *GetInstance();
	void ForEach(function<void(Group *)> func);

	Group *GetGroupFromId(string id);
	Group *GetGroupFromAddr(uint16_t addr);
	uint16_t GetNextGroupAddr();
	void AddGroup(Json::Value &sharedValue);
	Group *AddGroup(string id, Json::Value &groupValue, bool isSaveToDB = true);
	void AddGroup(Group *group);
	void DelGroup(Group *group);
	void DelAllGroup();
	void PrintGroup();

	void startCheckGroup();
	void stopCheckGroup();
};