#pragma once

#include "DeviceBle.h"
#include "module/ModuleTempHum.h"
#include "module/ModuleBatteryLevel.h"

using namespace std;

class DeviceBleSensorTempHum : public DeviceBle
{
private:
	ModuleTempHum *moduleTempHum;
	ModuleBatteryLevel *moduleBatteryLevel;

public:
	DeviceBleSensorTempHum(string id, string name, string mac, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
