#include "DeviceBleEcSaliTdsWaterAgriculturalSensor.h"

DeviceBleEcSaliTdsWaterAgriculturalSensor::DeviceBleEcSaliTdsWaterAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
    : DeviceBle(id, name, mac, type, addr, version, dataJson)
{
    moduleEcWater = new ModuleEcWater(this, addr);
    modules.push_back(moduleEcWater);
    moduleTempWater = new ModuleTempWater(this, addr);
    modules.push_back(moduleTempWater);
    moduleEcSaliTds = new ModuleEcSaliTds(this, addr);
    modules.push_back(moduleEcSaliTds);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);

    powerSource = POWER_BATTERY;
}
