#pragma once

#include "DeviceBle.h"
#include "module/ModuleEcWater.h"
#include "module/ModuleTempWater.h"
#include "module/ModuleEcSaliTds.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleEcSaliTdsWaterAgriculturalSensor : public DeviceBle
{
private:
	ModuleEcWater *moduleEcWater;
	ModuleTempWater *moduleTempWater;
	ModuleEcSaliTds *moduleEcSaliTds;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleEcSaliTdsWaterAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
