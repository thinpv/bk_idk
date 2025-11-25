#include "DeviceBleSwitchTouch.h"

DeviceBleSwitchTouch::DeviceBleSwitchTouch(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, uint8_t numRelay)
		: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	for (int i = 0; i < numRelay; i++)
	{
		moduleRelaySwitch = new ModuleRelaySwitch(this, addr, i);
		modules.push_back(moduleRelaySwitch);
	}

	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
