#include "DeviceBleLightOnoffCctDim.h"

DeviceBleLightOnoffCctDim::DeviceBleLightOnoffCctDim(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
		: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	moduleOnOff = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
	modules.push_back(moduleOnOff);
	moduleDim = new ModuleDim(this, addr);
	modules.push_back(moduleDim);
	moduleCallScene = new ModuleCallScene(this, addr);
	modules.push_back(moduleCallScene);
	moduleCct = new ModuleCct(this, addr + 1);
	modules.push_back(moduleCct);

	countElement = 2;
	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
