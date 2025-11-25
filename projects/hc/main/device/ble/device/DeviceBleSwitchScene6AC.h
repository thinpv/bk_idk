#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"

using namespace std;

class DeviceBleSwitchScene6AC : public DeviceBle
{
private:
	ModuleButton *moduleButton[6];

public:
	DeviceBleSwitchScene6AC(string id, string name, string mac, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
