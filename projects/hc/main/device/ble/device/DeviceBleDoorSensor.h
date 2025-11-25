#pragma once

#include "DeviceBle.h"
#include "module/ModuleDoorHangOn.h"
#include "module/ModuleDoorStatus.h"
#include "module/ModuleBatteryLevel.h"

using namespace std;

class DeviceBleDoorSensor : public DeviceBle
{
private:
	ModuleDoorHangOn *moduleDoorHangOn;
	ModuleDoorStatus *moduleDoorStatus;
	ModuleBatteryLevel *moduleBatteryLevel;

public:
	DeviceBleDoorSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
