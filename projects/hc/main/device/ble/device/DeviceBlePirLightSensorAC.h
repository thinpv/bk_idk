#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleLightSensor.h"
#include "module/ModuleTimeActionPir.h"
#include "module/ModulePirLight.h"
#include "module/ModuleSensiPir.h"

using namespace std;

class DeviceBlePirLightSensorAC : public DeviceBle
{
private:
	ModulePirLight *modulePirLight;
	ModulePirSensor *modulePirSensor;
	ModuleLightSensor *moduleLightSensor;
	ModuleTimeActionPir *moduleTimeActionPir;
	ModuleSensiPir *moduleSensiPir;

public:
	DeviceBlePirLightSensorAC(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
