#pragma once

#include <string>
#include <stdint.h>
#include <functional>
#include "json.h"

using namespace std;

class MqttProtocol
{
private:
	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnMqttProtocolCallbackFunc;
	map<string, OnMqttProtocolCallbackFunc> onMqttProtocolCallbackFuncList;

	int OnMqttProtocolCallbackRegister(string cmd, OnMqttProtocolCallbackFunc onMqttProtocolCallbackFunc);
	void OnMessage(string &topic, string &payload);
	int SendMessage(string data);

	int OnAddDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnAddFunction(Json::Value &reqValue, Json::Value &respValue);
	int OnDeviceStatus(Json::Value &reqValue, Json::Value &respValue);

public:
	MqttProtocol();
	virtual ~MqttProtocol();

	void init();
};

// extern MqttProtocol *mqttProtocol;
