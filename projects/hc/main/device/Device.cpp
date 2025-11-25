#include "Device.h"
#include "Gateway.h"
#include "Log.h"
#include <string.h>
#include <functional>
#include <unistd.h>
#include <algorithm>
#include <Util.h>
#include "Database.h"

Device::Device(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *data)
		: Object(id, name, addr, time(NULL))
{
	this->mac = mac;
	this->type = type;
	if (data)
		this->dataValue = *data;
	this->sw_ver = version;
	powerSource = POWER_UNKNOWN;
	canAddToGroup = false;
	canAddToScene = false;
	propertyJsonUpdate = Json::objectValue;

	lastOnlineState = true;
	lastTimeActive = time(NULL);
	lastTimeCheckActive = 0;
}

Device::~Device()
{
}

string Device::GetDeviceKey()
{
	return "";
}

string Device::GetVersionStr()
{
	return to_string((sw_ver >> 8) & 0xFF) + "." + to_string(sw_ver & 0xFF);
}

bool Device::isOnline()
{
	return lastOnlineState;
}

bool Device::isNeedCheckOnline()
{
	return powerSource == POWER_AC;
}

int Device::BuildAttributeValue(Json::Value &pushDataValue)
{
	Json::Value deviceData;
	deviceData["gateway"] = "Farm Gateway RAL";
	deviceData["name"] = name;
	deviceData["mac"] = mac;
	deviceData["type"] = (int)type;
	pushDataValue[id] = deviceData;
	return CODE_OK;
}

void Device::DeviceInputData(uint8_t *data, int len, uint16_t addr)
{
	InputData(data, len, addr);
}

void Device::UpdateLastTimeActive()
{
	lastTimeActive = time(NULL);
}

int Device::DoJsonArray(Json::Value &dataValue)
{
	if (dataValue.isArray())
	{
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			Do(dataValue[i]);
		}
	}
	else
	{
		Do(dataValue);
	}
	return CODE_OK;
}

void Device::SetPropertyJsonUpdate(Json::Value property)
{
	if (this->propertyJsonUpdate != property)
	{
		PushTelemetry(property);
		this->propertyJsonUpdate = property;
	}
}

void Device::UpdatePropertyJsonUpdate(Json::Value &propertyUpdate)
{
	if (propertyUpdate.isObject() && this->propertyJsonUpdate.isObject())
	{
		for (auto const &key : propertyUpdate.getMemberNames())
		{
			if (this->propertyJsonUpdate.isMember(key))
			{
				if (propertyUpdate[key].type() == this->propertyJsonUpdate[key].type())
				{
					this->propertyJsonUpdate[key] = propertyUpdate[key];
				}
			}
		}
	}
}

Json::Value Device::GetPropertyJsonUpdate()
{
	return this->propertyJsonUpdate;
}

int Device::PushAttribute()
{
	Json::Value jsonValue;
	int build = BuildAttributeValue(jsonValue);
	if (build == 0)
	{
		return Gateway::GetInstance()->DeviceAttribute(this, jsonValue);
	}
	return CODE_ERROR;
}

int Device::PushAttribute(Json::Value &jsonValue)
{
	return Gateway::GetInstance()->DeviceAttribute(this, jsonValue);
}

int Device::PushTelemetry()
{
	Json::Value jsonValue;
	Json::Value devValue;
	Json::Value telemetryValue;
	int build = BuildTelemetryValue(telemetryValue);
	if (build == 0)
	{
		return PushTelemetry(telemetryValue);
	}
	return CODE_ERROR;
}

int Device::PushTelemetry(Json::Value &telemetryValue)
{
	if (!telemetryValue.isNull())
	{
		Json::Value jsonValue;
		Json::Value devValue;
		devValue["ts"] = Util::millis();
		devValue["values"] = telemetryValue;
		jsonValue.append(devValue);
		int rs = Gateway::GetInstance()->DeviceTelemetry(this, jsonValue);

		// // push to matter bridge
		// bool isMatterDevice = false;
		// switch (this->getType())
		// {
		// case MATTER_DOOR_SENSOR:
		// case MATTER_SWITCH_1:
		// case MATTER_SWITCH_1_V2:
		// case MATTER_SWITCH_3:
		// case MATTER_DOWNLIGHT_SMT:
		// case MATTER_DOWNLIGHT_RGBCW:
		// 	isMatterDevice = true;
		// 	break;
		// default:
		// 	break;
		// }

		// if (!isMatterDevice)
		// {
		// 	// Let the specific matter device class handle the telemetry
		// 	return CODE_OK;
		// }

		// // push to matter bridge
		// bool isMatterDevice2 = false;
		// Json::Value dataValue;
		// dataValue["rqi"] = Util::genRandRQI(16);
		// Json::Value attributeValue;
		// Json::Value deviceValue;
		// deviceValue["id"] = id;

		// switch (this->getType())
		// {
		// case MATTER_DOOR_SENSOR:
		// 	isMatterDevice2 = true;
		// 	dataValue["cmd"] = "sensorUpdate";
		// 	attributeValue["value"] = telemetryValue["door"];
		// 	break;
		// case MATTER_SWITCH_1:
		// case MATTER_SWITCH_1_V2:
		// case MATTER_SWITCH_3:
		// 	isMatterDevice2 = true;
		// 	dataValue["cmd"] = "touchSwitchUpdate";
		// 	attributeValue["onoff"] = telemetryValue["bt"];
		// 	break;
		// case MATTER_DOWNLIGHT_SMT:
		// case MATTER_DOWNLIGHT_RGBCW:
		// 	isMatterDevice2 = true;
		// 	dataValue["cmd"] = "lightUpdate";
		// 	/*
		// 	{
		// 		"cmd": "controlDev",
		// 		"rqi": "123456",
		// 		"data": {
		// 			"id": "b717f8d8-6f18-43c0-ae46-69c32998f653",
		// 			"data": {
		// 			"onoff": 1,
		// 			"dim": 80,
		// 			"ctt": 100,
		// 			"h": 50,
		// 			"l": 50,
		// 			"s": 10
		// 			}
		// 		}
		// 		}
		// 	*/

		// 	if (telemetryValue.isMember("onoff"))
		// 	{
		// 		attributeValue["onoff"] = telemetryValue["onoff"];
		// 	}
		// 	if (telemetryValue.isMember("dim"))
		// 	{
		// 		attributeValue["dim"] = telemetryValue["dim"];
		// 	}
		// 	if (telemetryValue.isMember("cct"))
		// 	{
		// 		attributeValue["cct"] = telemetryValue["cct"];
		// 	}
		// 	if (this->getType() == MATTER_DOWNLIGHT_RGBCW)
		// 	{
		// 		if (telemetryValue.isMember("h") && telemetryValue.isMember("s") && telemetryValue.isMember("l"))
		// 		{
		// 			attributeValue["h"] = telemetryValue["h"];
		// 			attributeValue["s"] = telemetryValue["s"];
		// 			attributeValue["l"] = telemetryValue["l"];
		// 		}
		// 	}
		// 	break;
		// default:
		// 	break;
		// }

		// if (isMatterDevice2)
		// {
		// 	deviceValue["data"] = attributeValue;
		// 	dataValue["data"] = deviceValue;
		// 	string topic = "v1/hc/bridge/request";
		// 	std::string payload = dataValue.toString();
		// 	int rsm = Gateway::GetInstance()->OnHcBridgeRpc(topic, payload);
		// 	if (rsm != CODE_OK)
		// 	{
		// 		LOGW("update telemetry for device matter error");
		// 	}
		// }
		return rs;
	}
	return CODE_ERROR;
}

// TODO: remove
static map<uint32_t, const char *> typeToNameList;
static map<uint32_t, const char *> typeToModelList;
static map<uint32_t, const char *> typeToOtherModelList;
static map<uint32_t, uint32_t> bleTypeToGroupIdList;
static map<uint16_t, const char *> bleAttributeIdToAttributeString;
static map<uint32_t, uint32_t> bleToMatterTypeMap; // New map for BLE to Matter type conversion

// TODO: Check list device to Name
void Device::InitDeviceModelList()
{
	bleTypeToGroupIdList[TYPE_GROUP_SWITCH] = BLE_SWITCH_TOUCH_GROUP;

	bleTypeToGroupIdList[BLE_DOWNLIGHT_SMT] = BLE_DOWNLIGHT_SMT_GROUP;
	bleTypeToGroupIdList[BLE_DOWNLIGHT_COB_GOC_RONG] = BLE_DOWNLIGHT_COB_GOC_RONG_GROUP;
	bleTypeToGroupIdList[BLE_DOWNLIGHT_COB_GOC_HEP] = BLE_DOWNLIGHT_COB_GOC_HEP_GROUP;
	bleTypeToGroupIdList[BLE_DOWNLIGHT_COB_TRANG_TRI] = BLE_DOWNLIGHT_COB_TRANG_TRI_GROUP;
	bleTypeToGroupIdList[BLE_PANEL_TRON] = BLE_PANEL_TRON_GROUP;
	bleTypeToGroupIdList[BLE_PANEL_VUONG] = BLE_PANEL_VUONG_GROUP;
	bleTypeToGroupIdList[BLE_LED_OP_TRAN] = BLE_LED_OP_TRAN_GROUP;
	bleTypeToGroupIdList[BLE_LED_OP_TUONG] = BLE_LED_OP_TUONG_GROUP;
	bleTypeToGroupIdList[BLE_LED_CHIEU_TRANH] = BLE_LED_CHIEU_TRANH_GROUP;
	bleTypeToGroupIdList[BLE_TRACKLIGHT] = BLE_TRACKLIGHT_GROUP;
	bleTypeToGroupIdList[BLE_LED_THA_TRAN] = BLE_LED_THA_TRAN_GROUP;
	bleTypeToGroupIdList[BLE_LED_CHIEU_GUONG] = BLE_LED_CHIEU_GUONG_GROUP;
	bleTypeToGroupIdList[BLE_LED_DAY_LINEAR] = BLE_LED_DAY_LINEAR_GROUP;
	bleTypeToGroupIdList[BLE_LED_TUBE_M16] = BLE_LED_TUBE_M16_GROUP;
	bleTypeToGroupIdList[BLE_DEN_BAN] = BLE_DEN_BAN_GROUP;
	bleTypeToGroupIdList[BLE_LED_FLOOD] = BLE_LED_FLOOD_GROUP;
	bleTypeToGroupIdList[BLE_LED_DAY_RGB] = BLE_LED_DAY_RGB_GROUP;
	bleTypeToGroupIdList[BLE_LED_DAY_RGBCW] = BLE_LED_DAY_RGBCW_GROUP;
	bleTypeToGroupIdList[BLE_LED_BULB] = BLE_LED_BULB_GROUP;
	bleTypeToGroupIdList[BLE_DOWNLIGHT_RGBCW] = BLE_DOWNLIGHT_RGBCW_GROUP;
	bleTypeToGroupIdList[BLE_LED_OP_TRAN_LOA] = BLE_LED_OP_TRAN_LOA_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT03_06W] = BLE_LED_RLT03_06W_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT02_10W] = BLE_LED_RLT02_10W_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT02_20W] = BLE_LED_RLT02_20W_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT01_10W] = BLE_LED_RLT01_10W_GROUP;
	bleTypeToGroupIdList[BLE_LED_TRL08_20W] = BLE_LED_TRL08_20W_GROUP;
	bleTypeToGroupIdList[BLE_LED_TRL08_10W] = BLE_LED_TRL08_10W_GROUP;
	bleTypeToGroupIdList[BLE_LED_RLT03_12W] = BLE_LED_RLT03_12W_GROUP;
	bleTypeToGroupIdList[BLE_SWITCH_ONOFF] = BLE_SWITCH_ONOFF_GROUP;
	bleTypeToGroupIdList[BLE_SWITCH_ONOFF_V2] = BLE_SWITCH_ONOFF_V2_GROUP;

	// bleTypeToGroupIdList[BLE_SWITCH_RGB_1] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_3] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_4] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_1_SQUARE] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_2_SQUARE] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_3_SQUARE] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_4_SQUARE] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_1_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_1_SQUARE_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_2_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_2_SQUARE_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_3_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_3_SQUARE_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_RGB_4_V2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_1] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_2] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_3] = BLE_SWITCH_TOUCH_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_4] = BLE_SWITCH_TOUCH_GROUP;

	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_1] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_3] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_1_V2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_2_V2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_ELECTRICAL_3_V2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_ELECTRICAL_1] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_ELECTRICAL_2] = BLE_SWITCH_ELECTRICAL_GROUP;
	// bleTypeToGroupIdList[BLE_WIFI_SWITCH_ELECTRICAL_3] = BLE_SWITCH_ELECTRICAL_GROUP;

	// bleTypeToGroupIdList[BLE_SWITCH_2_CEILING] = BLE_SWITCH_CEILING_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_3_CEILING] = BLE_SWITCH_CEILING_GROUP;
	// bleTypeToGroupIdList[BLE_SWITCH_5_CEILING] = BLE_SWITCH_CEILING_GROUP;

	bleTypeToGroupIdList[BLE_SWITCH_RGB_CURTAIN_HCN] = BLE_SWITCH_CURTAIN_GROUP;
	bleTypeToGroupIdList[BLE_SWITCH_RGB_CURTAIN_SQUARE_V2] = BLE_SWITCH_CURTAIN_GROUP;
	bleTypeToGroupIdList[BLE_LED_AT39] = BLE_LED_AT39_GROUP;
	bleTypeToGroupIdList[BLE_LED_AT40] = BLE_LED_AT40_GROUP;
	bleTypeToGroupIdList[BLE_LED_AT41] = BLE_LED_AT41_GROUP;

#ifdef CONFIG_ENABLE_BLE
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_ONOFF] = KEY_ATTRIBUTE_ONOFF;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DIM] = KEY_ATTRIBUTE_DIM;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CCT] = KEY_ATTRIBUTE_CCT;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_HUE] = KEY_ATTRIBUTE_HUE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SATURATION] = KEY_ATTRIBUTE_SATURATION;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_LUMINANCE] = KEY_ATTRIBUTE_LUMINANCE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SONG] = KEY_ATTRIBUTE_SONG;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BLINK_MODE] = KEY_ATTRIBUTE_BLINK_MODE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BATTERY] = KEY_ATTRIBUTE_BATTERY;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_LUX] = KEY_ATTRIBUTE_LUX;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_PIR] = KEY_ATTRIBUTE_PIR;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_1] = KEY_ATTRIBUTE_BUTTON;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_2] = KEY_ATTRIBUTE_BUTTON "2";
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_3] = KEY_ATTRIBUTE_BUTTON "3";
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_4] = KEY_ATTRIBUTE_BUTTON "4";
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_5] = KEY_ATTRIBUTE_BUTTON "5";
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_BUTTON_6] = KEY_ATTRIBUTE_BUTTON "6";
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_ACTIME] = KEY_ATTRIBUTE_ACTIME;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_PM2_5] = KEY_ATTRIBUTE_PM2_5;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_PM10] = KEY_ATTRIBUTE_PM10;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_PM1_0] = KEY_ATTRIBUTE_PM1_0;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_TEMP] = KEY_ATTRIBUTE_TEMP;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_HUMIDITY] = KEY_ATTRIBUTE_HUMIDITY;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SCENE_RGB] = KEY_ATTRIBUTE_MODE_RGB;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_HANGON] = KEY_ATTRIBUTE_HANGON;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_COUNTDOWN] = KEY_ATTRIBUTE_COUNTDOWN;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_AIR_CONDITIONER_WIND] = KEY_ATTRIBUTE_AIR_CONDITIONER_WIND;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_AIR_CONDITIONER_MODE] = KEY_ATTRIBUTE_AIR_CONDITIONER_MODE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_AIR_CONDITIONER_TEMP] = KEY_ATTRIBUTE_AIR_CONDITIONER_TEMP;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CURTAIN_OPEN] = KEY_ATTRIBUTE_CURTAIN_OPEN;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CURTAIN_CLOSE] = KEY_ATTRIBUTE_CURTAIN_CLOSE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CURTAIN_PAUSE] = KEY_ATTRIBUTE_CURTAIN_PAUSE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CURTAIN_OPENED] = KEY_ATTRIBUTE_CURTAIN_OPENED;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SMOKE] = KEY_ATTRIBUTE_SMOKE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DOOR] = KEY_ATTRIBUTE_DOOR;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SMOKE_PIN] = KEY_ATTRIBUTE_SMOKE_PIN;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DKTX_SCENE] = KEY_ATTRIBUTE_DKTX_SCENE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_ONLINE_OFFLINE] = KEY_ATTRIBUTE_ONLINE_OFFLINE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_MOTOR] = KEY_ATTRIBUTE_MOTOR;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_R] = KEY_ATTRIBUTE_R;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_G] = KEY_ATTRIBUTE_G;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_B] = KEY_ATTRIBUTE_B;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DIM_ON] = KEY_ATTRIBUTE_DIM_ON;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DIM_OFF] = KEY_ATTRIBUTE_DIM_OFF;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_SENSI_SENSOR] = KEY_ATTRIBUTE_SENSI;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_DISTANCE] = KEY_ATTRIBUTE_DISTANCE;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_INPUT_MODE] = KEY_ATTRIBUTE_MODE_INPUT;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_STARTUP] = KEY_ATTRIBUTE_STATUS_STARTUP;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_LOCKED] = KEY_ATTRIBUTE_LOCK;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_CALIB_AUTO] = KEY_ATTRIBUTE_CALIB_AUTO;
	// bleAttributeIdToAttributeString[BLE_ATTRIBUTE_MODE_WIFI] = KEY_ATTRIBUTE_MODE_WIFI;

	RegisterDeviceModel(BLE_DOWNLIGHT_SMT, "RD_BLE_DOWNLIGHT_SMT", "Downlight SMT");
	RegisterDeviceModel(BLE_DOWNLIGHT_COB_GOC_RONG, "RD_BLE_DOWNLIGHT_COB_GOC_RONG", "Downlight COB");
	RegisterDeviceModel(BLE_DOWNLIGHT_COB_GOC_HEP, "RD_BLE_DOWNLIGHT_COB_GOC_HEP", "Downlight trang trí");
	RegisterDeviceModel(BLE_DOWNLIGHT_COB_TRANG_TRI, "RD_BLE_DOWNLIGHT_COB_TRANG_TRI", "Downlight trang trí");
	RegisterDeviceModel(BLE_PANEL_TRON, "RD_BLE_PANEL_TRON", "Panel tròn");
	RegisterDeviceModel(BLE_PANEL_VUONG, "RD_BLE_PANEL_VUONG", "Panel");
	RegisterDeviceModel(BLE_LED_OP_TRAN, "RD_BLE_LED_OP_TRAN", "Ốp trần");
	RegisterDeviceModel(BLE_LED_OP_TUONG, "RD_BLE_LED_GAN_TUONG", "Đèn gắn tường");
	RegisterDeviceModel(BLE_LED_CHIEU_TRANH, "RD_BLE_LED_CHIEU_TRANH", "Đèn gắn tường");
	RegisterDeviceModel(BLE_TRACKLIGHT, "RD_BLE_TRACKLIGHT", "Tracklight");
	RegisterDeviceModel(BLE_LED_THA_TRAN, "RD_BLE_LED_THA_TRAN", "Đèn thả trần");
	RegisterDeviceModel(BLE_LED_CHIEU_GUONG, "RD_BLE_LED_CHIEU_GUONG", "Đèn chiếu gương");
	RegisterDeviceModel(BLE_LED_DAY_LINEAR, "RD_BLE_LED_DAY_LINEAR", "LED dây sáng trắng");
	RegisterDeviceModel(BLE_LED_TUBE_M16, "RD_BLE_LED_TUBE", "Đèn tube");
	RegisterDeviceModel(BLE_DEN_BAN, "RD_BLE_DEN_BAN", "Đèn bàn");
	RegisterDeviceModel(BLE_LED_FLOOD, "RD_BLE_LED_FLOOD", "Đèn trang trí");
	RegisterDeviceModel(BLE_LED_AT39, "RD_BLE_LED_AT39", "Downlight trang trí");
	RegisterDeviceModel(BLE_LED_AT40, "RD_BLE_LED_AT40", "Downlight trang trí");
	RegisterDeviceModel(BLE_LED_AT41, "RD_BLE_LED_AT41", "Downlight trang trí");
	RegisterDeviceModel(BLE_LED_RLT03_06W, "RD_BLE_LED_RLT03_06W", "Đèn ray LED thanh x/g đổi màu RLT03.BLE.CW 130/6W 48V");
	RegisterDeviceModel(BLE_LED_RLT02_10W, "RD_BLE_LED_RLT02_10W", "Đèn ray LED thanh đổi màu RLT02.BLE.CW 370/10W 48V");
	RegisterDeviceModel(BLE_LED_RLT02_20W, "RD_BLE_LED_RLT02_20W", "Đèn ray LED thanh đổi màu RTL02.BLE.CW 670/20W 48V");
	RegisterDeviceModel(BLE_LED_RLT01_10W, "RD_BLE_LED_RLT01_10W", "Đèn ray LED thanh đổi màu RLT01.BLE.CW 330/10W 48V");
	RegisterDeviceModel(BLE_LED_TRL08_20W, "RD_BLE_LED_TRL08_20W", "Đèn LED Tracklight đôi đổi màu TRL08.BLE.CW 20W 48V");
	RegisterDeviceModel(BLE_LED_TRL08_10W, "RD_BLE_LED_TRL08_10W", "Đèn LED Tracklight đổi màu TRL08.BLE.CW 10W 48V");
	RegisterDeviceModel(BLE_LED_RLT03_12W, "RD_BLE_LED_RLT03_12W", "Đèn ray LED thanh x/g đổi màu RLT03.BLE.CW 240/12W 48V");
	RegisterDeviceModel(BLE_LED_OP_TRAN_40W, "RD_BLE_LED_OP_TRAN_40W", "Đèn LED Ốp Trần 40W");
	RegisterDeviceModel(BLE_LED_DOWNLIGHT_NOI_TRAN, "RD_BLE_LED_NOI_TRAN", "Đèn LED Nối Trần");
	RegisterDeviceModel(BLE_LED_TRL05_25W, "RD_BLE_LED_TRL05_25W", "Đèn LED Tracklight đôi đổi màu TRL05.BLE.CW 25W");
	RegisterDeviceModel(BLE_LED_DAY_RGB, "RD_BLE_LED_DAY_RGB", "LED dây màu RGB");
	RegisterDeviceModel(BLE_LED_DAY_RGBCW, "RD_BLE_LED_DAY_RGBCW", "LED dây màu");
	RegisterDeviceModel(BLE_LED_BULB, "RD_BLE_LED_BULB", "LED bulb màu");
	RegisterDeviceModel(BLE_DOWNLIGHT_RGBCW, "RD_BLE_DOWNLIGHT_RGBCW", "Downlight màu");
	RegisterDeviceModel(BLE_LED_OP_TRAN_LOA, "RD_BLE_LED_OP_TRAN_LOA", "Ốp trần có loa");
	RegisterDeviceModel(BLE_LED_HIGHBAY, "RD_BLE_LED_HIGHBAY", "Đèn LED HighBay");
	RegisterDeviceModel(BLE_SWITCH_ONOFF, "RD_BLE_SWITCH_ONOFF", "Công tắc đèn");
	RegisterDeviceModel(BLE_SWITCH_ONOFF_V2, "RD_BLE_SWITCH_ONOFF_V2", "Công tắc đèn");
	RegisterDeviceModel(BLE_SWITCH_1, "RD_BLE_SWITCH_1", "Công tắc 1 nút");
	RegisterDeviceModel(BLE_SWITCH_2, "RD_BLE_SWITCH_2", "Công tắc 2 nút");
	RegisterDeviceModel(BLE_SWITCH_3, "RD_BLE_SWITCH_3", "Công tắc 3 nút");
	RegisterDeviceModel(BLE_SWITCH_4, "RD_BLE_SWITCH_4", "Công tắc 4 nút");
	RegisterDeviceModel(BLE_SWITCH_WATER_HEATER, "RD_BLE_SWITCH_WATER_HEATER", "Công tắc BNL");
	RegisterDeviceModel(BLE_SWITCH_CURTAIN, "RD_BLE_SWITCH_CURTAIN", "Công tắc rèm");
	RegisterDeviceModel(BLE_SWITCH_RGB_1, "RD_BLE_SWITCH_RGB_1", "Công tắc RGB 1 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_2, "RD_BLE_SWITCH_RGB_2", "Công tắc 2 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_3, "RD_BLE_SWITCH_RGB_3", "Công tắc 3 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_4, "RD_BLE_SWITCH_RGB_4", "Công tắc 4 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_WATER_HEATER, "RD_BLE_SWITCH_RGB_WATER_HEATER", "Công tắc BNL");
	RegisterDeviceModel(BLE_SWITCH_RGB_CURTAIN, "RD_BLE_SWITCH_RGB_CURTAIN", "Công tắc rèm");
	RegisterDeviceModel(BLE_SWITCH_ROOLING_DOOR, "RD_BLE_SWITCH_ROOLING_DOOR", "Công tắc cửa cuốn");
	RegisterDeviceModel(BLE_SWITCH_RGB_1_SQUARE, "RD_BLE_SWITCH_RGB_1_SQUARE", "Công tắc 1 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_2_SQUARE, "RD_BLE_SWITCH_RGB_2_SQUARE", "Công tắc 2 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_3_SQUARE, "RD_BLE_SWITCH_RGB_3_SQUARE", "Công tắc 3 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_4_SQUARE, "RD_BLE_SWITCH_RGB_4_SQUARE", "Công tắc 4 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_CURTAIN_SQUARE, "RD_BLE_SWITCH_RGB_CURTAIN_SQUARE", "Công tắc rèm");
	RegisterDeviceModel(BLE_SWITCH_ROOLING_DOOR_V2, "RD_BLE_SWITCH_ROOLING_DOOR_V2", "Công tắc của cuốn");

	RegisterDeviceModel(BLE_SWITCH_RGB_1_V2, "RD_BLE_SWITCH_RGB_1_V2", "Công tắc 1 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_1_SQUARE_V2, "RD_BLE_SWITCH_RGB_1_V2_SQUARE", "Công tắc 1 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_2_V2, "RD_BLE_SWITCH_RGB_2_V2", "Công tắc 2 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_2_SQUARE_V2, "RD_BLE_SWITCH_RGB_2_V2_SQUARE", "Công tắc 2 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_3_V2, "RD_BLE_SWITCH_RGB_3_V2", "Công tắc 3 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_3_SQUARE_V2, "RD_BLE_SWITCH_RGB_3_V2_SQUARE", "Công tắc 3 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_4_V2, "RD_BLE_SWITCH_RGB_4_V2", "Công tắc 4 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_4_SQUARE_V2, "RD_BLE_SWITCH_RGB_4_V2_SQUARE", "Công tắc 4 nút");
	RegisterDeviceModel(BLE_SWITCH_RGB_CURTAIN_HCN, "RD_BLE_SWITCH_CURTAIN_RECT", "Công tắc rèm");
	RegisterDeviceModel(BLE_SWITCH_RGB_CURTAIN_SQUARE_V2, "RD_BLE_SWITCH_CURTAIN_SQUARE", "Công tắc rèm");
	RegisterDeviceModel(BLE_SWITCH_ROOLING_DOOR_SQUARE, "RD_BLE_SWITCH_ROOLING_DOOR_SQUARE", "Công tắc cửa cuốn");
	RegisterDeviceModel(BLE_SWITCH_2_CEILING, "RD_BLE_SWITCH_CEILING_2", "Công tắc chuyển mạch âm trần 2 nút");
	RegisterDeviceModel(BLE_SWITCH_3_CEILING, "RD_BLE_SWITCH_CEILING_3", "Công tắc chuyển mạch âm trần 3 nút");
	RegisterDeviceModel(BLE_SWITCH_5_CEILING, "RD_BLE_SWITCH_CEILING_5", "Công tắc chuyển mạch âm trần 5 nút");
	RegisterDeviceModel(BLE_WIFI_SWITCH_1, "RD_BLE_WIFI_SWITCH_1", "Công tắc ble_wifi 1 nút");
	RegisterDeviceModel(BLE_WIFI_SWITCH_2, "RD_BLE_WIFI_SWITCH_2", "Công tắc ble_wifi 2 nút");
	RegisterDeviceModel(BLE_WIFI_SWITCH_3, "RD_BLE_WIFI_SWITCH_3", "Công tắc ble_wifi 3 nút");
	RegisterDeviceModel(BLE_WIFI_SWITCH_4, "RD_BLE_WIFI_SWITCH_4", "Công tắc ble_wifi 4 nút");
	RegisterDeviceModel(BLE_WIFI_SWITCH_ROOLING_DOOR, "RD_BLE_WIFI_SWITCH_ROOLING_DOOR", "Công tắc cửa cuốn ble_wifi");
	RegisterDeviceModel(BLE_WIFI_SWITCH_ROOLING_DOOR_SQUARE, "RD_BLE_WIFI_SWITCH_ROOLING_DOOR_SQUARE", "Công tắc cửa cuốn ble_wifi");
	RegisterDeviceModel(BLE_WIFI_SWITCH_CURTAIN, "RD_BLE_WIFI_SWITCH_CURTAIN", "Công tắc cửa cuốn ble_wifi");
	RegisterDeviceModel(BLE_WIFI_SWITCH_CURTAIN_SQUARE, "RD_BLE_WIFI_SWITCH_CURTAIN_SQUARE", "Công tắc cửa cuốn ble_wifi");
	RegisterDeviceModel(BLE_WIFI_SWITCH_1_SQUARE, "RD_BLE_WIFI_SWITCH_1_SQUARE", "Công tắc ble_wifi 1 nút");
	RegisterDeviceModel(BLE_WIFI_SWITCH_2_SQUARE, "RD_BLE_WIFI_SWITCH_2_SQUARE", "Công tắc ble_wifi 2 nút");
	RegisterDeviceModel(BLE_WIFI_SWITCH_3_SQUARE, "RD_BLE_WIFI_SWITCH_3_SQUARE", "Công tắc ble_wifi 3 nút");
	RegisterDeviceModel(BLE_WIFI_SWITCH_4_SQUARE, "RD_BLE_WIFI_SWITCH_4_SQUARE", "Công tắc ble_wifi 4 nút");

	RegisterDeviceModel(BLE_DC_SCENE_CONTACT, "RD_BLE_DC_SCENE_CONTACT", "DKTX M2");
	RegisterDeviceModel(BLE_AC_SCENE_CONTACT, "RD_BLE_AC_SCENE_CONTACT", "DKTX âm tường");
	RegisterDeviceModel(BLE_AC_SCENE_SCREEN_TOUCH, "RD_BLE_AC_SCENE_SCREEN_TOUCH", "Màn hình DKTX");
	RegisterDeviceModel(BLE_SWITCH_KNOB, "RD_BLE_SWITCH_KNOB", "Công tắc núm xoay");
	RegisterDeviceModel(BLE_REMOTE_M3, "RD_BLE_REMOTE_M3", "DKTX M3");
	RegisterDeviceModel(BLE_REMOTE_M3_V2, "RD_BLE_REMOTE_M3_V2", "DKTX M3 V2");
	RegisterDeviceModel(BLE_REMOTE_M4, "RD_BLE_REMOTE_M4", "DKTX M4");
	RegisterDeviceModel(BLE_AC_SCENE_CONTACT_RGB, "RD_BLE_AC_SCENE_CONTACT_RGB", "DKTX âm tường");
	RegisterDeviceModel(BLE_AC_SCENE_CONTACT_RGB_SQUARE, "RD_BLE_AC_SCENE_CONTACT_RGB_SQUARE", "DKTX âm tường vuông");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_1, "RD_BLE_SWITCH_ELECTRICAL_1", "Công tắc cơ 1 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_2, "RD_BLE_SWITCH_ELECTRICAL_2", "Công tắc cơ 2 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_3, "RD_BLE_SWITCH_ELECTRICAL_3", "Công tắc cơ 3 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_4, "RD_BLE_SWITCH_ELECTRICAL_4", "Công tắc cơ 4 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_WATER_HEATER, "RD_BLE_SWITCH_ELECTRICAL_WATER_HEATER", "Công tắc cơ BNL");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_1_V2, "RD_BLE_SWITCH_ELECTRICAL_1_V2", "Công tắc cơ 1 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_2_V2, "RD_BLE_SWITCH_ELECTRICAL_2_V2", "Công tắc cơ 2 nút");
	RegisterDeviceModel(BLE_SWITCH_ELECTRICAL_3_V2, "RD_BLE_SWITCH_ELECTRICAL_3_V2", "Công tắc cơ 3 nút");
	RegisterDeviceModel(BLE_WIFI_SWITCH_ELECTRICAL_1, "RD_BLE_WIFI_SWITCH_ELECTRICAL_1", "Công tắc cơ 1 nút ble_wifi");
	RegisterDeviceModel(BLE_WIFI_SWITCH_ELECTRICAL_2, "RD_BLE_WIFI_SWITCH_ELECTRICAL_2", "Công tắc cơ 2 nút ble_wifi");
	RegisterDeviceModel(BLE_WIFI_SWITCH_ELECTRICAL_3, "RD_BLE_WIFI_SWITCH_ELECTRICAL_3", "Công tắc cơ 3 nút ble_wifi");

	RegisterDeviceModel(BLE_SOCKET, "RD_BLE_SOCKET", "Ổ cắm đơn");
	RegisterDeviceModel(BLE_SOCKET_EXTEN, "RD_BLE_SOCKET_EXTEN", "Ổ cắm kéo dài");
	RegisterDeviceModel(BLE_SWITCH_RGB_SOCKET_1, "RD_BLE_SWITCH_RGB_SOCKET_1", "Ổ cắm công tắc chữ nhật");
	RegisterDeviceModel(BLE_SWITCH_RGB_SOCKET_1_V2, "RD_BLE_SWITCH_RGB_SOCKET_1_V2", "Ổ cắm công tắc");
	RegisterDeviceModel(BLE_SWITCH_RGB_SOCKET_2, "RD_BLE_SWITCH_RGB_SOCKET_2", "Ổ cắm công tắc");

	RegisterDeviceModel(BLE_SEFTPOWER_REMOTE_1, "RD_BLE_SEFTPOWER_REMOTE_1", "Công tắc 2 chiều không dây 1 nút");
	RegisterDeviceModel(BLE_SEFTPOWER_REMOTE_2, "RD_BLE_SEFTPOWER_REMOTE_2", "Công tắc 2 chiều không dây 2 nút");
	RegisterDeviceModel(BLE_SEFTPOWER_REMOTE_3, "RD_BLE_SEFTPOWER_REMOTE_3", "Công tắc 2 chiều không dây 3 nút");
	RegisterDeviceModel(BLE_SEFTPOWER_REMOTE_6, "RD_BLE_SEFTPOWER_REMOTE_4", "Công tắc 2 chiều không dây 6 nút");

	RegisterDeviceModel(BLE_LIGHT_SENSOR, "RD_BLE_LIGHT_SENSOR_V1", "Cảm biến ánh sáng");
	RegisterDeviceModel(BLE_LIGHT_SENSOR_AGRI, "RD_BLE_LIGHT_SENSOR", "Cảm biến ánh sáng");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_DC, "RD_BLE_PIR_LIGHT_SENSOR_DC", "Cảm biến chuyển động");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_AC, "RD_BLE_PIR_LIGHT_SENSOR_AC", "Cảm biến chuyển động AC");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_AC_AMTRAN, "RD_BLE_PIR_LIGHT_SENSOR_AC_AMTRAN", "Cảm biến chuyển động âm trần");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_DC_CB10, "RD_BLE_PIR_LIGHT_SENSOR_DC_CB10", "Cảm biến chuyển động CB10");
	RegisterDeviceModel(BLE_PIR_LIGHT_SENSOR_DC_CB09, "RD_BLE_PIR_LIGHT_SENSOR_DC_CB09", "Cảm biến chuyển động CB09");
	RegisterDeviceModel(BLE_RADA_LIGHT_SENSOR_AC_CB15, "RD_BLE_RADA_LIGHT_SENSOR_AC_CB15", "Cảm biến chuyển động CB015 rada");
	RegisterDeviceModel(BLE_SMOKE_SENSOR, "RD_BLE_SMOKE_SENSOR", "Cảm biến khói");
	RegisterDeviceModel(BLE_DOOR_SENSOR, "RD_BLE_DOOR_SENSOR", "Cảm biến cửa");
	RegisterDeviceModel(BLE_DOOR_CB16_SENSOR, "RD_BLE_DOOR_CB16_SENSOR", "Cảm biến cửa");
	RegisterDeviceModel(BLE_PM_SENSOR, "RD_BLE_PM_SENSOR", "Cảm biến bụi mịn");
	RegisterDeviceModel(BLE_TEMP_HUM_SENSOR, "RD_BLE_TEMP_HUM_SENSOR", "Cảm biến nhiệt/ẩm");

	RegisterDeviceModel(BLE_TEMP_HUM_AIR_AGRICULTURAL_SENSOR, "RD_BLE_TEMP_HUM_AIR_SENSOR", "Cảm biến nhiệt/ẩm");
	RegisterDeviceModel(BLE_TEMP_HUM_SOIL_AGRICULTURAL_SENSOR, "RD_BLE_TEMP_HUM_SOIL_SENSOR", "Cảm biến độ ẩm đất");
	RegisterDeviceModel(BLE_PH_SOIL_AGRICULTURAL_SENSOR, "RD_BLE_PH_SOIL_SENSOR", "Cảm biến PH đất");
	RegisterDeviceModel(BLE_EC_TEMP_HUM_SOIL_AGRICULTURAL_SENSOR, "RD_BLE_EC_SOIL_SENSOR", "Cảm biến EC đất");
	RegisterDeviceModel(BLE_PH_WATER_AGRICULTURAL_SENSOR, "RD_BLE_PH_WATER_SENSOR", "Cảm biến PH nước");
	RegisterDeviceModel(BLE_EC_WATER_AGRICULTURAL_SENSOR, "RD_BLE_EC_WATER_SENSOR", "Cảm biến EC nước");
	RegisterDeviceModel(BLE_OXY_WATER_AGRICULTURAL_SENSOR, "RD_BLE_OXY_WATER_SENSOR", "Cảm biến oxy nước");

	RegisterDeviceModel(BLE_REPEATER, "RD_BLE_REPEATER", "Bộ lặp sóng");
	RegisterDeviceModel(BLE_MODULE_INOUT, "RD_BLE_MODULE_INOUT", "Module inout");
#endif

	RegisterDeviceModel(WIFI_IR, "RD_WIFI_IR", "Điều khiển hồng ngoại");
	RegisterDeviceModel(WIFI_IR_AIRCONDITION, "RD_WIFI_IR_AIRCONDITION", "Điều hoà");
	RegisterDeviceModel(WIFI_IR_FAN, "RD_WIFI_IR_FAN", "Quạt");
	RegisterDeviceModel(WIFI_IR_TV, "RD_WIFI_IR_TV", "TIVI");
	RegisterDeviceModel(WIFI_SOCKET, "RD_WIFI_SOCKET", "Ổ cắm đơn Wifi");
	RegisterDeviceModel(WIFI_SOCKET_4, "RD_WIFI_SOCKET_4", "Ổ cắm thông minh Rạng Đông 4 cổng");
	RegisterDeviceModel(WIFI_SOCKET_6, "RD_WIFI_SOCKET_6", "Ổ cắm thông minh Rạng Đông 6 cổng");
	RegisterDeviceModel(WIFI_SWITCH_1, "RD_WIFI_SWITCH_1", "Công tắc Wifi 1 nút");
	RegisterDeviceModel(WIFI_SWITCH_2, "RD_WIFI_SWITCH_2", "Công tắc Wifi 2 nút");
	RegisterDeviceModel(WIFI_SWITCH_3, "RD_WIFI_SWITCH_3", "Công tắc Wifi 3 nút");
	RegisterDeviceModel(WIFI_SWITCH_4, "RD_WIFI_SWITCH_4", "Công tắc Wifi 4 nút");
	RegisterDeviceModel(WIFI_DOOR_LOCK, "RD_WIFI_DOOR_LOCK", "Khoá cửa Wifi");
	RegisterDeviceModel(CAMERA_TUYA, "RD_CAMERA_TUYA", "Camera Wifi");
	RegisterDeviceModel(CAMERA_DAHUA, "RD_CAMERA_DAHUA", "Camera Dahua");
	RegisterDeviceModel(CAMERA_HKVISION, "RD_CAMERA_HKVISION", "Camera HikVision");
	RegisterDeviceModel(AI_ZONE, "RD_AI_ZONE", "Zone");
	RegisterDeviceModel(AI_FACE, "RD_AI_FACE", "Face");

	RegisterDeviceModel(BLE_REPEATER, "RD_BLE_REPEATER", "Bộ lặp sóng");

	RegisterDeviceModel(MQTT_AI_HUB, "", "AiHub");

	// zigbee
	RegisterDeviceModel(ZIGBEE_LUMI_PLUG, "lumi.plug", "Ổ cắm đơn Zigbee");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_SWITCH, "lumi.sensor_switch", "Chuông cửa Zigbee");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_TEMP_HUM, "lumi.sensor_ht", "Cam biet nhiet do do am");
	RegisterDeviceOtherModel(ZIGBEE_LUMI_SENSOR_TEMP_HUM, "lumi.sens");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_WLEAK_AQ1, "lumi.sensor_wleak.aq1", "Cam bien ro nuoc");
	RegisterDeviceModel(ZIGBEE_LUMI_SENSOR_MAGNET, "lumi.sensor_magnet", "Cam bien cua");
	RegisterDeviceModel(ZIGBEE_LUMI_MOTION_AC01, "lumi.motion.ac01", "Cam bien hien dien");
	RegisterDeviceModel(ZIGBEE_TUYA_SENSOR_MAGNET_TY0203, "TY0203", "Cam bien cua Tuya");
	RegisterDeviceModel(ZIGBEE_TUYA_SENSOR_PIR_RH3040, "RH3040", "Cảm biến chuyển động Zigbee");
	RegisterDeviceModel(ZIGBEE_TUYA_SENSOR_HUMAN_PRESENCE_TS0225, "TS0225", "Cảm biến nhan dien nguoi Zigbee");

	RegisterDeviceModel(ZIGBEE_RAL_LIGHT_CCT_DIM, "ral.light.cct", "Led cct/dim");
	RegisterDeviceModel(ZIGBEE_RAL_SWITCH_1, "ral.switch.1", "Công tắc 1 nút Zigbee");
	RegisterDeviceModel(ZIGBEE_RAL_SWITCH_3, "ral.switch.3", "Công tắc 3 nút Zigbee");
	RegisterDeviceModel(ZIGBEE_RAL_SWITCH_5, "ral.switch.5", "Công tắc 5 nút Zigbee");
	RegisterDeviceModel(ZIGBEE_RAL_SENSOR_DOOR, "ral.sensor.door", "Cảm biến cửa Zigbee");
	RegisterDeviceModel(ZIGBEE_RAL_SENSOR_PIR_LIGHT_CB10, "ral.sensor.cb10", "Cảm biến chuyển động cb10");

	// modbus
	RegisterDeviceModel(MODBUS_SOIL_MOIS_SENSOR, "modbusMois", "CB do am dat");
	RegisterDeviceModel(MODBUS_RTU_RELAY, "modbusRTURelay", "relay mo rong");

	// matter
	// RegisterDeviceModel(MATTER_DOWNLIGHT_SMT, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_DOWNLIGHT_COB_GOC_RONG, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_DOWNLIGHT_COB_GOC_HEP, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_DOWNLIGHT_COB_TRANG_TRI, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_PANEL_TRON, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_PANEL_VUONG, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_LED_OP_TRAN, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_LED_CHIEU_TRANH, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_TRACKLIGHT, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_LED_THA_TRAN, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_LED_CHIEU_GUONG, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_LED_TUBE_M16, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_DEN_BAN, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_LED_FLOOD, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_LED_AT39, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_LED_AT40, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_PANEL_TRON, "268", "Downlight SMT");
	// RegisterDeviceModel(MATTER_LED_AT41, "268", "Downlight SMT");

	// RegisterDeviceModel(MATTER_DOWNLIGHT_RGBCW, "269", "Downlight màu");
	// RegisterDeviceModel(MATTER_LED_DAY_LINEAR, "269", "Downlight màu");

	// RegisterDeviceModel(MATTER_SWITCH_1, "259", "Công tắc RGB 1 nút");
	// RegisterDeviceModel(BLE_SWITCH_RGB_1_V2, "259", "Công tắc 1 nút");
	// // RegisterDeviceModel(MATTER_SWITCH_3, "259", "Công tắc RGB 3 nút");
	// RegisterDeviceModel(MATTER_DOOR_SENSOR, "21", "Cảm biến cửa");
	// // RegisterDeviceModel(MATTER_DOOR_SENSOR, "21", "Cảm biến cửa");
	// // RegisterDeviceModel(MATTER_DOOR_SENSOR, "21", "Cảm biến cửa");
	// // RegisterDeviceModel(MATTER_DOOR_SENSOR, "21", "Cảm biến cửa");
	// // RegisterDeviceModel(MATTER_DOOR_SENSOR, "21", "Cảm biến cửa");
	// // RegisterDeviceModel(MATTER_DOOR_SENSOR, "21", "Cảm biến cửa");
	// // RegisterDeviceModel(MATTER_DOOR_SENSOR, "21", "Cảm biến cửa");

	RegisterDeviceModel(DEVICE_UNKNOWN_TYPE, "Unknown", "Thiet bi khong xac dinh");

	// Add BLE to Matter type mappings
	// bleToMatterTypeMap[BLE_DOWNLIGHT_SMT] = MATTER_DOWNLIGHT_SMT;
	// bleToMatterTypeMap[BLE_DOWNLIGHT_RGBCW] = MATTER_DOWNLIGHT_RGBCW;
	// bleToMatterTypeMap[BLE_SWITCH_1] = MATTER_SWITCH_1;
	// bleToMatterTypeMap[BLE_SWITCH_3] = MATTER_SWITCH_3;
	// bleToMatterTypeMap[BLE_DOOR_SENSOR] = MATTER_DOOR_SENSOR;
}

void Device::RegisterDeviceModel(uint32_t type, const char *model, const char *name)
{
	typeToNameList[type] = name;
	typeToModelList[type] = model;
}

void Device::RegisterDeviceOtherModel(uint32_t type, const char *model)
{
	typeToOtherModelList[type] = model;
}

uint32_t Device::BleTypeToGroupId(uint32_t deviceType)
{
	return bleTypeToGroupIdList[deviceType];
}

const char *Device::BleAttributeIdToAttributeStr(uint16_t attributeId)
{
	return bleAttributeIdToAttributeString[attributeId];
}

const char *Device::ConvertDeviceTypeToName(uint32_t type)
{
	// TODO: check null when use
	return typeToNameList[type];
}

const char *Device::ConvertDeviceTypeToModel(uint32_t type)
{
	const char *model = typeToModelList[type];
	if (model)
		return model;
	else
		return "";
}

uint32_t Device::ConvertModelToDeviceType(const char *model)
{
	for (const auto &[_type, _model] : typeToModelList)
	{
		if (strcmp(model, _model) == 0)
			return _type;
	}
	for (const auto &[_type, _model] : typeToOtherModelList)
	{
		if (strcmp(model, _model) == 0)
			return _type;
	}
	return DEVICE_UNKNOWN_TYPE;
}

// New function to convert BLE type to Matter type
// uint32_t Device::ConvertBleToMatterType(uint32_t bleType)
// {
// 	auto it = bleToMatterTypeMap.find(bleType);
// 	if (it != bleToMatterTypeMap.end()) {
// 		return it->second;
// 	}
// 	return DEVICE_UNKNOWN_TYPE;
// }

void Device::InputData(uint8_t *data, int len, uint16_t epId)
{
	LOGI("InputData");
	if (protocol == BLE_DEVICE || protocol == ZIGBEE_DEVICE)
	{
		deviceInGroupUnconfigListMtx.lock();
		for (auto it = deviceInGroupUnconfigList.begin(); it != deviceInGroupUnconfigList.end(); ++it)
		{
			DeviceInGroup *deviceInGroup = *it;
			if (deviceInGroup->getIsInGroup())
			{
				if (!deviceInGroup->getIsConfigured())
				{
					AddToGroup(deviceInGroup->epId, deviceInGroup->groupAddr);
				}
			}
			else // if (!deviceInGroup->getIsInGroup())
			{
				if (deviceInGroup->getIsConfigured())
				{
					RemoveFromGroup(deviceInGroup->epId, deviceInGroup->groupAddr);
				}
			}
		}
		deviceInGroupUnconfigListMtx.unlock();

		deviceInSceneUnconfigListMtx.lock();
		for (auto it = deviceInSceneUnconfigList.begin(); it != deviceInSceneUnconfigList.end(); ++it)
		{
			DeviceInScene *deviceInScene = *it;
			if (deviceInScene->getIsInScene())
			{
				if (!deviceInScene->getIsConfigured())
				{
					AddToScene(deviceInScene->epId, deviceInScene->sceneAddr);
				}
			}
			else // if (!deviceInScene->getIsInScene())
			{
				if (deviceInScene->getIsConfigured())
				{
					RemoveFromScene(deviceInScene->epId, deviceInScene->sceneAddr);
				}
			}
		}
		deviceInSceneUnconfigListMtx.unlock();
	}
}

DeviceInGroup *Device::GetDeviceInGroupUnconfig(uint16_t epId, uint16_t groupAddr)
{
	LOGI("GetDeviceInGroupUnconfig epId %d, groupId: 0x%04X", epId, groupAddr);
	if (canAddToGroup)
	{
		deviceInGroupUnconfigListMtx.lock();
		for (auto it = deviceInGroupUnconfigList.begin(); it != deviceInGroupUnconfigList.end(); ++it)
		{
			DeviceInGroup *deviceInGroup = *it;
			if (deviceInGroup->epId == epId && deviceInGroup->groupAddr == groupAddr)
			{
				deviceInGroupUnconfigListMtx.unlock();
				return deviceInGroup;
			}
		}
		deviceInGroupUnconfigListMtx.unlock();
	}
	return NULL;
}

void Device::AddDeviceInGroupUnconfig(DeviceInGroup *deviceInGroup)
{
	LOGI("AddDeviceInGroupUnconfig device %s, epId: %d, groupId: %d", name.c_str(), deviceInGroup->epId, deviceInGroup->groupAddr);
	if (canAddToGroup && deviceInGroup)
	{
		deviceInGroupUnconfigListMtx.lock();
		deviceInGroupUnconfigList.push_back(deviceInGroup);
		deviceInGroupUnconfigListMtx.unlock();
	}
}

void Device::DelDeviceInGroupUnconfig(DeviceInGroup *deviceInGroup)
{
	LOGI("DelDeviceInGroupUnconfig device %s, epId: %d, groupId: %d", name.c_str(), deviceInGroup->epId, deviceInGroup->groupAddr);
	if (canAddToGroup && deviceInGroup)
	{
		deviceInGroupUnconfigListMtx.lock();
		for (auto it = deviceInGroupUnconfigList.begin(); it != deviceInGroupUnconfigList.end(); ++it)
		{
			DeviceInGroup *deviceInGroup_ = *it;
			if (deviceInGroup == deviceInGroup_)
			{
				deviceInGroupUnconfigList.erase(it);
				deviceInGroupUnconfigListMtx.unlock();
				return;
			}
		}
		deviceInGroupUnconfigListMtx.unlock();
	}
}

DeviceInScene *Device::GetDeviceInSceneUnconfig(uint16_t epId, uint16_t sceneAddr)
{
	LOGI("GetDeviceInSceneUnconfig epId %d, sceneId: 0x%04X", epId, sceneAddr);
	if (canAddToScene)
	{
		deviceInSceneUnconfigListMtx.lock();
		for (auto it = deviceInSceneUnconfigList.begin(); it != deviceInSceneUnconfigList.end(); ++it)
		{
			DeviceInScene *deviceInScene = *it;
			if (deviceInScene->epId == epId && deviceInScene->sceneAddr == sceneAddr)
			{
				deviceInSceneUnconfigListMtx.unlock();
				return deviceInScene;
			}
		}
		deviceInSceneUnconfigListMtx.unlock();
	}
	return NULL;
}

void Device::AddDeviceInSceneUnconfig(DeviceInScene *deviceInScene)
{
	LOGI("AddDeviceInSceneUnconfig device %s, sceneId: %d", name.c_str(), deviceInScene->sceneAddr);
	if (canAddToScene && deviceInScene)
	{
		deviceInSceneUnconfigListMtx.lock();
		deviceInSceneUnconfigList.push_back(deviceInScene);
		deviceInSceneUnconfigListMtx.unlock();
	}
}

void Device::DelDeviceInSceneUnconfig(DeviceInScene *deviceInScene)
{
	LOGI("DelDeviceInSceneUnconfig device %s, sceneId: %d", name.c_str(), deviceInScene->sceneAddr);
	if (canAddToScene && deviceInScene)
	{
		deviceInSceneUnconfigListMtx.lock();
		for (auto it = deviceInSceneUnconfigList.begin(); it != deviceInSceneUnconfigList.end(); ++it)
		{
			DeviceInScene *deviceInScene_ = *it;
			if (deviceInScene == deviceInScene_)
			{
				deviceInSceneUnconfigList.erase(it);
				deviceInSceneUnconfigListMtx.unlock();
				return;
			}
		}
		deviceInSceneUnconfigListMtx.unlock();
	}
}

int Device::OnAddToGroup(uint16_t epId, uint16_t groupAddr, int status)
{
	DeviceInGroup *deviceInGroup = GetDeviceInGroupUnconfig(epId, groupAddr);
	if (deviceInGroup)
	{
		deviceInGroup->setIsConfigured(true);
		DelDeviceInGroupUnconfig(deviceInGroup);
		Database::GetInstance()->DeviceInGroupUpdateData(deviceInGroup);
	}
	else
	{
		LOGW("Add device to group not in waiting list");
	}
	return CODE_OK;
}

int Device::OnRemoveFromGroup(uint16_t epId, uint16_t groupAddr, int status)
{
	DeviceInGroup *deviceInGroup = GetDeviceInGroupUnconfig(epId, groupAddr);
	if (deviceInGroup)
	{
		deviceInGroup->setIsConfigured(false);
		DelDeviceInGroupUnconfig(deviceInGroup);
		Database::GetInstance()->DeviceInGroupDel(deviceInGroup);
		delete deviceInGroup;
	}
	else
	{
		LOGW("Remove device in group not in waiting list");
	}
	return CODE_OK;
}

int Device::OnAddToScene(uint16_t epId, uint16_t sceneAddr, int status)
{
	DeviceInScene *deviceInScene = GetDeviceInSceneUnconfig(epId, sceneAddr);
	if (deviceInScene)
	{
		deviceInScene->setIsConfigured(true);
		DelDeviceInSceneUnconfig(deviceInScene);
		Database::GetInstance()->DeviceInSceneUpdateData(deviceInScene);
	}
	else
	{
		LOGW("Add device to scene not in waiting list");
	}
	return CODE_OK;
}

int Device::OnRemoveFromScene(uint16_t epId, uint16_t sceneAddr, int status)
{
	DeviceInScene *deviceInScene = GetDeviceInSceneUnconfig(epId, sceneAddr);
	if (deviceInScene)
	{
		deviceInScene->setIsConfigured(false);
		DelDeviceInSceneUnconfig(deviceInScene);
		Database::GetInstance()->DeviceInSceneDel(deviceInScene);
		delete deviceInScene;
	}
	else
	{
		LOGW("Remove device in scene not in waiting list");
	}
	return CODE_OK;
}