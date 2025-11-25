#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <byteswap.h>
#include "json.h"
#include "DeviceDefine.h"
#include "ErrorCode.h"
#include "Object.h"
#include "RuleInputRegister.h"

class DeviceInGroup;
class DeviceInScene;
class Device : public Object, public RuleInputRegister
{
protected:
	string mac;
	int type;
	uint32_t sw_ver;
	uint32_t hw_ver;
	int rssi;
	protocol_e protocol;
	Json::Value values;
	Json::Value propertyJsonUpdate;
	int powerSource;

	bool canAddToGroup;
	vector<DeviceInGroup *> deviceInGroupUnconfigList;
	mutex deviceInGroupUnconfigListMtx;

	bool canAddToScene;
	vector<DeviceInScene *> deviceInSceneUnconfigList;
	mutex deviceInSceneUnconfigListMtx;

public:
	bool lastOnlineState;
	time_t lastTimeActive;
	time_t lastTimeCheckActive;

public:
	Device(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
	virtual ~Device();

	// Getters and Setters
	string getMac() const { return mac; }
	void setMac(const string &mac_) { mac = mac_; }

	int getType() const { return type; }
	void setType(int type_) { type = type_; }

	uint32_t getSwVer() const { return sw_ver; }
	void setSwVer(uint32_t sw_ver_) { sw_ver = sw_ver_; }

	uint32_t getHwVer() const { return hw_ver; }
	void setHwVer(uint32_t hw_ver_) { hw_ver = hw_ver_; }

	int getRssi() const { return rssi; }
	void setRssi(int rssi_) { rssi = rssi_; }

	void setProtocol(protocol_e protocol_) { protocol = protocol_; }
	protocol_e getProtocol() const { return protocol; }

	Json::Value getValues() const { return values; }
	void setValues(const Json::Value &vals) { values = vals; }

	virtual bool CheckAddr(uint16_t addr) { return this->addr == addr; }
	// virtual bool CheckId(string id) { return this->id == id; }
	virtual string GetDeviceKey();
	string GetVersionStr();

	bool isOnline();
	bool isNeedCheckOnline();
	void UpdateLastTimeActive();

	virtual int BuildAttributeValue(Json::Value &pushDataValue);

	void DeviceInputData(uint8_t *data, int len, uint16_t addr);

	virtual int InitAttribute(string attribute, double value) { return CODE_ERROR; }
	virtual int DoJsonArray(Json::Value &dataValue);

	virtual void SetPropertyJsonUpdate(Json::Value property);
	virtual Json::Value GetPropertyJsonUpdate();
	virtual void UpdatePropertyJsonUpdate(Json::Value &property);

	int PushAttribute();
	int PushAttribute(Json::Value &jsonValue);
	int PushTelemetry();
	int PushTelemetry(Json::Value &jsonValue);

	static void InitDeviceModelList();
	static void RegisterDeviceModel(uint32_t type, const char *model, const char *name);
	static void RegisterDeviceOtherModel(uint32_t type, const char *model);
	static uint32_t BleTypeToGroupId(uint32_t deviceType);
	static const char *BleAttributeIdToAttributeStr(uint16_t attributeId);
	static const char *ConvertDeviceTypeToName(uint32_t type);
	static const char *ConvertDeviceTypeToModel(uint32_t type);
	static uint32_t ConvertModelToDeviceType(const char *model);

	DeviceInGroup *GetDeviceInGroupUnconfig(uint16_t epId, uint16_t groupAddr);
	void AddDeviceInGroupUnconfig(DeviceInGroup *deviceInGroup);
	void DelDeviceInGroupUnconfig(DeviceInGroup *deviceInGroup);

	DeviceInScene *GetDeviceInSceneUnconfig(uint16_t epId, uint16_t sceneAddr);
	void AddDeviceInSceneUnconfig(DeviceInScene *deviceInScene);
	void DelDeviceInSceneUnconfig(DeviceInScene *deviceInScene);

	// callback from RF
	int OnAddToGroup(uint16_t epId, uint16_t groupAddr, int status);
	int OnRemoveFromGroup(uint16_t epId, uint16_t groupAddr, int status);
	int OnAddToScene(uint16_t epId, uint16_t sceneAddr, int status);
	int OnRemoveFromScene(uint16_t epId, uint16_t sceneAddr, int status);

	virtual int BuildTelemetryValue(Json::Value &pushDataValue) { return CODE_ERROR; }
	// virtual void Getstatus(Json::Value &jsonValue) {}
	virtual void Isolate() {}

	virtual void InputData(Json::Value &dataValue) {}
	virtual void InputData(uint8_t *data, int len, uint16_t epId = 0);
	virtual bool CheckData(Json::Value &dataValue, bool &rs) { return false; }
	virtual int GetNumElement() { return 1; }

	// send RF
	virtual int AddToGroup(uint16_t epId, uint16_t groupAddr) { return CODE_ERROR; }
	virtual int RemoveFromGroup(uint16_t epId, uint16_t groupAddr) { return CODE_ERROR; }
	virtual int AddToScene(uint16_t epId, uint16_t sceneAddr) { return CODE_ERROR; }
	virtual int RemoveFromScene(uint16_t epId, uint16_t sceneAddr) { return CODE_ERROR; }

	virtual int AddToScene(uint16_t sceneAddr, Json::Value &dataValue, uint16_t groupAddr = 0) { return CODE_ERROR; }
	virtual int RemoveFromScene(uint16_t sceneAddr, Json::Value &dataValue, uint16_t groupAddr = 0) { return CODE_ERROR; }
	virtual int EditInScene(uint16_t sceneAddr, Json::Value &dataValue) { return CODE_ERROR; }

	virtual int Do(Json::Value &dataValue) { return CODE_ERROR; }
};
