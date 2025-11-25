#include <stdio.h>
#include "MqttWrap.h"
#include "Log.h"
#include "ErrorCode.h"
#include "Gateway.h"
#include "Config.h"
#include "DeviceManager.h"
#include "Util.h"

// static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
// {
// 	LOGV("Event dispatched from event loop base=%s, event_id=%d", base, event_id);
// 	MqttWrap *mqtt = (MqttWrap *)handler_args;
// 	esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
// 	esp_mqtt_client_handle_t client = event->client;
// 	int msg_id;
// 	switch ((esp_mqtt_event_id_t)event_id)
// 	{
// 	case MQTT_EVENT_CONNECTED:
// 		LOGI("MQTT_EVENT_CONNECTED");
// 		mqtt->OnConnect(0);
// 		break;
// 	case MQTT_EVENT_DISCONNECTED:
// 		LOGV("MQTT_EVENT_DISCONNECTED");
// 		mqtt->OnDisconnect(0);
// 		break;
// 	case MQTT_EVENT_SUBSCRIBED:
// 		LOGV("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
// 		mqtt->OnSubscribe(event->msg_id);
// 		break;
// 	case MQTT_EVENT_UNSUBSCRIBED:
// 		LOGV("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
// 		mqtt->OnUnsubscribe(event->msg_id);
// 		break;
// 	case MQTT_EVENT_PUBLISHED:
// 		LOGV("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
// 		mqtt->OnPublish(event->msg_id);
// 		break;
// 	case MQTT_EVENT_DATA:
// 		LOGV("MQTT_EVENT_DATA");
// 		{
// 			string topic = string(event->topic, event->topic + event->topic_len);
// 			mqtt->OnMessage(topic, event->data, event->data_len);
// 		}
// 		break;
// 	case MQTT_EVENT_ERROR:
// 		LOGV("MQTT_EVENT_ERROR");
// 		if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
// 		{
// 			// log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
// 			// log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
// 			// log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
// 			LOGW("Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
// 		}
// 		break;
// 	default:
// 		LOGV("Other event id:%d", event->event_id);
// 		break;
// 	}
// }

MqttWrap::MqttWrap(string host,
									 int port,
									 const char *clientId,
									 string username,
									 string password,
									 int keepalive,
									 bool useTls,
									 string willsetTopic,
									 string willsetPayload)
		: host(host),
			port(port),
			clientId(clientId),
			username(username),
			password(password),
			keepalive(keepalive),
			useTls(useTls),
			willsetTopic(willsetTopic),
			willsetPayload(willsetPayload)
{
	// client = NULL;
	isConnected = false;
}

int MqttWrap::init()
{
	LOGI("Connect host %s, port %d, clientId: %s, username: %s, pass: %s", host.c_str(), port, clientId, username.c_str(), password.c_str());
	// esp_mqtt_client_config_t mqtt_cfg = {0};
	// mqtt_cfg.broker.address.hostname = host.c_str();
	// mqtt_cfg.broker.address.port = (uint32_t)port;
	// mqtt_cfg.broker.address.transport = useTls ? MQTT_TRANSPORT_OVER_SSL : MQTT_TRANSPORT_OVER_TCP;
	// mqtt_cfg.credentials.client_id = clientId;
	// mqtt_cfg.credentials.username = username.c_str();
	// mqtt_cfg.credentials.authentication.password = password.c_str();
	// if (!willsetTopic.empty() && !willsetPayload.empty())
	// {
	// 	mqtt_cfg.session.last_will.topic = willsetTopic.c_str();
	// 	mqtt_cfg.session.last_will.msg = willsetPayload.c_str();
	// 	mqtt_cfg.session.last_will.msg_len = willsetPayload.length();
	// }
	// mqtt_cfg.session.keepalive = keepalive;
	// mqtt_cfg.task.stack_size = 10 * 1024;
	// mqtt_cfg.buffer.size = 50 * 1024;
	// client = esp_mqtt_client_init(&mqtt_cfg);
	// /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
	// esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, this);
	// // esp_mqtt_client_start(client);
	return CODE_OK;
}

int MqttWrap::Connect(int timeout)
{
	// esp_mqtt_client_start(client);
	return CODE_OK;
}

int MqttWrap::Subscribe(int *msgId, string topic)
{
	// if (client && isConnected)
	// {
	// 	LOGI("Subscribe topic: %s", topic.c_str());
	// 	// DeviceManager::GetInstance()->ledConf->Control(0);
	// 	int rs = esp_mqtt_client_subscribe(client, topic.c_str(), 0);
	// 	// DeviceManager::GetInstance()->ledConf->Control(1);
	// 	if (rs >= 0)
	// 	{
	// 		if (msgId)
	// 			*msgId = rs;
	// 		return CODE_OK;
	// 	}
	// }
	// else
	// {
	// 	LOGW("Subscribe topic: %s err, MQTT not connected", topic.c_str());
	// }
	return CODE_ERROR;
}

int MqttWrap::Unsubscribe(int *msgId, string topic)
{
	// if (client && isConnected)
	// {
	// 	LOGI("Unsubscribe topic: %s", topic.c_str());
	// 	// DeviceManager::GetInstance()->ledConf->Control(0);
	// 	int rs = esp_mqtt_client_unsubscribe(client, topic.c_str());
	// 	// DeviceManager::GetInstance()->ledConf->Control(1);
	// 	if (rs >= 0)
	// 	{
	// 		if (msgId)
	// 			*msgId = rs;
	// 		return CODE_OK;
	// 	}
	// }
	// else
	// {
	// 	LOGW("Unsubscribe topic: %s err, MQTT not connected", topic.c_str());
	// }
	return CODE_ERROR;
}

int MqttWrap::Publish(string topic, const char *payload, int payloadLen)
{
	// if (client && isConnected)
	// {
	// 	LOGD("Publish topic: %s, payload: %.*s", topic.c_str(), payloadLen, payload);
	// 	// DeviceManager::GetInstance()->ledConf->Control(0);
	// 	int rs = esp_mqtt_client_publish(client, topic.c_str(), payload, payloadLen, 0, 0);
	// 	// DeviceManager::GetInstance()->ledConf->Control(1);
	// 	if (rs >= 0)
	// 	{
	// 		return CODE_OK;
	// 	}
	// }
	// else
	// {
	// 	LOGW("Publish topic: %s, payload: %.*s err, MQTT not connected", topic.c_str(), payloadLen, payload);
	// }
	return CODE_ERROR;
}

int MqttWrap::Publish(string topic, string payload)
{
	return Publish(topic, payload.c_str(), payload.length());
}

bool MqttWrap::IsConnected()
{
	return isConnected;
}

void MqttWrap::OnConnect(int rc)
{
	isConnected = true;
}

void MqttWrap::OnDisconnect(int rc)
{
	isConnected = false;
}

void MqttWrap::OnPublish(int msgId)
{
}

void MqttWrap::OnSubscribe(int msgId)
{
}

void MqttWrap::OnUnsubscribe(int msgId)
{
}

void MqttWrap::OnMessage(string topic, char *payload, int payloadLen)
{
}

// bool MqttWrap::CheckData(Json::Value &dataValue, bool &rs)
// {
// 	LOGD("CheckData data: %s", dataValue.toString().c_str());
// 	if (dataValue.isObject() &&
// 			dataValue.isMember("value") &&
// 			dataValue.isMember("op") && dataValue["op"].isString())
// 	{
// 		string op = dataValue["op"].asString();
// 		if (dataValue["value"].isInt())
// 		{
// 			int value = dataValue["value"].asInt();
// 			rs = Util::CompareNumber(op, isConnected, value);
// 			return true;
// 		}
// 		else if (dataValue["value"].isArray())
// 		{
// 			Json::Value listValue = dataValue["value"];
// 			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
// 			{
// 				int value1 = listValue[0].asInt();
// 				int value2 = listValue[1].asInt();
// 				rs = Util::CompareNumber(op, isConnected, value1, value2);
// 				return true;
// 			}
// 		}
// 	}
// 	return false;
// }