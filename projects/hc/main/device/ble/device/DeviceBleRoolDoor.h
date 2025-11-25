#pragma once

#include "DeviceBle.h"
#include "module/ModuleCurtain.h"
#include "module/ModuleTimeActionPir.h"

using namespace std;

class DeviceBleRoolDoor : public DeviceBle
{
private:
	ModuleCurtain *moduleCurtain;
	ModuleTimeActionPir *moduleTimeAction;

public:
	DeviceBleRoolDoor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
