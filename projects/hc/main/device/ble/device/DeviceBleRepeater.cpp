#include "DeviceBleRepeater.h"

DeviceBleRepeater::DeviceBleRepeater(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	powerSource = POWER_AC;
}