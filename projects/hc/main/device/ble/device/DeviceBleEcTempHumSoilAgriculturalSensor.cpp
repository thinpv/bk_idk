#include "DeviceBleEcTempHumSoilAgriculturalSensor.h"

DeviceBleEcTempHumSoilAgriculturalSensor::DeviceBleEcTempHumSoilAgriculturalSensor(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
    : DeviceBle(id, name, mac, type, addr, version, dataJson)
{
    moduleEcSoil = new ModuleEcSoil(this, addr);
    modules.push_back(moduleEcSoil);
    moduleTempHumSoil = new ModuleTempHumSoil(this, addr);
    modules.push_back(moduleTempHumSoil);
    moduleTimeRspSensor = new ModuleTimeRspSensor(this, addr);
    modules.push_back(moduleTimeRspSensor);

    powerSource = POWER_BATTERY;
}
