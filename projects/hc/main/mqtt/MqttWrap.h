#pragma once

// #include "esp_event.h"
// #include "mqtt_client.h"
#include <string>
#include "ActionCallback.h"
// #include "RuleInputRegister.h"

using namespace std;

class MqttWrap // : public RuleInputRegister
{
protected:
	string host;
	int port;
	const char *clientId;
	string username;
	string password;
	int keepalive;
	bool useTls;
	string willsetTopic;
	string willsetPayload;
	volatile bool isConnected;
	// esp_mqtt_client_handle_t client;

public:
	MqttWrap(string host, int port, const char *clientId, string username, string password, int keepalive, bool useTls = false, string willsetTopic = "", string willsetPayload = "");

	int init();
	int Connect(int timeout = 10);
	int Subscribe(int *msgId, string topic);
	int Unsubscribe(int *msgId, string topic);
	int Publish(string topic, const char *payload, int payloadLen);
	int Publish(string topic, string payload);
	bool IsConnected();

	virtual void OnConnect(int rc);
	virtual void OnDisconnect(int rc);
	virtual void OnPublish(int msgId);
	virtual void OnSubscribe(int msgId);
	virtual void OnUnsubscribe(int msgId);
	virtual void OnMessage(string topic, char *payload, int payloadLen);

	// bool CheckData(Json::Value &dataValue, bool &rs);
};
