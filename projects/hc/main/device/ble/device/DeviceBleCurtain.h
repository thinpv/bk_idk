#pragma once

#include "DeviceBle.h"
#include "module/ModuleCurtain.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleCurtain : public DeviceBle
{
private:
	ModuleCurtain *moduleCurtain;
	ModuleRgb *moduleRgb;

public:
	DeviceBleCurtain(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
