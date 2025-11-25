#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "json.h"
#include "Object.h"
#include "Device.h"

using namespace std;

class DeviceInScene
{
private:
	bool isConfigured;
	bool isInScene;
	int retryCount;
	bool waitingToCheck;
	Json::Value dataValue;

public:
	Device *device;
	uint16_t epId;
	uint16_t sceneAddr;

	DeviceInScene(Device *device, uint16_t epId, uint16_t sceneAddr, bool isInScene = true, bool isConfigured = true)
			: isConfigured(isConfigured),
				isInScene(isInScene),
				retryCount(0),
				waitingToCheck(false),
				device(device),
				epId(epId),
				sceneAddr(sceneAddr)
	{
		dataValue["isConfigured"] = isConfigured;
		dataValue["isInScene"] = isInScene;
		dataValue["retryCount"] = retryCount;
	}

	bool getWaitingToCheck() const { return waitingToCheck; }
	void setWaitingToCheck(bool value) { waitingToCheck = value; }

	bool getIsConfigured() const { return isConfigured; }
	void setIsConfigured(bool value);

	bool getIsInScene() const { return isInScene; }
	void setIsInScene(bool value);

	int getRetryCount() const { return retryCount; }
	void setRetryCount(int value);

	Json::Value getData() const { return dataValue; }
	string getDataStr() const { return dataValue.toString(); }
	void setData(const string &value);
	void parseDataValue(Json::Value &dataValue);
};

class Scene : public Object
{
private:
	mutex deviceListMtx;

public:
	vector<DeviceInScene *> deviceList;
	Scene(string id, string name, uint16_t addr, uint64_t updatedAt);
	Scene(string id, string name, uint16_t addr);
	virtual ~Scene();

	size_t countDevice();
	DeviceInScene *GetDeviceInScene(Device *device, uint16_t epId);
	int GetPositionDevice(Device *device, uint16_t epId);
	DeviceInScene *AddDeviceInScene(DeviceInScene *deviceInScene);
	DeviceInScene *AddDeviceAndConfig(Device *device, uint16_t epId);
	int DelDevice(Device *device, uint16_t epId);
	int DelDeviceAndConfig(Device *device, uint16_t epId);

	int DeleteDeviceInScene(DeviceInScene *deviceInScene, bool needConfig);
	int RemoveDevice(Device *device, bool needConfig);
	void StartCheckDeviceInScene();
	void StopCheckDeviceInScene();

	int Do(Json::Value &dataValue, bool ack = true);
	int Do(bool ack = true, uint16_t groupAddr = 0);
};
