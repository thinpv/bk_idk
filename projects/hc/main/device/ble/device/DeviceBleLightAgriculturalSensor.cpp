#include "DeviceBleLightAgriculturalSensor.h"

DeviceBleLightAgriculturalSensor::DeviceBleLightAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
    : DeviceBle(id, name, mac, type, addr, version, dataJson)
{
    moduleLightSensor = new ModuleLightSensor(this, addr);
    modules.push_back(moduleLightSensor);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);
    powerSource = POWER_BATTERY;
}
