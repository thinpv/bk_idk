#include "DeviceBleSensorTempHum.h"

DeviceBleSensorTempHum::DeviceBleSensorTempHum(string id, string name, string mac, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, BLE_TEMP_HUM_SENSOR, addr, version, dataJson)
{
	moduleTempHum = new ModuleTempHum(this, addr);
	modules.push_back(moduleTempHum);
	moduleBatteryLevel = new ModuleBatteryLevel(this, addr);
	modules.push_back(moduleBatteryLevel);

	powerSource = POWER_BATTERY;
}
