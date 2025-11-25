#include "DeviceManager.h"
#include "Log.h"
#include "Gateway.h"

#ifdef CONFIG_ENABLE_BLE
#include "DeviceBleSwitchOnoff.h"
#include "DeviceBleLightOnoffCctDim.h"
#include "DeviceBleLightOnoffHslModeRGB.h"
#include "DeviceBleLightOnoffCctDimHslModeRGB.h"
#include "DeviceBleSwitchTouchRgb.h"
#include "DeviceBleSwitchElectrical.h"
#include "DeviceBleSwitchScene6DC.h"
#include "DeviceBleSwitchScene6AC.h"
#include "DeviceBleSwitchScene6ACRgb.h"
#include "DeviceBleSensorTempHum.h"
#include "DeviceBleSensorPm.h"
#include "DeviceBlePirLightSensorDC.h"
#include "DeviceBlePirLightSensorAC.h"
#include "DeviceBlePirLightSensorAC_CB09.h"
#include "DeviceBleRadaSensorAc.h"
#include "DeviceBleSmokeSensor.h"
#include "DeviceBleDoorSensor.h"
#include "DeviceBleScreenTouch.h"
#include "DeviceBleCurtain.h"
#include "DeviceBleRepeater.h"
#include "DeviceBleRoolDoor.h"
#include "DeviceBleSwitchTouch.h"
#include "DeviceBleSwitchCeiling.h"
#include "DeviceBleSeftPowerRemote.h"
#include "DeviceBleWifiSwitchTouch.h"
#include "DeviceBleWifiSwitchElectrical.h"
#include "DeviceBleSocketSwitch.h"
#include "DeviceBleWifiCurtain.h"
#include "DeviceBleWifiSwitchRoolDoor.h"
#include "DeviceBleSwitchKnob.h"
#include "DeviceBleLightAgriculturalSensor.h"
#include "DeviceBleTempHumAirAgriculturalSensor.h"
#include "DeviceBleTempHumSoilAgriculturalSensor.h"
#include "DeviceBlePhSoilAgriculturalSensor.h"
#include "DeviceBleEcTempHumSoilAgriculturalSensor.h"
#include "DeviceBlePhTempWaterAgriculturalSensor.h"
#include "DeviceBleOxyTempWaterAgriculturalSensor.h"
#include "DeviceBleEcSaliTdsWaterAgriculturalSensor.h"
#include "DeviceBleLedHightBay.h"
#include "DeviceBleModuleInOut.h"
#endif

#ifdef CONFIG_ENABLE_MQTT
#include "DeviceMqttAihub.h"
#endif

#ifdef ESP_PLATFORM
#include "Config.h"
#include "Led.h"
#include "esp_spiffs.h"
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#include "DeviceZigbeeLumiSensorMagnet.h"
#include "DeviceZigbeeLumiSensorSwitch.h"
#include "DeviceZigbeeLumiSensorTempHum.h"
#include "DeviceZigbeeLumiSensorWleakAQ1.h"
#include "DeviceZigbeeOnoff.h"

#include "DeviceZigbeeTuyaSensorMagnet.h"
#include "DeviceZigbeeTuyaSensorPir.h"
#include "DeviceZigbeeTuyaSensorHumanPresence.h"

#include "DeviceZigbeeRalSwitch.h"
#include "DeviceZigbeeRalLightCct.h"
#include "DeviceZigbeeRalSensorDoor.h"
#include "DeviceZigbeeRalSensorPirCb10.h"
#endif

#ifdef CONFIG_ENABLE_IR
#include "DeviceIrSamsungTV.h"
#endif

#ifdef CONFIG_ENABLE_MODBUS
#include "DeviceSoilMoisSensor.h"
#include "DeviceModbusRTU_RELAY_01.h"
#include "ModbusProtocol.h"
#endif
#include <Database.h>

DeviceManager *DeviceManager::GetInstance()
{
    static DeviceManager *deviceManager = NULL;
    if (!deviceManager)
    {
        deviceManager = new DeviceManager();
    }
    return deviceManager;
}

DeviceManager::DeviceManager()
{
}

void DeviceManager::ForEach(function<void(Device *)> func)
{
    deviceListMtx.lock();
    for (const auto &p : deviceList)
    {
        const auto &device = p.second;
        func(device);
    }
    deviceListMtx.unlock();
}

Device *DeviceManager::AddDevice(string &mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
{
    string id = "dev_" + Gateway::GetInstance()->getMac() + "_" + mac;
    return AddDevice(id, mac, type, addr, version, dataJson);
}

Device *DeviceManager::AddDevice(string &id, string &mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson)
{
    const char *typeName = Device::ConvertDeviceTypeToName(type);
    if (typeName)
    {
        string name = string(typeName) + "_" + mac;
        LOGI("Add device id: %s, name: %s, mac: %s, addr: 0x%04X, type: %d, verion: %d", id.c_str(), name.c_str(), mac.c_str(), addr, type, version);
        Device *device = DeviceManager::GetInstance()->GetDeviceFromId(id);
        if (device)
        {
#ifdef CONFIG_ENABLE_BLE
            BleProtocol::GetInstance()->ResetDev(device->getAddr());
            // TODO: remove device from room, group,...
            delete device;
#endif
        }
        switch (type)
        {
#ifdef CONFIG_ENABLE_BLE
        case BLE_LED_CHIEU_TRANH:
        case BLE_LED_CHIEU_GUONG:
        case BLE_DEN_BAN:
        case BLE_DOWNLIGHT_SMT:
        case BLE_DOWNLIGHT_COB_GOC_HEP:
        case BLE_DOWNLIGHT_COB_GOC_RONG:
        case BLE_DOWNLIGHT_COB_TRANG_TRI:
        case BLE_LED_FLOOD:
        case BLE_LED_DAY_LINEAR:
        case BLE_LED_OP_TRAN:
        case BLE_LED_OP_TRAN_40W:
        case BLE_LED_OP_TUONG:
        case BLE_LED_OP_TRAN_LOA:
        case BLE_LED_AT39:
        case BLE_LED_AT40:
        case BLE_LED_AT41:
        case BLE_PANEL_TRON:
        case BLE_PANEL_VUONG:
        case BLE_TRACKLIGHT:
        case BLE_LED_THA_TRAN:
        case BLE_LED_TUBE_M16:
        case BLE_LED_RLT03_06W:
        case BLE_LED_RLT02_10W:
        case BLE_LED_RLT02_20W:
        case BLE_LED_RLT01_10W:
        case BLE_LED_TRL08_20W:
        case BLE_LED_TRL08_10W:
        case BLE_LED_RLT03_12W:
            device = new DeviceBleLightOnoffCctDim(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_DOWNLIGHT_RGBCW:
        case BLE_LED_DAY_RGBCW:
        case BLE_LED_BULB:
            device = new DeviceBleLightOnoffCctDimHslModeRGB(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_LED_DAY_RGB:
            device = new DeviceBleLightOnoffHslModeRGB(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_LED_HIGHBAY:
            device = new DeviceBleLedHightBay(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_SWITCH_ONOFF:
        case BLE_SWITCH_ONOFF_V2:
            device = new DeviceBleSwitchOnoff(id, name, mac, type, addr, version, dataJson);
            break;

        case BLE_SWITCH_RGB_SOCKET_1:
        case BLE_SWITCH_RGB_SOCKET_1_V2:
            device = new DeviceBleSocketSwitch(id, name, mac, type, addr, version, dataJson, 1);
            break;
        case BLE_SWITCH_RGB_SOCKET_2:
            device = new DeviceBleSocketSwitch(id, name, mac, type, addr, version, dataJson, 2);
            break;
        case BLE_SWITCH_RGB_1:
        case BLE_SWITCH_RGB_1_SQUARE:
        case BLE_SWITCH_RGB_WATER_HEATER:
        case BLE_SWITCH_RGB_1_V2:
        case BLE_SWITCH_RGB_1_SQUARE_V2:
            device = new DeviceBleSwitchTouchRgb(id, name, mac, type, addr, version, dataJson, 1);
            break;
        case BLE_SWITCH_RGB_2:
        case BLE_SWITCH_RGB_2_SQUARE:
        case BLE_SWITCH_RGB_2_V2:
        case BLE_SWITCH_RGB_2_SQUARE_V2:
            device = new DeviceBleSwitchTouchRgb(id, name, mac, type, addr, version, dataJson, 2);
            break;
        case BLE_SWITCH_RGB_3:
        case BLE_SWITCH_RGB_3_SQUARE:
        case BLE_SWITCH_RGB_3_V2:
        case BLE_SWITCH_RGB_3_SQUARE_V2:
            device = new DeviceBleSwitchTouchRgb(id, name, mac, type, addr, version, dataJson, 3);
            break;
        case BLE_SWITCH_RGB_4:
        case BLE_SWITCH_RGB_4_SQUARE:
        case BLE_SWITCH_RGB_4_V2:
        case BLE_SWITCH_RGB_4_SQUARE_V2:
            device = new DeviceBleSwitchTouchRgb(id, name, mac, type, addr, version, dataJson, 4);
            break;
        case BLE_SWITCH_ELECTRICAL_1:
        case BLE_SWITCH_ELECTRICAL_1_V2:
        case BLE_SWITCH_ELECTRICAL_WATER_HEATER:
            device = new DeviceBleSwitchElectrical(id, name, mac, type, addr, version, dataJson, 1);
            break;
        case BLE_SWITCH_ELECTRICAL_2:
        case BLE_SWITCH_ELECTRICAL_2_V2:
            device = new DeviceBleSwitchElectrical(id, name, mac, type, addr, version, dataJson, 2);
            break;
        case BLE_SWITCH_ELECTRICAL_3:
        case BLE_SWITCH_ELECTRICAL_3_V2:
            device = new DeviceBleSwitchElectrical(id, name, mac, type, addr, version, dataJson, 3);
            break;
        case BLE_SWITCH_ELECTRICAL_4:
            device = new DeviceBleSwitchElectrical(id, name, mac, type, addr, version, dataJson, 4);
            break;
        case BLE_WIFI_SWITCH_ELECTRICAL_1:
            device = new DeviceBleWifiSwitchElectrical(id, name, mac, type, addr, version, dataJson, 1);
            break;
        case BLE_WIFI_SWITCH_ELECTRICAL_2:
            device = new DeviceBleWifiSwitchElectrical(id, name, mac, type, addr, version, dataJson, 2);
            break;
        case BLE_WIFI_SWITCH_ELECTRICAL_3:
            device = new DeviceBleWifiSwitchElectrical(id, name, mac, type, addr, version, dataJson, 3);
            break;
        case BLE_SWITCH_2_CEILING:
            device = new DeviceBleSwitchCeiling(id, name, mac, type, addr, version, dataJson, 2);
            break;
        case BLE_SWITCH_3_CEILING:
            device = new DeviceBleSwitchCeiling(id, name, mac, type, addr, version, dataJson, 3);
            break;
        case BLE_SWITCH_5_CEILING:
            device = new DeviceBleSwitchCeiling(id, name, mac, type, addr, version, dataJson, 5);
            break;
        case BLE_DC_SCENE_CONTACT:
        case BLE_REMOTE_M3:
        case BLE_REMOTE_M3_V2:
        case BLE_REMOTE_M4:
            device = new DeviceBleSwitchScene6DC(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_AC_SCENE_CONTACT:
            device = new DeviceBleSwitchScene6AC(id, name, mac, addr, version, dataJson);
            break;
        case BLE_AC_SCENE_CONTACT_RGB:
        case BLE_AC_SCENE_CONTACT_RGB_SQUARE:
            device = new DeviceBleSwitchScene6ACRgb(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_TEMP_HUM_SENSOR:
            device = new DeviceBleSensorTempHum(id, name, mac, addr, version, dataJson);
            break;
        case BLE_PM_SENSOR:
            device = new DeviceBleSensorPm(id, name, mac, addr, version, dataJson);
            break;
        case BLE_PIR_LIGHT_SENSOR_DC:
        case BLE_PIR_LIGHT_SENSOR_DC_CB10:
        case BLE_PIR_LIGHT_SENSOR_DC_CB09:
            device = new DeviceBlePirLightSensorDC(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_RADA_LIGHT_SENSOR_AC_CB15:
            device = new DeviceBleRadaSensorAc(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_PIR_LIGHT_SENSOR_AC:
            device = new DeviceBlePirLightSensorAC(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_PIR_LIGHT_SENSOR_AC_AMTRAN:
            device = new DeviceBlePirLightSensorAC_CB09(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_SMOKE_SENSOR:
            device = new DeviceBleSmokeSensor(id, name, mac, addr, version, dataJson);
            break;
        case BLE_DOOR_SENSOR:
        case BLE_DOOR_CB16_SENSOR:
            device = new DeviceBleDoorSensor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_AC_SCENE_SCREEN_TOUCH:
            device = new DeviceBleScreenTouch(id, name, mac, addr, version, dataJson);
            break;
        case BLE_SWITCH_CURTAIN:
        case BLE_SWITCH_RGB_CURTAIN:
        case BLE_SWITCH_RGB_CURTAIN_SQUARE:
        case BLE_SWITCH_RGB_CURTAIN_HCN:
        case BLE_SWITCH_RGB_CURTAIN_SQUARE_V2:
            device = new DeviceBleCurtain(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_SWITCH_ROOLING_DOOR:
        case BLE_SWITCH_ROOLING_DOOR_V2:
        case BLE_SWITCH_ROOLING_DOOR_SQUARE:
            device = new DeviceBleRoolDoor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_SWITCH_1:
        case BLE_SWITCH_WATER_HEATER:
            device = new DeviceBleSwitchTouch(id, name, mac, type, addr, version, dataJson, 1);
            break;
        case BLE_SWITCH_2:
            device = new DeviceBleSwitchTouch(id, name, mac, type, addr, version, dataJson, 2);
            break;
        case BLE_SWITCH_3:
            device = new DeviceBleSwitchTouch(id, name, mac, type, addr, version, dataJson, 3);
            break;
        case BLE_SWITCH_4:
            device = new DeviceBleSwitchTouch(id, name, mac, type, addr, version, dataJson, 4);
            break;
        case BLE_REPEATER:
            device = new DeviceBleRepeater(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_WIFI_SWITCH_1:
        case BLE_WIFI_SWITCH_1_SQUARE:
            device = new DeviceBleWifiSwitchTouch(id, name, mac, type, addr, version, dataJson, 1);
            break;
        case BLE_WIFI_SWITCH_2:
        case BLE_WIFI_SWITCH_2_SQUARE:
            device = new DeviceBleWifiSwitchTouch(id, name, mac, type, addr, version, dataJson, 2);
            break;
        case BLE_WIFI_SWITCH_3:
        case BLE_WIFI_SWITCH_3_SQUARE:
            device = new DeviceBleWifiSwitchTouch(id, name, mac, type, addr, version, dataJson, 3);
            break;
        case BLE_WIFI_SWITCH_4:
        case BLE_WIFI_SWITCH_4_SQUARE:
            device = new DeviceBleWifiSwitchTouch(id, name, mac, type, addr, version, dataJson, 4);
            break;
        case BLE_WIFI_SWITCH_ROOLING_DOOR:
        case BLE_WIFI_SWITCH_ROOLING_DOOR_SQUARE:
            device = new DeviceBleWifiSwitchRoolDoor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_WIFI_SWITCH_CURTAIN:
        case BLE_WIFI_SWITCH_CURTAIN_SQUARE:
            device = new DeviceBleWifiCurtain(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_SEFTPOWER_REMOTE_1:
        case BLE_SEFTPOWER_REMOTE_2:
        case BLE_SEFTPOWER_REMOTE_3:
        case BLE_SEFTPOWER_REMOTE_6:
            device = DeviceManager::GetInstance()->GetDeviceFromId(id);
            if (device)
            {
                // DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(device);
                // if (deviceBleSeftPowerRemote)
                // {
                // 	Device *parent = deviceBleSeftPowerRemote->GetParent();
                // 	if (parent)
                // 	{
                // 		Database::GetInstance()->DeviceBleChildDel(deviceBleSeftPowerRemote, parent);
                // 		BleProtocol::GetInstance()->ResetSeftPowerRemote(parent->getAddr(), deviceBleSeftPowerRemote->getAddr());
                // 	}
                // 	else
                // 		LOGW("parent device null");
                // }
                RemoveDevice(device);
            }
            device = new DeviceBleSeftPowerRemote(id, name, mac, type, addr, version, dataJson, NULL);
            break;
        case BLE_SWITCH_KNOB:
            device = new DeviceBleSwitchKnob(id, name, mac, type, addr, version, dataJson, 2);
            break;
        case BLE_TEMP_HUM_AIR_AGRICULTURAL_SENSOR:
            device = new DeviceBleTempHumAirAgriculturalSensor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_LIGHT_SENSOR:
        case BLE_LIGHT_SENSOR_AGRI:
            device = new DeviceBleLightAgriculturalSensor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_TEMP_HUM_SOIL_AGRICULTURAL_SENSOR:
            device = new DeviceBleTempHumSoilAgriculturalSensor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_PH_SOIL_AGRICULTURAL_SENSOR:
            device = new DeviceBlePhSoilAgriculturalSensor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_EC_TEMP_HUM_SOIL_AGRICULTURAL_SENSOR:
            device = new DeviceBleEcTempHumSoilAgriculturalSensor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_PH_WATER_AGRICULTURAL_SENSOR:
            device = new DeviceBlePhTempWaterAgriculturalSensor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_EC_WATER_AGRICULTURAL_SENSOR:
            device = new DeviceBleEcSaliTdsWaterAgriculturalSensor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_OXY_WATER_AGRICULTURAL_SENSOR:
            device = new DeviceBleOxyTempWaterAgriculturalSensor(id, name, mac, type, addr, version, dataJson);
            break;
        case BLE_MODULE_INOUT:
            device = new DeviceBleModuleInOut(id, name, mac, type, addr, version, dataJson, 2, 4);
            break;
#endif

#ifdef CONFIG_ENABLE_MQTT
        case MQTT_AI_HUB:
            device = new DeviceMqttAihub(id, name, mac, version, dataJson);
            break;
        case CAMERA_TUYA:
        case CAMERA_DAHUA:
        case CAMERA_HKVISION:
            device = new Device(id, name, mac, type, addr, version, dataJson);
            break;
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
        case ZIGBEE_LUMI_SENSOR_MAGNET:
            device = new DeviceZigbeeLumiSensorMagnet(id, name, mac, addr, version, dataJson);
            break;
        case ZIGBEE_LUMI_SENSOR_SWITCH:
            device = new DeviceZigbeeLumiSensorSwitch(id, name, mac, addr, version, dataJson);
            break;
        case ZIGBEE_LUMI_SENSOR_TEMP_HUM:
            device = new DeviceZigbeeLumiSensorTempHum(id, name, mac, addr, version, dataJson);
            break;
        case ZIGBEE_LUMI_SENSOR_WLEAK_AQ1:
            device = new DeviceZigbeeLumiSensorWleakAQ1(id, name, mac, addr, version, dataJson);
            break;
        case ZIGBEE_LUMI_PLUG:
            device = new DeviceZigbeeOnoff(id, name, mac, addr, version, dataJson);
            break;
        case ZIGBEE_TUYA_SENSOR_MAGNET_TY0203:
            device = new DeviceZigbeeTuyaSensorMagnet(id, name, mac, addr, version, dataJson);
            break;
        case ZIGBEE_TUYA_SENSOR_PIR_RH3040:
            device = new DeviceZigbeeTuyaSensorPir(id, name, mac, addr, version, dataJson);
            break;
        case ZIGBEE_TUYA_SENSOR_HUMAN_PRESENCE_TS0225:
            device = new DeviceZigbeeTuyaSensorHumanPresence(id, name, mac, addr, version, dataJson);
            break;
        case ZIGBEE_RAL_SWITCH_1:
            device = new DeviceZigbeeRalSwitch(id, name, mac, type, addr, version, dataJson, 1);
            break;
        case ZIGBEE_RAL_SWITCH_3:
            device = new DeviceZigbeeRalSwitch(id, name, mac, type, addr, version, dataJson, 3);
            break;
        case ZIGBEE_RAL_SWITCH_5:
            device = new DeviceZigbeeRalSwitch(id, name, mac, type, addr, version, dataJson, 5);
            break;
        case ZIGBEE_RAL_LIGHT_CCT_DIM:
            device = new DeviceZigbeeRalLightCct(id, name, mac, addr, version, dataJson);
            break;
        case ZIGBEE_RAL_SENSOR_DOOR:
            device = new DeviceZigbeeRalSensorDoor(id, name, mac, type, addr, version, dataJson);
            break;
        case ZIGBEE_RAL_SENSOR_PIR_LIGHT_CB10:
            device = new DeviceZigbeeRalSensorPirCb10(id, name, mac, type, addr, version, dataJson);
            break;

#endif

#ifdef CONFIG_ENABLE_IR
        case IR_SAMSUNG_TV:
            device = new DeviceIrSamsungTV(id, name, dataJson);
            break;
#endif

#ifdef CONFIG_ENABLE_MODBUS
        case MODBUS_SOIL_MOIS_SENSOR:
            device = new DeviceSoilMoisSensor(id, name, "mac-modbus", 1);
            break;
        case MODBUS_RTU_RELAY:
            device = new DeviceModbusRTU_RELAY_01(id, name, "mac-modbus-2", 1);
            break;
#endif

        default:
            LOGW("Add new device not support type: 0x%04X", type);
            break;
        }
        if (device)
        {
            deviceListMtx.lock();
            deviceList[device->getId()] = device;
            deviceListMtx.unlock();
        }
#ifdef CONFIG_ENABLE_MODBUS
        DeviceModbus *deviceModbus = dynamic_cast<DeviceModbus *>(device);
        if (deviceModbus)
        {
            // modbusProtocol->AddDeviceModbus(deviceModbus);
            ModbusProtocol::GetInstance()->AddDeviceModbus(deviceModbus);
        }
#endif
        return device;
    }
    else
    {
        LOGW("Device type 0x%08x not support!!!", type);
    }
    return NULL;
}

Device *DeviceManager::GetDeviceFromId(string id)
{
    deviceListMtx.lock();
    if (deviceList.find(id) != deviceList.end())
    {
        deviceListMtx.unlock();
        return deviceList[id];
    }
    deviceListMtx.unlock();
    return NULL;
}

Device *DeviceManager::GetDeviceFromMac(string mac)
{
    deviceListMtx.lock();
    for (const auto &p : deviceList)
    {
        const auto &device = p.second;
        if (device->getMac() == mac)
        {
            deviceListMtx.unlock();
            return device;
        }
    }
    deviceListMtx.unlock();
    return NULL;
}

#ifdef CONFIG_ENABLE_BLE
DeviceBle *DeviceManager::GetDeviceBleFromAddr(uint16_t addr)
{
    deviceListMtx.lock();
    for (const auto &p : deviceList)
    {
        const auto &device = p.second;
        if (device->CheckAddr(addr) && device->getProtocol() == BLE_DEVICE)
        {
            DeviceBle *deviceBle = dynamic_cast<DeviceBle *>(device);
            if (deviceBle)
            {
                deviceListMtx.unlock();
                return deviceBle;
            }
        }
    }
    deviceListMtx.unlock();
    return NULL;
}

uint32_t DeviceManager::GetNextAddrBleAndroidProvision()
{
    uint32_t rs = 20000;
    if (deviceList.empty())
        return rs;

    auto it = deviceList.begin();
    Device *devMaxAddr = it->second;
    if (devMaxAddr)
    {
        for (; it != deviceList.end(); ++it)
        {
            if (it->second->getProtocol() == BLE_DEVICE)
            {
                if (it->second->getAddr() > devMaxAddr->getAddr())
                {
                    devMaxAddr = it->second;
                }
            }
        }

        if ((devMaxAddr->getAddr() + (uint32_t)devMaxAddr->GetNumElement()) > rs)
        {
            rs = devMaxAddr->getAddr() + devMaxAddr->GetNumElement();
        }
    }

    return rs;
}

uint32_t DeviceManager::GetMaxAddrBle()
{
    uint32_t nextAddr = 2;
    deviceListMtx.lock();
    for (const auto &p : deviceList)
    {
        const auto &device = p.second;
        if (device->getProtocol() == BLE_DEVICE)
        {
            if ((device->getAddr() > nextAddr) && (device->getAddr() < 20000))
            {
                nextAddr = device->getAddr();
            }
        }
    }
    deviceListMtx.unlock();
    return nextAddr;
}

vector<Device *> DeviceManager::GetListDeviceBle()
{
    vector<Device *> deviceList;
    ForEach([&](Device *device)
            {if (device->getProtocol() == BLE_DEVICE) 
				deviceList.push_back(device); });
    return deviceList;
}

#endif

#ifdef CONFIG_ENABLE_ZIGBEE
DeviceZigbee *DeviceManager::getDeviceZigbeeFromAddr(uint16_t addr)
{
    deviceListMtx.lock();
    for (const auto &[id, device] : deviceList)
    {
        if (device->CheckAddr(addr) && device->getProtocol() == ZIGBEE_DEVICE)
        {
            DeviceZigbee *deviceZigbee = dynamic_cast<DeviceZigbee *>(device);
            if (deviceZigbee)
            {
                deviceListMtx.unlock();
                return deviceZigbee;
            }
        }
    }
    deviceListMtx.unlock();
    return NULL;
}

vector<Device *> DeviceManager::GetListDeviceZigbee()
{
    vector<Device *> deviceList;
    ForEach([&](Device *device)
            {if (device->getProtocol() == ZIGBEE_DEVICE) 
				deviceList.push_back(device); });
    return deviceList;
}
#endif

// Del dev in all group, scenBle, room
void DeviceManager::RemoveDevice(Device *device)
{
    deviceListMtx.lock();
    deviceList.erase(device->getId());
    deviceListMtx.unlock();
    Database::GetInstance()->DeviceDel(device);
    delete device;
}

void DeviceManager::DelAllDevice()
{
    deviceListMtx.lock();
    for (const auto &p : deviceList)
    {
        const auto &device = p.second;
        delete device;
    }
    deviceList.clear();
    deviceListMtx.unlock();
}

extern "C" void led_eth(int value)
{
    // DeviceManager::GetInstance()->ledEth->Control(value);
}
extern "C" void led_wifi(int value)
{
    // DeviceManager::GetInstance()->ledWifi->Control(value);
}
extern "C" void led_lte(int value)
{
    // DeviceManager::GetInstance()->ledLte->Control(value);
}