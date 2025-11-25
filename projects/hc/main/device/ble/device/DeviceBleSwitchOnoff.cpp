#include "DeviceBleSwitchOnoff.h"

DeviceBleSwitchOnoff::DeviceBleSwitchOnoff(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
		: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	moduleOnOff = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
	modules.push_back(moduleOnOff);
	moduleModeInput = new ModuleModeInput(this, addr);
	modules.push_back(moduleModeInput);
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	moduleCallScene = new ModuleCallScene(this, addr);
	modules.push_back(moduleCallScene);

	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
