#include "DeviceBleSwitchScene6DC.h"

DeviceBleSwitchScene6DC::DeviceBleSwitchScene6DC(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
	}
	moduleBatteryLevel = new ModuleBatteryLevel(this, addr);
	modules.push_back(moduleBatteryLevel);

	powerSource = POWER_BATTERY;
}
