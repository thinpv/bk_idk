#include "DeviceBlePhTempWaterAgriculturalSensor.h"

DeviceBlePhTempWaterAgriculturalSensor::DeviceBlePhTempWaterAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
    : DeviceBle(id, name, mac, type, addr, version, dataJson)
{
    modulePhWater = new ModulePhWater(this, addr);
    modules.push_back(modulePhWater);
    moduleTempWater = new ModuleTempWater(this, addr);
    modules.push_back(moduleTempWater);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);

    powerSource = POWER_BATTERY;
}
