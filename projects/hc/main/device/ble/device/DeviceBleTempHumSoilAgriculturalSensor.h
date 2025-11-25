#pragma once

#include "DeviceBle.h"
#include "module/ModuleTempHumSoil.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleTempHumSoilAgriculturalSensor : public DeviceBle
{
private:
	ModuleTempHumSoil *moduleTempHumSoil;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleTempHumSoilAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
