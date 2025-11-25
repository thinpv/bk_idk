#pragma once

#include "DeviceBle.h"
#include "module/ModuleOxyWater.h"
#include "module/ModuleTempWater.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleOxyTempWaterAgriculturalSensor : public DeviceBle
{
private:
	ModuleOxyWater *moduleOxyWater;
	ModuleTempWater *moduleTempWater;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleOxyTempWaterAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
