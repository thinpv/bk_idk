#pragma once

#include <map>
#include <mutex>
#include <functional>
#include "Device.h"
#ifdef CONFIG_ENABLE_BLE
#include "DeviceBle.h"
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
#include "DeviceZigbee.h"
#endif

using namespace std;

class DeviceManager
{
private:
	map<string, Device *> deviceList;
	mutex deviceListMtx;

	DeviceManager();

public:
	static DeviceManager *GetInstance();
	void ForEach(function<void(Device *)> func);

	Device *AddDevice(string &mac, uint32_t type, uint16_t addr = 0, uint16_t version = 0, Json::Value *dataJson = NULL);
	Device *AddDevice(string &id, string &mac, uint32_t type, uint16_t addr = 0, uint16_t version = 0, Json::Value *dataJson = NULL);
	Device *GetDeviceFromId(string id);
	Device *GetDeviceFromMac(string mac);
#ifdef CONFIG_ENABLE_BLE
	DeviceBle *GetDeviceBleFromAddr(uint16_t addr);
	uint32_t GetNextAddrBleAndroidProvision();
	uint32_t GetMaxAddrBle();
	vector<Device *> GetListDeviceBle();
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
	DeviceZigbee *getDeviceZigbeeFromAddr(uint16_t addr);
	vector<Device *> GetListDeviceZigbee();
#endif
	void RemoveDevice(Device *device);
	void DelAllDevice();
};