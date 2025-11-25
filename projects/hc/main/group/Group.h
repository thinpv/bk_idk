#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "json.h"
#include "Object.h"
#include "Device.h"

using namespace std;

class DeviceInGroup
{
private:
	bool isConfigured;
	bool isInGroup;
	int retryCount;
	bool waitingToCheck;
	Json::Value dataValue;

public:
	Device *device;
	uint16_t epId;
	uint16_t groupAddr;

	DeviceInGroup(Device *device, uint16_t epId, uint16_t groupAddr, bool isInGroup = true, bool isConfigured = true)
			: isConfigured(isConfigured),
				isInGroup(isInGroup),
				retryCount(0),
				waitingToCheck(false),
				device(device),
				epId(epId),
				groupAddr(groupAddr)
	{
		dataValue["isConfigured"] = isConfigured;
		dataValue["isInGroup"] = isInGroup;
		dataValue["retryCount"] = retryCount;
	}

	bool getWaitingToCheck() const { return waitingToCheck; }
	void setWaitingToCheck(bool value) { waitingToCheck = value; }

	bool getIsConfigured() const { return isConfigured; }
	void setIsConfigured(bool value);

	bool getIsInGroup() const { return isInGroup; }
	void setIsInGroup(bool value);

	int getRetryCount() const { return retryCount; }
	void setRetryCount(int value);

	Json::Value getData() const { return dataValue; }
	string getDataStr() const { return dataValue.toString(); }
	void setData(const string &value);
	void parseDataValue(Json::Value &dataValue);
};

class Group : public Object
{
private:
	mutex deviceListMtx;
	int onoff;

public:
	vector<DeviceInGroup *> deviceList;
	Group(string id, string name, uint16_t addr, uint64_t updatedAt);
	Group(string id, string name, uint16_t addr);
	virtual ~Group();

	size_t countDevice();
	DeviceInGroup *GetDeviceInGroup(Device *device, uint16_t epId);
	int GetPositionDevice(Device *device, uint16_t epId);
	DeviceInGroup *AddDeviceInGroup(DeviceInGroup *deviceInGroup);
	DeviceInGroup *AddDeviceAndConfig(Device *device, uint16_t epId);
	int DelDevice(Device *device, uint16_t epId);
	int DelDeviceAndConfig(Device *device, uint16_t epId);

	int DeleteDeviceInGroup(DeviceInGroup *deviceInGroup, bool needConfig);
	int RemoveDevice(Device *device, bool needConfig);
	void StartCheckDeviceInGroup();
	void StopCheckDeviceInGroup();

	int Do(Json::Value &dataValue, bool ack = true);
};
