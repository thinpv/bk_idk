#pragma once

#include "DeviceBle.h"
#include "module/ModuleButton.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCallScene.h"

using namespace std;

class DeviceBleSwitchCeiling : public DeviceBle
{
private:
	ModuleCountDownSwitch *moduleCountDownSwitch;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleOnOff *moduleOnOff;
	ModuleCallScene *moduleCallScene;

public:
	DeviceBleSwitchCeiling(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, uint8_t element = 0);
};
