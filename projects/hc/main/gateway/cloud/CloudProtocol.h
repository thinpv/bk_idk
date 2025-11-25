#pragma once

#include "Mqtt.h"
#include "json.h"
#include <map>
#include <atomic>
#include <mutex>
#include "Device.h"
#include "Group.h"

using namespace std;

class CloudProtocol : public Mqtt
{
	typedef struct
	{
		bool status;
		string respCmd;
		Json::Value *respValue;
		string pubTopic;
	} request_t;
	map<string, request_t *> requestList;

	typedef struct
	{
		bool status;
		char *payload;
		int *payloadLen;
	} request_bin_t;
	map<string, request_bin_t *> requestBinList;

	atomic<bool> isBusy;
	mutex mtx;
	int msgId;

	string fwChecksumAlgorithm;
	string fwChecksum;
	string fwUrl;
	int fwSize;
	int msgOtaId;
	int chunkId;
	int chunkTotal;

	Json::Value telemetryValue;
	mutex telemetryValueMtx;
	uint64_t telemetryValueLastUpdate;

	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnRpcCallbackFunc;
	map<string, OnRpcCallbackFunc> onRpcCallbackFuncList;

	typedef function<int(string &deviceId, Json::Value &reqValue, Json::Value &respValue)> OnDeviceRpcCallbackFunc;
	map<string, OnDeviceRpcCallbackFunc> onDeviceRpcCallbackFuncList;

	void OnServerDeviceAttribute(string &topic, string &payload);
	void OnServerDeviceAttributeResp(string &topic, string &payload);
	void OnServerRpc(string &topic, string &payload);
	void OnServerDeviceRpc(string &topic, string &payload);
	void OnFwChunkResp(string &topic, char *payload, int payloadLen);
	void OnBridgeMatter2HcRpc(string &topic, string &payload);
	void OnRouterRpc(string &topic, string &payload);

protected:
	virtual void CheckSharedDeviceAttrite(Json::Value &payloadJson, bool updateParams);

public:
	CloudProtocol(string address, int port, const char *clientId, string username, string password, int keepalive, bool ssl);
	virtual ~CloudProtocol();

	void init();
	int GetMsgId();
	void SetMsgId(int msgId);

	bool IsBusy() { return isBusy; }

	int OnRpcCallbackRegister(string cmd, OnRpcCallbackFunc onRpcCallbackFunc);
	int OnDeviceRpcCallbackRegister(string cmd, OnDeviceRpcCallbackFunc onDeviceRpcCallbackFunc);

	void pushTelemetry();

	int GatewayAttribute(Json::Value &dataValue);
	int GatewayTelemetry(Json::Value &dataValue);
	int DeviceAddNew(Device *device);
	int DeviceConnect(Device *device);
	int DeviceDisconnect(Device *device);
	int DeviceAttribute(Device *device, Json::Value &dataValue);
	int DeviceAttribute(string deviceId, Json::Value &dataValue);
	int DeviceTelemetry(Device *device, Json::Value &dataValue);
	int DeviceTelemetry(string deviceId, Json::Value &dataValue);
	int DeviceAttributeRequest(string data);
	int DeviceRpcRequest(string data);

	int FwRequestChunk(int chunkId);

	int PublishToCloudMessage(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout = 0);
	int PublishBinToCloudMessage(string sessionId, int index, char *payload, int payloadLen, string respCmd, Json::Value *respValue, uint32_t timeout = 0);
	int PublishToCloudRecieveBinMessage(string reqCmd, Json::Value &reqValue, string rqi, char *payload, int *payloadLen, uint32_t timeout = 0);

	virtual bool FirmwareCheckNeedUpdate(string fwVersion) { return false; }
	virtual int FirmwareUpdate(string fwUrl, string fwChecksumAlgorithm, string fwChecksum) { return CODE_ERROR; }
	virtual int FirmwareUpdate(int fwSize, string fwChecksumAlgorithm, string fwChecksum) { return CODE_ERROR; }
	virtual int FirmwareUpdateChunk(int chunkId, uint8_t *data, int dataLen) { return CODE_ERROR; }
	virtual int FirmwareUpdateFinish() { return CODE_ERROR; }
	int SendTimeoutProvisionDev();

	// // Matter register device
	// int OnHcBridgeRpc(string &topic, string &payload);
	// int DeviceMatterRegister(Device *device);
	// int DeviceMatterDelete(Device *device);
	// int ListDeviceMatterInit();
	// int GetMatterCommission();
	// int GetMatterFabric();
};