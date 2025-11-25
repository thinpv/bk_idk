#include "AndroidBleProtocol.h"
#include "Gateway.h"
#include "Log.h"
#include "Util.h"

#ifdef CONFIG_ENABLE_BLE
#include "BleProtocol.h"
#endif

#ifdef CONFIG_ENABLE_MQTT
#include "DeviceMqtt.h"
#endif

AndroidBleProtocol *androidBleProtocol = NULL;

AndroidBleProtocol::AndroidBleProtocol()
{
}

AndroidBleProtocol::~AndroidBleProtocol()
{
}

void AndroidBleProtocol::init()
{
	subTopic = "androidBle/HC";
	pubTopic = "HC/androidBle";
	Gateway::GetInstance()->localAddActionCallback(bind(&AndroidBleProtocol::OnMessage, this, placeholders::_1, placeholders::_2), subTopic);
	// Gateway::GetInstance()->localAddActionCallback(bind(&AndroidBleProtocol::OnAndroidBleResp, this, placeholders::_1, placeholders::_2), subTopic);
	OnAndroidBleProtocolCallbackRegister("bleInfo", bind(&AndroidBleProtocol::OnBleInfo, this, placeholders::_1, placeholders::_2));
	OnAndroidBleProtocolCallbackRegister("newDev", bind(&AndroidBleProtocol::OnNewDevice, this, placeholders::_1, placeholders::_2));
	OnAndroidBleProtocolCallbackRegister("DeviceStatus", bind(&AndroidBleProtocol::OnDeviceStatus, this, placeholders::_1, placeholders::_2));
	// OnAndroidBleProtocolCallbackRegister("provisionNormal", bind(&AndroidBleProtocol::OnProvisionNormal, this, placeholders::_1, placeholders::_2));
}

int AndroidBleProtocol::StartScan()
{
	string cmd = "startScanBle";
	Json::Value dataRequest = Json::objectValue;
	dataRequest["netKey"] = Gateway::GetInstance()->getBleNetKey();
	dataRequest["appKey"] = Gateway::GetInstance()->getBleAppKey();
	dataRequest["ivIndex"] = Gateway::GetInstance()->getBleIvIndex();
	dataRequest["addrGw"] = Gateway::GetInstance()->getBleAddr();
	dataRequest["addProvision"] = Gateway::GetInstance()->GetNextAndroidProvisionAddr();

	Json::Value element = Json::arrayValue;
	element.append(BLE_SWITCH_RGB_1);
	element.append(BLE_SWITCH_WATER_HEATER);
	element.append(BLE_SWITCH_RGB_WATER_HEATER);
	element.append(BLE_SWITCH_RGB_1_SQUARE);
	element.append(BLE_SWITCH_RGB_1_V2);
	element.append(BLE_SWITCH_RGB_1_SQUARE_V2);
	element.append(BLE_SWITCH_CURTAIN);
	element.append(BLE_SWITCH_RGB_CURTAIN);
	element.append(BLE_SWITCH_RGB_CURTAIN_SQUARE);
	element.append(BLE_SWITCH_RGB_CURTAIN_HCN);
	element.append(BLE_SWITCH_RGB_CURTAIN_SQUARE_V2);
	element.append(BLE_SWITCH_ROOLING_DOOR);
	element.append(BLE_SWITCH_ROOLING_DOOR_V2);
	element.append(BLE_SWITCH_ROOLING_DOOR_SQUARE);
	element.append(BLE_SWITCH_ELECTRICAL_1);
	element.append(BLE_SWITCH_ELECTRICAL_1_V2);
	element.append(BLE_SWITCH_RGB_SOCKET_1);
	element.append(BLE_SWITCH_ONOFF_V2);
	element.append(BLE_SWITCH_ONOFF);
	element.append(BLE_REPEATER);
	element.append(BLE_WIFI_SWITCH_1);
	element.append(BLE_WIFI_SWITCH_1_SQUARE);
	element.append(BLE_WIFI_SWITCH_ELECTRICAL_1);
	element.append(BLE_WIFI_SWITCH_ROOLING_DOOR);
	element.append(BLE_WIFI_SWITCH_ROOLING_DOOR_SQUARE);
	element.append(BLE_WIFI_SWITCH_CURTAIN);
	element.append(BLE_WIFI_SWITCH_CURTAIN_SQUARE);

	Json::Value secondElements = Json::arrayValue;
	secondElements.append(BLE_DOWNLIGHT_SMT);
	secondElements.append(BLE_DOWNLIGHT_COB_GOC_RONG);
	secondElements.append(BLE_DOWNLIGHT_COB_GOC_HEP);
	secondElements.append(BLE_DOWNLIGHT_COB_TRANG_TRI);
	secondElements.append(BLE_PANEL_TRON);
	secondElements.append(BLE_PANEL_VUONG);
	secondElements.append(BLE_LED_OP_TRAN);
	secondElements.append(BLE_LED_OP_TUONG);
	secondElements.append(BLE_LED_CHIEU_TRANH);
	secondElements.append(BLE_TRACKLIGHT);
	secondElements.append(BLE_LED_THA_TRAN);
	secondElements.append(BLE_LED_CHIEU_GUONG);
	secondElements.append(BLE_LED_DAY_LINEAR);
	secondElements.append(BLE_LED_TUBE_M16);
	secondElements.append(BLE_DEN_BAN);
	secondElements.append(BLE_LED_FLOOD);
	secondElements.append(BLE_LED_AT39);
	secondElements.append(BLE_LED_AT40);
	secondElements.append(BLE_LED_AT41);
	secondElements.append(BLE_LED_RLT03_06W);
	secondElements.append(BLE_LED_RLT02_10W);
	secondElements.append(BLE_LED_RLT02_20W);
	secondElements.append(BLE_LED_RLT01_10W);
	secondElements.append(BLE_LED_TRL08_20W);
	secondElements.append(BLE_LED_TRL08_10W);
	secondElements.append(BLE_LED_RLT03_12W);
	secondElements.append(BLE_LED_OP_TRAN_LOA);

	secondElements.append(BLE_SWITCH_RGB_2);
	secondElements.append(BLE_SWITCH_RGB_2_SQUARE);
	secondElements.append(BLE_SWITCH_RGB_2_V2);
	secondElements.append(BLE_SWITCH_RGB_2_SQUARE_V2);
	secondElements.append(BLE_SWITCH_2_CEILING);
	secondElements.append(BLE_SWITCH_ELECTRICAL_2);
	secondElements.append(BLE_SWITCH_ELECTRICAL_2_V2);
	secondElements.append(BLE_WIFI_SWITCH_2);
	secondElements.append(BLE_WIFI_SWITCH_2_SQUARE);
	secondElements.append(BLE_WIFI_SWITCH_ELECTRICAL_2);

	Json::Value thirdElements = Json::arrayValue;
	thirdElements.append(BLE_LED_DAY_RGB);
	thirdElements.append(BLE_SWITCH_RGB_3);
	thirdElements.append(BLE_SWITCH_RGB_3_SQUARE);
	thirdElements.append(BLE_SWITCH_RGB_3_V2);
	thirdElements.append(BLE_SWITCH_RGB_3_SQUARE_V2);
	thirdElements.append(BLE_SWITCH_3_CEILING);
	thirdElements.append(BLE_SWITCH_ELECTRICAL_3);
	thirdElements.append(BLE_SWITCH_ELECTRICAL_3_V2);
	thirdElements.append(BLE_WIFI_SWITCH_3);
	thirdElements.append(BLE_WIFI_SWITCH_3_SQUARE);
	thirdElements.append(BLE_WIFI_SWITCH_ELECTRICAL_3);

	Json::Value fourthElements = Json::arrayValue;
	fourthElements.append(BLE_LED_DAY_RGBCW);
	fourthElements.append(BLE_LED_BULB);
	fourthElements.append(BLE_DOWNLIGHT_RGBCW);

	fourthElements.append(BLE_SWITCH_RGB_4);
	fourthElements.append(BLE_SWITCH_RGB_4_SQUARE);
	fourthElements.append(BLE_SWITCH_RGB_4_V2);
	fourthElements.append(BLE_SWITCH_RGB_4_SQUARE_V2);
	fourthElements.append(BLE_WIFI_SWITCH_4);
	fourthElements.append(BLE_WIFI_SWITCH_4_SQUARE);

	Json::Value fiveElements = Json::arrayValue;
	fiveElements.append(BLE_SWITCH_5_CEILING);

	dataRequest["mapTypeElement"]["1"] = element;
	dataRequest["mapTypeElement"]["2"] = secondElements;
	dataRequest["mapTypeElement"]["3"] = thirdElements;
	dataRequest["mapTypeElement"]["4"] = fourthElements;
	dataRequest["mapTypeElement"]["5"] = fiveElements;

	Json::Value dataResponse = Json::objectValue;
	return PublishToAndroidBleMessage(cmd, dataRequest, cmd, &dataResponse, 2000);
}

int AndroidBleProtocol::StopScan()
{
	string cmd = "stopScanBle";
	Json::Value dataRequest = Json::objectValue;
	Json::Value dataResponse;
	return PublishToAndroidBleMessage(cmd, dataRequest, cmd, &dataResponse, 2000);
}

int AndroidBleProtocol::OnAndroidBleProtocolCallbackRegister(string cmd, OnAndroidBleProtocolCallbackFunc onAndroidBleProtocolCallbackFunc)
{
	LOGI("OnAndroidBleProtocolCallbackRegister cmd: %s", cmd.c_str());
	onAndroidBleProtocolCallbackFuncList[cmd] = onAndroidBleProtocolCallbackFunc;
	return CODE_OK;
}

void AndroidBleProtocol::OnMessage(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
		payloadJson.isMember("cmd") && payloadJson["cmd"].isString() &&
		payloadJson.isMember("rqi") && payloadJson["rqi"].isString() &&
		payloadJson.isMember("data") && payloadJson["data"].isObject())
	{
		string cmd = payloadJson["cmd"].asString();
		string rqi = payloadJson["rqi"].asString();
		if (onAndroidBleProtocolCallbackFuncList.find(cmd) != onAndroidBleProtocolCallbackFuncList.end())
		{
			OnAndroidBleProtocolCallbackFunc onAndroidBleProtocolCallbackFunc = onAndroidBleProtocolCallbackFuncList[cmd];
			int rs = onAndroidBleProtocolCallbackFunc(payloadJson["data"], respValue);
			if (rs == CODE_OK)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				respValue["rqi"] = rqi;
				Gateway::GetInstance()->LocalPublish(pubTopic, respValue.toString());
			}
			else if (rs == CODE_DATA_ARRAY)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				if (respValue.isArray())
				{
					for (auto &respV : respValue)
					{
						respV["rqi"] = rqi;
						Gateway::GetInstance()->LocalPublish(pubTopic, respV.toString());
					}
				}
			}
			else if (rs == CODE_NOT_RESPONSE)
			{
				LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
			}
			else
			{
				LOGW("Call %s ERR rs: %d", cmd.c_str(), rs);
			}
		}
		else
		{
			LOGW("Method %s not registed", cmd.c_str());
			LOGW("OnMessage payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnMessage topic: %s", topic.c_str());
		LOGW("OnMessage payload: %s", payload.c_str());
	}
}

int AndroidBleProtocol::SendMessage(string data)
{
	return CODE_OK;
}

int AndroidBleProtocol::OnBleInfo(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("BleInfo Request");
	respValue["cmd"] = "bleInfoRsp";
	respValue["data"]["code"] = CODE_OK;
	respValue["data"]["netKey"] = Gateway::GetInstance()->getBleNetKey();
	respValue["data"]["appKey"] = Gateway::GetInstance()->getBleAppKey();
	respValue["data"]["ivIndex"] = Gateway::GetInstance()->getBleIvIndex();
	respValue["data"]["addrGw"] = Gateway::GetInstance()->getBleAddr();
	return CODE_OK;
}

int AndroidBleProtocol::OnProvisionNormal(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Start provision normal");
	int rs = CODE_ERROR;
	respValue["cmd"] = "provisionNormal";
#ifdef CONFIG_ENABLE_BLE
	BleProtocol::GetInstance()->SetProvisioning(true);
	BleProtocol::GetInstance()->StartScan();
	rs = CODE_OK;
#endif
	respValue["data"]["code"] = rs;
	return CODE_OK;
}

int AndroidBleProtocol::OnNewDevice(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnNewDevice");
	respValue["data"]["code"] = CODE_OK;
	if (reqValue.isMember("device") && reqValue["device"].isArray())
	{
		Json::Value deviceJsonList = reqValue["device"];
		for (auto deviceJson : deviceJsonList)
		{
			if (deviceJson.isMember("id") && deviceJson["id"].isString() &&
				deviceJson.isMember("mac") && deviceJson["mac"].isString() &&
				deviceJson.isMember("addr") && deviceJson["addr"].isInt())
			{
				string uuid = deviceJson["id"].asString();
				string mac = deviceJson["mac"].asString();
				uint32_t addr = deviceJson["addr"].asInt();
				uint16_t vid;
				uint16_t pid;
				string devKey;
				Json::Value dataJson;
				if (deviceJson.isMember("data") && deviceJson["data"].isObject())
				{
					dataJson = deviceJson["data"];
					if (dataJson.isMember("devicekey") && dataJson["devicekey"].isString() &&
						dataJson.isMember("vid") && dataJson["vid"].isInt() &&
						dataJson.isMember("pid") && dataJson["pid"].isInt())
					{
						devKey = dataJson["devicekey"].asString();
						pid = dataJson["pid"].asInt();
						vid = dataJson["vid"].asInt();

						// 						uint32_t deviceType = Device::ConverPidToDeviveType(pid);
						// 						Device *device = Gateway::GetInstance()->AddDevice(uuid, Util::setString(Device::ConvertDeviceTypeToName(deviceType)), mac, dataJson, addr, deviceType, vid);
						// 						if (device)
						// 						{
						// 							Database::GetInstance()->DeviceAdd(device);
						// #ifdef CONFIG_ENABLE_BLE
						// 							BleProtocol::GetInstance()->UpdateDeviceKeyDev(addr, devKey);
						// #endif
						// 						}
						// 						else
						// 						{
						// 							respValue["data"]["code"] = CODE_MEMORY_ERROR;
						// 							LOGW("New device id %s err", uuid.c_str());
						// 						}
					}
				}
			}
			else
			{
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
				LOGW("OnDeviceStatus %s format error", reqValue.toString().c_str());
			}
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnNewDevice %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "OnNewDeviceRsp";
	return CODE_OK;
}

int AndroidBleProtocol::OnDeviceStatus(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnDeviceStatus");
	respValue["data"]["code"] = CODE_OK;
	if (reqValue.isMember("device") && reqValue["device"].isArray())
	{
		Json::Value deviceJsonList = reqValue["device"];
		for (auto deviceJson : deviceJsonList)
		{
			if (deviceJson.isMember("id") && deviceJson["id"].isString() &&
				deviceJson.isMember("data") && deviceJson["data"].isObject())
			{
				string deviceId = deviceJson["id"].asString();
				Json::Value devData = deviceJson["data"];
				Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
				if (device)
				{
					device->InputData(devData);
				}
				else
				{
					respValue["data"]["code"] = CODE_NOT_FOUND_DEVICE;
					LOGW("Device id %s not found", deviceId.c_str());
				}
			}
			else
			{
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
				LOGW("OnDeviceStatus %s format error", reqValue.toString().c_str());
			}
		}
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
		LOGW("OnDeviceStatus %s format error", reqValue.toString().c_str());
	}
	respValue["cmd"] = "DeviceStatusRsp";
	return CODE_OK;
}

int AndroidBleProtocol::PublishToAndroidBleMessage(string reqCmd, Json::Value &reqValue, string respCmd, Json::Value *respValue, uint32_t timeout)
{
	// LOGD("PublishToAndroidBleMessage: %s", reqValue.toString().c_str());
	int rs = CODE_OK;
	uint32_t countDelay = timeout / 10;
	Json::Value sendValue;
	string rqi = Util::genRandRQI(16);
	sendValue["data"] = reqValue;
	sendValue["rqi"] = rqi;
	sendValue["cmd"] = reqCmd;
	request_t request = {
		.status = false,
		.respCmd = respCmd,
		.respValue = respValue,
	};
	requestList[rqi] = &request;
	if (request.pubTopic == "")
	{
		request.pubTopic = pubTopic;
	}
	LOGD("PublishToAndroidBleMessage: Topic: %s: msg: %s", request.pubTopic.c_str(), (sendValue.toString()).c_str());
	Gateway::GetInstance()->LocalPublish(request.pubTopic, sendValue.toString());
	while (!request.status && countDelay--)
	{
		SLEEP_MS(10);
	}
	if (!request.status)
	{
		rs = CODE_ERROR;
	}
	requestList.erase(rqi);
	LOGD("PublishToAndroidBleMessage rs: %d", rs);
	return rs;
}

void AndroidBleProtocol::OnAndroidBleResp(string &topic, string &payload)
{
	Json::Value respValue;
	Json::Value payloadJson;
	if (payloadJson.parse(payload) && payloadJson.isObject() &&
		payloadJson.isMember("cmd") && payloadJson["cmd"].isString() &&
		payloadJson.isMember("rqi") && payloadJson["rqi"].isString())
	{
		string cmd = payloadJson["cmd"].asString();
		string rqi = payloadJson["rqi"].asString();
		if (requestList.find(rqi) != requestList.end())
		{
			request_t *request = requestList[rqi];
			if (cmd == request->respCmd)
			{
				request->status = true;
				if (request->respValue && payloadJson.isMember("data") && payloadJson["data"].isObject())
				{
					*request->respValue = payloadJson["data"];
				}
			}
		}
		else
		{
			LOGW("rqi %s not found", rqi.c_str());
			LOGW("OnAndroidBleResp payload: %s", payload.c_str());
		}
	}
	else
	{
		LOGW("OnAndroidBleResp error topic: %s, payload: %s", topic.c_str(), payload.c_str());
	}
}
