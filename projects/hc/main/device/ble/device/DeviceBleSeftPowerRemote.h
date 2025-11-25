#pragma once

#include "DeviceBle.h"
#include "module/ModuleButtonSeftPowerRemote.h"

using namespace std;

class DeviceBleSeftPowerRemote : public DeviceBle
{
private:
	Device *parent;
	ModuleButtonSeftPowerRemote *moduleButtonSeftPowerRemote;

public:
	DeviceBleSeftPowerRemote(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, Device *parent);
	void SetParentDev(Device *parent);
	Device *GetParent();
};
