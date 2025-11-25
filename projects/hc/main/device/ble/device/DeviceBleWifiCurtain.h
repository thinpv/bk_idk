#pragma once

#include "DeviceBle.h"
#include "module/ModuleCalibAuto.h"
#include "module/ModuleCurtain.h"
#include "module/ModuleModeWifi.h"

using namespace std;

class DeviceBleWifiCurtain : public DeviceBle
{
private:
	ModuleCalibAuto *moduleCalibAuto;
	ModuleModeWifi *moduleModeWifi;
	ModuleCurtain *moduleCurtain;

public:
	DeviceBleWifiCurtain(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
