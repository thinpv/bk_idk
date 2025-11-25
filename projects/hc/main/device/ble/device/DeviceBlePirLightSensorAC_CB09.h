#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleLightSensor.h"
#include "module/ModuleTimeActionPir.h"
#include "module/ModuleModeActionPir.h"
#include "module/ModulePirLight.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleSensiPir.h"
#include "module/ModulePirLightSensorStartup.h"

using namespace std;

class DeviceBlePirLightSensorAC_CB09 : public DeviceBle
{
private:
	ModulePirLight *modulePirLight;
	ModulePirSensor *modulePirSensor;
	ModuleLightSensor *moduleLightSensor;
	ModuleTimeActionPir *moduleTimeActionPir;
	ModuleModeActionPir *moduleModeActionPir;
	ModuleSensiPir *moduleSensiPir;
	ModuleOnOff *moduleOnOff;
	ModulePirLightSensorStartup *modulePirLightSensorStartup;

public:
	DeviceBlePirLightSensorAC_CB09(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
