#include "DeviceBlePirLightSensorAC.h"

DeviceBlePirLightSensorAC::DeviceBlePirLightSensorAC(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	modulePirLight = new ModulePirLight(this, addr);
	modules.push_back(modulePirLight);
	modulePirSensor = new ModulePirSensor(this, addr);
	modules.push_back(modulePirSensor);
	moduleLightSensor = new ModuleLightSensor(this, addr);
	modules.push_back(moduleLightSensor);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
	modules.push_back(moduleTimeActionPir);
	moduleSensiPir = new ModuleSensiPir(this, addr);
	modules.push_back(moduleSensiPir);

	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
