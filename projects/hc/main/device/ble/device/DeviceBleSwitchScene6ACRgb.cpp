#include "DeviceBleSwitchScene6ACRgb.h"

DeviceBleSwitchScene6ACRgb::DeviceBleSwitchScene6ACRgb(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	for (int i = 0; i < 6; i++)
	{
		moduleRgb[i] = new ModuleRgb(this, addr, i);
		modules.push_back(moduleRgb[i]);
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
	}
}
