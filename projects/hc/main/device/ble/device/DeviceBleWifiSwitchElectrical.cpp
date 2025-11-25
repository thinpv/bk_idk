#include "DeviceBleWifiSwitchElectrical.h"

DeviceBleWifiSwitchElectrical::DeviceBleWifiSwitchElectrical(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, uint8_t countElement)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	this->countElement = countElement;
	for (int i = 0; i < countElement; i++)
	{
		moduleOnOff = new ModuleOnOff(this, addr + i, KEY_ATTRIBUTE_BUTTON, i);
		modules.push_back(moduleOnOff);
		moduleDimonDimoff = new ModuleDimonDimoff(this, addr + i, i);
		modules.push_back(moduleDimonDimoff);
	}
	moduleOnOffAll = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
	modules.push_back(moduleOnOffAll);
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);
	moduleCallScene = new ModuleCallScene(this, addr);
	modules.push_back(moduleCallScene);
	moduleButtonAll = new ModuleButtonAll(this, addr, countElement);
	modules.push_back(moduleButtonAll);

	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
