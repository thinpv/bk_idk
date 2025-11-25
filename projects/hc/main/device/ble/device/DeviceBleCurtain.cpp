#include "DeviceBleCurtain.h"

DeviceBleCurtain::DeviceBleCurtain(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	moduleCurtain = new ModuleCurtain(this, addr);
	modules.push_back(moduleCurtain);
	moduleRgb = new ModuleRgb(this, addr);
	modules.push_back(moduleRgb);
	
	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
