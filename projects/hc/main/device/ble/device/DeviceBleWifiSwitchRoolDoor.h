#pragma once

#include "DeviceBle.h"
#include "module/ModuleCalibAuto.h"
#include "module/ModuleCurtain.h"
#include "module/ModuleCountDownSwitch.h"
#include "module/ModuleLock.h"
#include "module/ModuleModeWifi.h"

using namespace std;

class DeviceBleWifiSwitchRoolDoor : public DeviceBle
{
private:
	ModuleCalibAuto *moduleCalibAuto;
	ModuleCountDownSwitch *moduleCountDownSwitch;
	ModuleLock *moduleLock;
	ModuleModeWifi *moduleModeWifi;
	ModuleCurtain *moduleCurtain;

public:
	DeviceBleWifiSwitchRoolDoor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
