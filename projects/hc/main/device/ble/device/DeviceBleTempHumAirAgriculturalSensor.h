#pragma once

#include "DeviceBle.h"
#include "module/ModuleTempHum.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleTempHumAirAgriculturalSensor : public DeviceBle
{
private:
	ModuleTempHum *moduleTempHum;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleTempHumAirAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
