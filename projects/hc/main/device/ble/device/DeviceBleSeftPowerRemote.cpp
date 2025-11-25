#include "DeviceBleSeftPowerRemote.h"

DeviceBleSeftPowerRemote::DeviceBleSeftPowerRemote(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, Device *parent)
	: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	this->parent = parent;
	moduleButtonSeftPowerRemote = new ModuleButtonSeftPowerRemote(this, addr);
	modules.push_back(moduleButtonSeftPowerRemote);
	
	powerSource = POWER_BATTERY;
}

void DeviceBleSeftPowerRemote::SetParentDev(Device *parent)
{
	this->parent = parent;
}

Device *DeviceBleSeftPowerRemote::GetParent()
{
	return this->parent;
}
