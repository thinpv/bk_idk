#pragma once

#include "DeviceBle.h"
#include "module/ModuleRelaySwitch.h"

using namespace std;

class DeviceBleSwitchTouch : public DeviceBle
{
private:
	ModuleRelaySwitch *moduleRelaySwitch;

public:
	DeviceBleSwitchTouch(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, uint8_t numRelay = 1);
};
