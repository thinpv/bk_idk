#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleRgb.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleStatusStartup.h"

using namespace std;

class DeviceBleSocketSwitch : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleRgb *moduleRgb;
	ModuleCountDownSwitch *moduleCountDownSwitch;
	ModuleStatusStartup *moduleStatusStartup;

public:
	DeviceBleSocketSwitch(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, uint8_t countElement);
};
