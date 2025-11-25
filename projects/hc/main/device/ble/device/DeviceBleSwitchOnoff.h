#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleModeInput.h"
#include "module/ModuleStatusStartup.h"
#include "module/ModuleCallScene.h"

using namespace std;
class DeviceBleSwitchOnoff : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleModeInput *moduleModeInput;
	ModuleStatusStartup *moduleStatusStartup;
	ModuleCallScene *moduleCallScene;

public:
	DeviceBleSwitchOnoff(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
