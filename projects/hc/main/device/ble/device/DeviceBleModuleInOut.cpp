#include "DeviceBleModuleInOut.h"

DeviceBleModuleInOut::DeviceBleModuleInOut(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson, uint8_t countElement, uint8_t numInput)
		: DeviceBle(id, name, mac, type, addr, version, dataJson)
{
	this->countElement = countElement;
	this->numInput = numInput;
	for (int i = 0; i < countElement; i++)
	{
		moduleOnOff = new ModuleOnOff(this, addr + i, KEY_ATTRIBUTE_BUTTON, i);
		modules.push_back(moduleOnOff);
		moduleLinkInOut = new ModuleLinkInOut(this, addr, KEY_ATTRIBUTE_LINK_INOUT, i);
		modules.push_back(moduleLinkInOut);
	}
	moduleCallScene = new ModuleCallScene(this, addr);
	modules.push_back(moduleCallScene);
	moduleDelta = new ModuleDelta(this, addr);
	modules.push_back(moduleDelta);
	moduleADC = new ModuleADC(this, addr);
	modules.push_back(moduleADC);

	for (int i = 0; i < numInput; i++)
	{
		moduleInputModuleInOut = new ModuleInputModuleInOut(this, addr, KEY_ATTRIBUTE_INPUT_MODULE_INOUT, i);
		modules.push_back(moduleInputModuleInOut);
		moduleModeInModuleInOut = new ModuleModeInModuleInOut(this, addr, KEY_ATTRIBUTE_MODE_MODULE_INOUT, i);
		modules.push_back(moduleModeInModuleInOut);
		moduleStatusStartupRelay = new ModuleStatusStartupRelay(this, addr, KEY_ATTRIBUTE_STATUS_STARTUP_RELAY, i);
		modules.push_back(moduleStatusStartupRelay);
	}

	powerSource = POWER_AC;
	canAddToGroup = true;
	canAddToScene = true;
}
