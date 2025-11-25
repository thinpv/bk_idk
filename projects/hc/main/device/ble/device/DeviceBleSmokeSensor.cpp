#include "DeviceBleSmokeSensor.h"

DeviceBleSmokeSensor::DeviceBleSmokeSensor(string id, string name, string mac, uint16_t addr, uint16_t version, Json::Value *dataJson)
		: DeviceBle(id, name, mac, BLE_SMOKE_SENSOR, addr, version, dataJson)
{
	moduleSmoke = new ModuleSmoke(this, addr);
	modules.push_back(moduleSmoke);
	
	powerSource = POWER_BATTERY;
}
