#pragma once

#include "DeviceBle.h"
#include "module/ModulePmSensor.h"
#include "module/ModuleTempHum.h"
#include "module/ModuleBatteryLevel.h"

using namespace std;

class DeviceBleSensorPm : public DeviceBle
{
private:
	ModulePmSensor *modulePmSensor;
	ModuleTempHum *moduleTempHum;

public:
	DeviceBleSensorPm(string id, string name, string mac, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
