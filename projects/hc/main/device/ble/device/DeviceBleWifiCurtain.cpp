#include "DeviceBleWifiCurtain.h"

DeviceBleWifiCurtain::DeviceBleWifiCurtain(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
		: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	moduleCalibAuto = new ModuleCalibAuto(this, addr);
	modules.push_back(moduleCalibAuto);
	moduleModeWifi = new ModuleModeWifi(this, addr);
	modules.push_back(moduleModeWifi);
	moduleCurtain = new ModuleCurtain(this, addr);
	modules.push_back(moduleCurtain);

	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
