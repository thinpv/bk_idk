
#include "DeviceBleLedHightBay.h"

DeviceBleLedHightBay::DeviceBleLedHightBay(string id, string name, string mac, uint32_t type, uint32_t addr, uint16_t version, Json::Value *dataJson)
		: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	modulePirSensor = new ModulePirSensor(this, addr);
	modules.push_back(modulePirSensor);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
	modules.push_back(moduleTimeActionPir);
	moduleModeActionPir = new ModuleModeActionPir(this, addr);
	modules.push_back(moduleModeActionPir);
	moduleSensiPir = new ModuleSensiPir(this, addr);
	modules.push_back(moduleSensiPir);
	moduleDimLevel = new ModuleDimLevel(this, addr);
	modules.push_back(moduleDimLevel);
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	moduleOnOff = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
	modules.push_back(moduleOnOff);
	moduleDim = new ModuleDim(this, addr);
	modules.push_back(moduleDim);

	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
