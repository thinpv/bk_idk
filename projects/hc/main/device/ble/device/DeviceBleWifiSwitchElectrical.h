#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleDimonDimoff.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleCallScene.h"
#include "module/ModuleButtonAll.h"

using namespace std;

class DeviceBleWifiSwitchElectrical : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleOnOff *moduleOnOffAll;
	ModuleDimonDimoff *moduleDimonDimoff;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCallScene *moduleCallScene;
	ModuleButtonAll *moduleButtonAll;

public:
	DeviceBleWifiSwitchElectrical(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, uint8_t countElement);
};
