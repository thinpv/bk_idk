#pragma once

#include "DeviceBle.h"
#include "module/ModuleNotifyScene.h"

using namespace std;

class DeviceBleScreenTouch : public DeviceBle
{
private:
	ModuleNotifyScene *moduleNotifyScene;

public:
	DeviceBleScreenTouch(string id, string name, string mac, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
