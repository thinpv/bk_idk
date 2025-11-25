#include <stdio.h>
#include "Mqtt.h"
#include "Log.h"
#include "ErrorCode.h"
#include "Gateway.h"
#include "Config.h"
#include "DeviceManager.h"
#include "Util.h"

Mqtt::Mqtt(string host,
					 int port,
					 const char *clientId,
					 string username,
					 string password,
					 int keepalive,
					 bool useTls,
					 string willsetTopic,
					 string willsetPayload)
		: MqttWrap(host, port, clientId, username, password, keepalive, useTls, willsetTopic, willsetPayload)
{
}

int Mqtt::init()
{
	MqttWrap::init();
	return CODE_OK;
}

void Mqtt::OnConnect(int rc)
{
	MqttWrap::OnConnect(rc);

	LOGI("OnConnect");
	// DeviceManager::GetInstance()->ledConf->Control(1);
	SubscribeList();
	// if (oldState == false)
	// 	CheckTrigger();
}

void Mqtt::OnDisconnect(int rc)
{
	MqttWrap::OnDisconnect(rc);

	LOGW("OnDisconnect");
	// bool oldState = isConnected;
	// DeviceManager::GetInstance()->ledConf->Control(0);
	for (auto &action : actionCallbacks)
	{
		action.SetState(false);
	}
	// if (oldState == true)
	// 	CheckTrigger();
}

void Mqtt::OnPublish(int msgId)
{
	MqttWrap::OnPublish(msgId);
}

void Mqtt::OnSubscribe(int msgId)
{
	MqttWrap::OnSubscribe(msgId);

	for (auto &action : actionCallbacks)
	{
		if (action.GetMsgId() == msgId)
		{
			action.SetState(true);
		}
	}
	SubscribeList();
}

void Mqtt::OnSubscribeDone()
{
}

void Mqtt::OnUnsubscribe(int msgId)
{
	MqttWrap::OnUnsubscribe(msgId);

	for (auto &action : actionCallbacks)
	{
		if (action.GetMsgId() == msgId)
		{
			action.SetState(false);
		}
	}
}

void Mqtt::OnMessage(string topic, char *payload, int payloadLen)
{
	MqttWrap::OnMessage(topic, payload, payloadLen);

	// DeviceManager::GetInstance()->ledConf->Control(0);
	string msg = string(payload, payloadLen);
	LOGD("OnMessage topic: %s", topic.c_str());
	ActionCallback *actionCallback;
	if (FindActionCallbackFuncFromTopic(topic, &actionCallback) == CODE_OK)
	{
		if (actionCallback->getType() == 1)
		{
			string payloadStr = string(payload, payloadLen);
			actionCallback->actionCallbackFuncType1(topic, payloadStr);
		}
		else if (actionCallback->getType() == 2)
		{
			actionCallback->actionCallbackFuncType2(topic, payload, payloadLen);
		}
		else if (actionCallback->getType() == 3)
		{
			string payloadStr = string(payload, payloadLen);
			actionCallback->actionCallbackFuncType3(topic, payloadStr);
		}
		else if (actionCallback->getType() == 4)
		{
			actionCallback->actionCallbackFuncType4(topic, payload, payloadLen);
		}
	}
	// DeviceManager::GetInstance()->ledConf->Control(1);
}

void Mqtt::AddActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic)
{
	ActionCallback actionCallback(actionCallbackFuncType1, topic);
	actionCallbacks.push_back(actionCallback);
	Subscribe(actionCallback.GetMsgIdPtr(), topic);
}

void Mqtt::AddActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic)
{
	ActionCallback actionCallback(actionCallbackFuncType2, topic);
	actionCallbacks.push_back(actionCallback);
	Subscribe(actionCallback.GetMsgIdPtr(), topic);
}

void Mqtt::AddActionCallback(ActionCallbackFuncType3 actionCallbackFuncType3, string topic)
{
	ActionCallback actionCallback(actionCallbackFuncType3, topic);
	actionCallbacks.push_back(actionCallback);
	Subscribe(actionCallback.GetMsgIdPtr(), topic);
}

void Mqtt::AddActionCallback(ActionCallbackFuncType4 actionCallbackFuncType4, string topic)
{
	ActionCallback actionCallback(actionCallbackFuncType4, topic);
	actionCallbacks.push_back(actionCallback);
	Subscribe(actionCallback.GetMsgIdPtr(), topic);
}

int checkMqttTopic(string retrieveTopic, string registerTopic)
{
	vector<string> retrieveList = Util::splitString(retrieveTopic, '/');
	vector<string> registerList = Util::splitString(registerTopic, '/');
	for (size_t i = 0; i < retrieveList.size(); i++)
	{
		// fix MQTT check topic bug https://github.com/thinpv/smh_HC/commit/cf90cea3695d99ce10d8c8e6e048031e77923236
		if (registerList.size() <= i)
			return CODE_ERROR;
		if (registerList.at(i) == "#")
			return CODE_OK; // OK
		if (registerList.at(i) == "+")
			continue;
		if (registerList.at(i) != retrieveList.at(i))
			return CODE_ERROR;
	}
	if (registerList.size() == retrieveList.size())
		return CODE_OK; // OK
	return CODE_ERROR;
}

int Mqtt::FindActionCallbackFuncFromTopic(string topic, ActionCallback **actionCallback)
{
	for (auto &action : actionCallbacks)
	{
		if (checkMqttTopic(topic, action.getTopic()) == CODE_OK)
		{
			*actionCallback = &action;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

void Mqtt::SubscribeList()
{
	for (auto &action : actionCallbacks)
	{
		if (!action.GetState())
		{
			Subscribe(action.GetMsgIdPtr(), action.getTopic());
			return;
		}
	}
	OnSubscribeDone();
}

bool Mqtt::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("value") &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue["value"].isInt())
		{
			int value = dataValue["value"].asInt();
			rs = Util::CompareNumber(op, isConnected, value);
			return true;
		}
		else if (dataValue["value"].isArray())
		{
			Json::Value listValue = dataValue["value"];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int value1 = listValue[0].asInt();
				int value2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, isConnected, value1, value2);
				return true;
			}
		}
	}
	return false;
}