#include "DeviceBleLightOnoffHslModeRGB.h"

DeviceBleLightOnoffHslModeRGB::DeviceBleLightOnoffHslModeRGB(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
		: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	moduleOnOff = new ModuleOnOff(this, addr, KEY_ATTRIBUTE_ONOFF);
	modules.push_back(moduleOnOff);
	moduleModeRgb = new ModuleModeRgb(this, addr);
	modules.push_back(moduleModeRgb);
	moduleHsl = new ModuleHsl(this, addr);
	modules.push_back(moduleHsl);
	moduleCallScene = new ModuleCallScene(this, addr);
	modules.push_back(moduleCallScene);

	countElement = 2;
	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
