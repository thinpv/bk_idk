#pragma once

#include "DeviceBle.h"
#include "module/ModulePhSoil.h"
#include "module/ModuleTimeRspSensor.h"

using namespace std;

class DeviceBlePhSoilAgriculturalSensor : public DeviceBle
{
private:
	ModulePhSoil *modulePhSoil;
	ModuleTimeRspSensor *moduleTimeRspSensor;

public:
	DeviceBlePhSoilAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
