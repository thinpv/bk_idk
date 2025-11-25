#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleLightSensor.h"
#include "module/ModuleTimeActionPir.h"
#include "module/ModuleModeActionPir.h"
#include "module/ModulePirLight.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleSensiPir.h"
#include "module/ModuleDistance.h"
#include "module/ModulePirLightSensorStartup.h"

using namespace std;

class DeviceBleRadaSensorAc : public DeviceBle
{
private:
	ModulePirLight *modulePirLight;
	ModulePirSensor *modulePirSensor;
	ModuleLightSensor *moduleLightSensor;
	ModuleTimeActionPir *moduleTimeActionPir;
	ModuleModeActionPir *moduleModeActionPir;
	ModuleSensiPir *moduleSensiPir;
	ModuleOnOff *moduleOnOff;
	ModuleDistance *moduleDistance;
	ModulePirLightSensorStartup *modulePirLightSensorStartup;

public:
	DeviceBleRadaSensorAc(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
