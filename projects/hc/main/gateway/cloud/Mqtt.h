#pragma once

#include <string>
#include "json.h"
#include "MqttWrap.h"
#include "ActionCallback.h"

using namespace std;

class Mqtt : public MqttWrap
{
protected:
	vector<ActionCallback> actionCallbacks;

	virtual void OnConnect(int rc);
	virtual void OnDisconnect(int rc);
	virtual void OnPublish(int msgId);
	virtual void OnSubscribe(int msgId);
	virtual void OnSubscribeDone();
	virtual void OnUnsubscribe(int msgId);
	virtual void OnMessage(string topic, char *payload, int payloadLen);

public:
	Mqtt(string host, int port, const char *clientId, string username, string password, int keepalive, bool useTls = false, string willsetTopic = "", string willsetPayload = "");

	int init();

	void AddActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic);
	void AddActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic);
	void AddActionCallback(ActionCallbackFuncType3 actionCallbackFuncType3, string topic);
	void AddActionCallback(ActionCallbackFuncType4 actionCallbackFuncType4, string topic);
	int FindActionCallbackFuncFromTopic(string topic, ActionCallback **actionCallback);
	void SubscribeList();

	bool CheckData(Json::Value &dataValue, bool &rs);
};
