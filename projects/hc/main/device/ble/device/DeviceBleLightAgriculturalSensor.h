#pragma once

#include "DeviceBle.h"
#include "module/ModuleLightSensor.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBleLightAgriculturalSensor : public DeviceBle
{
private:
	ModuleLightSensor *moduleLightSensor;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBleLightAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
