#include "DeviceBleSensorPm.h"

DeviceBleSensorPm::DeviceBleSensorPm(string id, string name, string mac, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, BLE_PM_SENSOR, addr, version, dataJson)
{
	modulePmSensor = new ModulePmSensor(this, addr);
	modules.push_back(modulePmSensor);
	moduleTempHum = new ModuleTempHum(this, addr);
	modules.push_back(moduleTempHum);

	powerSource = POWER_BATTERY;
}
