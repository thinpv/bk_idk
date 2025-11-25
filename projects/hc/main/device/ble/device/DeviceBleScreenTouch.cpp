#include "DeviceBleScreenTouch.h"

DeviceBleScreenTouch::DeviceBleScreenTouch(string id, string name, string mac, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, BLE_AC_SCENE_SCREEN_TOUCH, addr, version, dataJson)
{
	moduleNotifyScene = new ModuleNotifyScene(this, addr);
	modules.push_back(moduleNotifyScene);
	
	powerSource = POWER_AC;
}
