#pragma once

#include "DeviceBle.h"
#include "module/ModuleOnOff.h"
#include "module/ModuleHsl.h"
#include "module/ModuleModeRgb.h"
#include "module/ModuleCallScene.h"

using namespace std;

class DeviceBleLightOnoffHslModeRGB : public DeviceBle
{
private:
	ModuleOnOff *moduleOnOff;
	ModuleModeRgb *moduleModeRgb;
	ModuleHsl *moduleHsl;
	ModuleCallScene *moduleCallScene;

public:
	DeviceBleLightOnoffHslModeRGB(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};
