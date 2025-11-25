#include "DeviceBleDoorSensor.h"

DeviceBleDoorSensor::DeviceBleDoorSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	moduleDoorHangOn = new ModuleDoorHangOn(this, addr);
	modules.push_back(moduleDoorHangOn);
	moduleDoorStatus = new ModuleDoorStatus(this, addr);
	modules.push_back(moduleDoorStatus);
	moduleBatteryLevel = new ModuleBatteryLevel(this, addr);
	modules.push_back(moduleBatteryLevel);
	
	powerSource = POWER_BATTERY;
}
