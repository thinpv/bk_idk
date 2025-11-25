#include "DeviceBleSwitchScene6AC.h"

DeviceBleSwitchScene6AC::DeviceBleSwitchScene6AC(string id, string name, string mac, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, BLE_AC_SCENE_CONTACT, addr, version, dataJson)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
	}
}
