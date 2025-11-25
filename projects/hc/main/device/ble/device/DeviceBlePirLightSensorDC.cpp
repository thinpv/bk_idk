#include "DeviceBlePirLightSensorDC.h"

DeviceBlePirLightSensorDC::DeviceBlePirLightSensorDC(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	modulePirLight = new ModulePirLight(this, addr);
	modules.push_back(modulePirLight);
	modulePirSensor = new ModulePirSensor(this, addr);
	modules.push_back(modulePirSensor);
	moduleLightSensor = new ModuleLightSensor(this, addr);
	modules.push_back(moduleLightSensor);
	moduleBatteryLevel = new ModuleBatteryLevel(this, addr);
	modules.push_back(moduleBatteryLevel);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
	modules.push_back(moduleTimeActionPir);
	moduleSensiPir = new ModuleSensiPir(this, addr);
	modules.push_back(moduleSensiPir);

	powerSource = POWER_BATTERY;
}
