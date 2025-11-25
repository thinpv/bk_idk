#include "DeviceBleSocketSwitch.h"

DeviceBleSocketSwitch::DeviceBleSocketSwitch(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, uint8_t countElement)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	this->countElement = countElement;
	for (int i = 0; i < countElement; i++)
	{
		moduleOnOff = new ModuleOnOff(this, addr + i, KEY_ATTRIBUTE_ONOFF, i);
		modules.push_back(moduleOnOff);
		moduleRgb = new ModuleRgb(this, addr + i, i);
		modules.push_back(moduleRgb);
	}
	moduleCountDownSwitch = new ModuleCountDownSwitch(this, addr);
	modules.push_back(moduleCountDownSwitch);
	moduleStatusStartup = new ModuleStatusStartup(this, addr);
	modules.push_back(moduleStatusStartup);

	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
