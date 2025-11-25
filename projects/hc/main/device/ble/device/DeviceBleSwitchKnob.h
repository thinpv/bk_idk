#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDimonDimoff.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleCallScene.h"
#include "module/ModuleRgb.h"

using namespace std;

class DeviceBleSwitchKnob : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleOnOff *moduleOnOffAll;
	ModuleDimonDimoff *moduleDimonDimoff;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCountDownSwitch *moduleCountDownSwitch;
	ModuleCallScene *moduleCallScene;
	ModuleRgb *moduleRgb;

public:
	DeviceBleSwitchKnob(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, uint8_t countElement);
};
