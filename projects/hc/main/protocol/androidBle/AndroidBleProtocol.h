#pragma once

#include <string>
#include <stdint.h>
#include <functional>
#include "json.h"

using namespace std;

class AndroidBleProtocol
{
private:
	typedef struct
	{
		bool status;
		string respCmd;
		Json::Value *respValue;
		string pubTopic;
	} request_t;
	map<string, request_t *> requestList;

	string subTopic;
	string pubTopic;

	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnAndroidBleProtocolCallbackFunc;
	map<string, OnAndroidBleProtocolCallbackFunc> onAndroidBleProtocolCallbackFuncList;

	int OnAndroidBleProtocolCallbackRegister(string cmd, OnAndroidBleProtocolCallbackFunc onAndroidBleProtocolCallbackFunc);
	void OnMessage(string &topic, string &payload);
	int SendMessage(string data);

	int OnBleInfo(Json::Value &reqValue, Json::Value &respValue);
	int OnProvisionNormal(Json::Value &reqValue, Json::Value &respValue);
	int OnNewDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnDeviceStatus(Json::Value &reqValue, Json::Value &respValue);

	void OnAndroidBleResp(string &topic, string &payload);

public:
	AndroidBleProtocol();
	virtual ~AndroidBleProtocol();

	void init();

	int StartScan();
	int StopScan();

	int PublishToAndroidBleMessage(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout = 1000);

};

extern AndroidBleProtocol *androidBleProtocol;
