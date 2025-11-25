#include "CloudProtocol.h"
#include "Log.h"
#include "Util.h"
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <thread>
#include "Define.h"

#ifdef ESP_PLATFORM
#include "app_task.h"
#endif

#define OTA_CHUNK_SIZE 1024

CloudProtocol::CloudProtocol(string address, int port, const char *clientId, string username, string password, int keepalive, bool ssl)
		: Mqtt(address, port, clientId, username, password, keepalive, ssl)
{
	msgId = 0;
	fwChecksumAlgorithm = "";
	fwChecksum = "";
	fwUrl = "";
	fwSize = 0;
	msgOtaId = 0;
	chunkId = 0;
	chunkTotal = 0;
	telemetryValue = Json::objectValue;
}

CloudProtocol::~CloudProtocol()
{
}

void CloudProtocol::init()
{
	Mqtt::init();

	AddActionCallback(bind(&CloudProtocol::OnServerDeviceAttribute, this, placeholders::_1, placeholders::_2), "v1/devices/me/attributes");
	AddActionCallback(bind(&CloudProtocol::OnServerDeviceAttributeResp, this, placeholders::_1, placeholders::_2), "v1/devices/me/attributes/response/+");
	AddActionCallback(bind(&CloudProtocol::OnServerRpc, this, placeholders::_1, placeholders::_2), "v1/devices/me/rpc/request/+");
	AddActionCallback(bind(&CloudProtocol::OnServerDeviceRpc, this, placeholders::_1, placeholders::_2), "v1/gateway/rpc");
	AddActionCallback(bind(&CloudProtocol::OnFwChunkResp, this, placeholders::_1, placeholders::_2, placeholders::_3), "v2/fw/response/+/chunk/+");

#ifdef ESP_PLATFORM
	LOGI("Free memory: %d bytes, internal: %d bytes", esp_get_free_heap_size(), esp_get_free_internal_heap_size());
	if (!app_new_task([](void *arg)
										{
	LOGI("pushTelemetryThread Start");
	CloudProtocol *cloudProtocol = (CloudProtocol *)arg;
	cloudProtocol->pushTelemetry();
	vTaskDelete(NULL); },									 // Task function
										"pushTelemetryThread", // Task name
										20480,								 // Stack size (words, not bytes)
										this,									 // Param
										5											 // Priority
										))
	{
		LOGE("Failed to create pushTelemetryThread task");
	}
#else
	thread pushTelemetryThread(bind(&CloudProtocol::pushTelemetry, this));
	pushTelemetryThread.detach();
#endif
}

void CloudProtocol::pushTelemetry()
{
	while (true)
	{
		telemetryValueMtx.lock();
		if (!telemetryValue.empty() &&
				((Util::millis() - telemetryValueLastUpdate > 500) || (telemetryValue.toString().length() > 1024)))
		{
			Publish("v1/gateway/telemetry", telemetryValue.toString());
			telemetryValue = Json::objectValue;
		}
		telemetryValueMtx.unlock();
		SLEEP_MS(100);
	}
}

int CloudProtocol::GetMsgId()
{
	return this->msgId;
}

void CloudProtocol::SetMsgId(int msgId)
{
	this->msgId = msgId;
}

void CloudProtocol::CheckSharedDeviceAttrite(Json::Value &payloadJson, bool updateParams)
{
	if (payloadJson.isObject())
	{
		for (const auto &attribute : payloadJson.getMemberNames())
		{
			LOGD("attribute: %s", attribute.c_str());
		}
		if (payloadJson.isMember("fw_version") && payloadJson["fw_version"].isString())
		{
			string fwVersion = payloadJson["fw_version"].asString();
			LOGI("Newest fw version: %s", fwVersion.c_str());
			if (FirmwareCheckNeedUpdate(fwVersion))
			{
				if (payloadJson.isMember("fw_size") && payloadJson["fw_size"].isInt() &&
						payloadJson.isMember("fw_checksum_algorithm") && payloadJson["fw_checksum_algorithm"].isString() &&
						payloadJson.isMember("fw_checksum") && payloadJson["fw_checksum"].isString())
				{
					string fwChecksumAlgorithm = payloadJson["fw_checksum_algorithm"].asString();
					string fwChecksum = payloadJson["fw_checksum"].asString();
					if (this->fwChecksum != "" &&
							fwChecksumAlgorithm == this->fwChecksumAlgorithm &&
							fwChecksum == this->fwChecksum)
					{
						LOGI("Continue update firmware chunkId: %d", chunkId);
						FwRequestChunk(chunkId);
					}
					else
					{
						LOGI("New fw checksum algorithm: %s, checksum: %s", fwChecksumAlgorithm.c_str(), fwChecksum.c_str());
						this->fwChecksumAlgorithm = fwChecksumAlgorithm;
						this->fwChecksum = fwChecksum;
						fwSize = payloadJson["fw_size"].asInt();
						chunkTotal = fwSize / OTA_CHUNK_SIZE;
						if (fwSize % OTA_CHUNK_SIZE)
							chunkTotal++;
						chunkId = 0;
						LOGI("New fw size: %d, chunk: %d", fwSize, chunkTotal);
						if (FirmwareUpdate(fwSize, fwChecksumAlgorithm, fwChecksum) == CODE_OK)
						{
							FwRequestChunk(chunkId);
						}
					}
				}
				else if (payloadJson.isMember("fw_url") && payloadJson["fw_url"].isString())
				{
					string fwUrl = payloadJson["fw_url"].asString();
					if (fwUrl != this->fwUrl)
					{
						this->fwUrl = fwUrl;
						LOGI("New fw url: %s", fwUrl.c_str());
					}
					else
					{
						LOGI("Continue update firmware url: %s", fwUrl.c_str());
					}
					LOGI("New fw url: %s", fwUrl.c_str());
					int rs = FirmwareUpdate(fwUrl, "", "");
					Json::Value jsonValue;
					if (rs == CODE_OK)
					{
						jsonValue["fw_state"] = "UPDATED";
					}
					else
					{
						jsonValue["fw_state"] = "FAILED";
					}
					GatewayTelemetry(jsonValue);
					if (rs == CODE_OK)
					{
						sleep(5);
						exit(1);
					}
				}
			}
		}
	}
}

void CloudProtocol::OnServerDeviceAttribute(string &topic, string &payload)
{
	Json::Value payloadJson;
	if (payloadJson.parse(payload) && payloadJson.isObject())
	{
		CheckSharedDeviceAttrite(payloadJson, true);
	}
	else
	{
		LOGW("OnServerDeviceAttribute topic: %s payload: %s", topic.c_str(), payload.c_str());
	}
}

void CloudProtocol::OnServerDeviceAttributeResp(string &topic, string &payload)
{
	LOGD("OnServerDeviceAttributeResp: %s", payload.c_str());
	// string id = topic.substr(strlen("v1/devices/me/attributes/response/"));
	Json::Value payloadJson;
	payloadJson.parse(payload);
	CheckSharedDeviceAttrite(payloadJson["shared"], false);
}

void CloudProtocol::OnServerRpc(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	string id = topic.substr(strlen("v1/devices/me/rpc/request/"));
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
			payloadJson.isMember("method") && payloadJson["method"].isString() &&
			payloadJson.isMember("params"))
	{
		string method = payloadJson["method"].asString();
		if (onRpcCallbackFuncList.find(method) != onRpcCallbackFuncList.end())
		{
			OnRpcCallbackFunc onRpcCallbackFunc = onRpcCallbackFuncList[method];
			int rs = onRpcCallbackFunc(payloadJson["params"], respValue);
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", method.c_str(), rs);
				Publish("v1/devices/me/rpc/response/" + id, respValue.toString());
			}
			else if (rs == CODE_EXIT)
			{
				LOGD("Call %s OK, rs: %d", method.c_str(), rs);
				Publish("v1/devices/me/rpc/response/" + id, respValue.toString());
				sleep(5);
				exit(1);
			}
			else if (rs == CODE_NOT_RESPONSE)
			{
				LOGD("Call %s OK, rs: %d", method.c_str(), rs);
			}
			else
			{
				LOGW("Call %s ERR rs: %d", method.c_str(), rs);
			}
			SLEEP_MS(10);
		}
		else
		{
			LOGW("Cmd %s not registed", method.c_str());
			LOGW("OnServerRpc payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnServerRpc topic: %s payload: %s", topic.c_str(), payload.c_str());
	}
}

// TODO: move to Gateway handle
void CloudProtocol::OnServerDeviceRpc(string &topic, string &payload)
{
	LOGI("payload: %s", payload.c_str());
	Json::Value payloadJson;
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
			payloadJson.isMember("device") && payloadJson["device"].isString() &&
			payloadJson.isMember("data") && payloadJson["data"].isObject())
	{
		Json::Value dataValue = payloadJson["data"];
		string deviceId = payloadJson["device"].asString();
		if (dataValue.isMember("id") && dataValue["id"].isInt() &&
				dataValue.isMember("method") && dataValue["method"].isString() &&
				dataValue.isMember("params") && dataValue["params"].isObject())
		{
			Json::Value dataRespValue;
			Json::Value respValue;
			int id = dataValue["id"].asInt();
			string method = dataValue["method"].asString();
			if (onDeviceRpcCallbackFuncList.find(method) != onDeviceRpcCallbackFuncList.end())
			{
				OnDeviceRpcCallbackFunc onDeviceRpcCallbackFunc = onDeviceRpcCallbackFuncList[method];
				int rs = onDeviceRpcCallbackFunc(deviceId, dataValue["params"], dataRespValue);
				if (rs == CODE_OK)
				{
					LOGD("Call %s OK, rs: %d", method.c_str(), rs);
					respValue["device"] = deviceId;
					respValue["id"] = id;
					respValue["data"] = dataRespValue;
					Publish("v1/gateway/rpc", respValue.toString());
				}
				else if (rs == CODE_EXIT)
				{
					LOGD("Call %s OK, rs: %d", method.c_str(), rs);
					respValue["device"] = deviceId;
					respValue["id"] = id;
					respValue["data"] = dataRespValue;
					Publish("v1/gateway/rpc", respValue.toString());
					sleep(5);
					exit(1);
				}
				else if (rs == CODE_NOT_RESPONSE)
				{
					LOGD("Call %s OK, rs: %d", method.c_str(), rs);
				}
				else
				{
					LOGW("Call %s ERR rs: %d", method.c_str(), rs);
				}
				SLEEP_MS(10);
			}
			else
			{
				LOGW("Cmd %s not registed", method.c_str());
				LOGW("OnServerDeviceRpc payload: %s", payload.c_str());
			}
		}
	}
	else
	{
		LOGW("OnServerDeviceRpc topic: %s payload: %s", topic.c_str(), payload.c_str());
	}
}

void CloudProtocol::OnFwChunkResp(string &topic, char *payload, int payloadLen)
{
	vector<string> topics = Util::splitString(topic, '/');
	if (topics.size() != 6)
		return;
	int msgOtaId = stoi(topics[3]);
	int chunkId = stoi(topics[5]);
	LOGD("OnFwChunkResp megId: %d, chunkId: %d, payloadLen: %d", msgOtaId, chunkId, payloadLen);
	if (msgOtaId == this->msgOtaId && chunkId == this->chunkId)
	{
		LOGI("OnFwChunkResp Write firm chunkId: %d", chunkId);
		if (payloadLen)
		{
			if (FirmwareUpdateChunk(chunkId, (uint8_t *)payload, payloadLen) == CODE_OK)
			{
				if (chunkId < chunkTotal - 1)
				{
					FwRequestChunk(chunkId + 1);
				}
				else
				{
					int rs = FirmwareUpdateFinish();
					Json::Value jsonValue;
					if (rs == CODE_OK)
					{
						jsonValue["fw_state"] = "UPDATED";
					}
					else
					{
						jsonValue["fw_state"] = "UPDATE_FAILED";
					}
					GatewayTelemetry(jsonValue);
					if (rs == CODE_OK)
					{
						sleep(5);
						exit(1);
					}
				}
			}
			else
			{
				// FwRequestChunk(chunkId);
				Json::Value jsonValue;
				jsonValue["fw_state"] = "DOWNLOAD_FAILED";
				GatewayTelemetry(jsonValue);
			}
		}
		else
		{
			int rs = FirmwareUpdateFinish();
			Json::Value jsonValue;
			if (rs == CODE_OK)
			{
				jsonValue["fw_state"] = "UPDATED";
			}
			else
			{
				jsonValue["fw_state"] = "FAILED";
			}
			GatewayTelemetry(jsonValue);
			if (rs == CODE_OK)
			{
				sleep(5);
				exit(1);
			}
		}
	}
}

int CloudProtocol::OnRpcCallbackRegister(string cmd, OnRpcCallbackFunc onRpcCallbackFunc)
{
	LOGI("OnRpcCallbackRegister cmd: %s", cmd.c_str());
	onRpcCallbackFuncList[cmd] = onRpcCallbackFunc;
	return CODE_OK;
}

int CloudProtocol::OnDeviceRpcCallbackRegister(string cmd, OnDeviceRpcCallbackFunc onDeviceRpcCallbackFunc)
{
	LOGI("OnDeviceRpcCallbackRegister cmd: %s", cmd.c_str());
	onDeviceRpcCallbackFuncList[cmd] = onDeviceRpcCallbackFunc;
	return CODE_OK;
}

int CloudProtocol::GatewayAttribute(Json::Value &dataValue)
{
	return Publish("v1/devices/me/attributes", dataValue.toString());
}

int CloudProtocol::GatewayTelemetry(Json::Value &dataValue)
{
	return Publish("v1/devices/me/telemetry", dataValue.toString());
}

int CloudProtocol::DeviceAddNew(Device *device)
{
	Json::Value jsonValue;
	jsonValue["device"] = device->getId();
	jsonValue["type"] = Device::ConvertDeviceTypeToModel(device->getType());
	return Publish("v1/gateway/connect", jsonValue.toString());
}

int CloudProtocol::DeviceConnect(Device *device)
{
	Json::Value jsonValue;
	jsonValue["device"] = device->getId();
	jsonValue["type"] = "Type_" + to_string(device->getType());
	return Publish("v1/gateway/connect", jsonValue.toString());
}

int CloudProtocol::DeviceDisconnect(Device *device)
{
	Json::Value jsonValue;
	jsonValue["device"] = device->getId();
	return Publish("v1/gateway/disconnect", jsonValue.toString());
}

int CloudProtocol::DeviceAttribute(Device *device, Json::Value &dataValue)
{
	return DeviceAttribute(device->getId(), dataValue);
}

int CloudProtocol::DeviceAttribute(string deviceId, Json::Value &dataValue)
{
	if (!dataValue.isNull())
	{
		Json::Value jsonValue;
		jsonValue["device"] = deviceId;
		jsonValue["data"] = dataValue;
		return Publish("v1/gateway/attributes", jsonValue.toString());
	}
	return CODE_ERROR;
}

int CloudProtocol::DeviceTelemetry(Device *device, Json::Value &dataValue)
{
	return DeviceTelemetry(device->getId(), dataValue);
}

int CloudProtocol::DeviceTelemetry(string deviceId, Json::Value &dataValue)
{
	if (!dataValue.isNull() && dataValue.isArray())
	{
		telemetryValueMtx.lock();
		if (telemetryValue.isObject())
		{
			if (!telemetryValue.isMember(deviceId))
			{
				telemetryValue[deviceId] = Json::arrayValue;
			}
			for (const auto &v : dataValue)
				telemetryValue[deviceId].append(v);
			telemetryValueLastUpdate = Util::millis();
		}
		telemetryValueMtx.unlock();
	}
	return CODE_ERROR;
}

int CloudProtocol::DeviceAttributeRequest(string data)
{
	return Publish("v1/devices/me/attributes/request/" + to_string(++msgId), data);
}

int CloudProtocol::FwRequestChunk(int chunkId)
{
	this->chunkId = chunkId;
	return Publish("v2/fw/request/" + to_string(++msgOtaId) + "/chunk/" + to_string(chunkId), STR(OTA_CHUNK_SIZE));
}

int CloudProtocol::PublishToCloudMessage(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout)
{
	return -1;
}

int CloudProtocol::SendTimeoutProvisionDev()
{
	Json::Value jsonValue;
	jsonValue["method"] = "timeoutScan";
	Json::Value params = Json::objectValue;
	// params["mac"] = mac;
	jsonValue["params"] = params;
	return Publish("v1/devices/me/rpc/request/" + to_string(++msgId), jsonValue.toString());
}