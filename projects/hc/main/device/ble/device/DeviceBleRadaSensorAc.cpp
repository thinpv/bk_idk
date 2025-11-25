
#include "DeviceBleRadaSensorAc.h"

DeviceBleRadaSensorAc::DeviceBleRadaSensorAc(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
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
	moduleModeActionPir = new ModuleModeActionPir(this, addr);
	modules.push_back(moduleModeActionPir);
	moduleSensiPir = new ModuleSensiPir(this, addr);
	modules.push_back(moduleSensiPir);
	moduleOnOff = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
	modules.push_back(moduleOnOff);
	moduleDistance = new ModuleDistance(this, addr);
	modules.push_back(moduleDistance);
	modulePirLightSensorStartup = new ModulePirLightSensorStartup(this, addr);
	modules.push_back(modulePirLightSensorStartup);
	
	powerSource = POWER_AC;
}
