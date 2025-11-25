#include "ModuleDoorStatus.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleDoorStatus::ModuleDoorStatus(Device *device, uint16_t addr) : Module(device, addr)
{
	status = 0;
}

ModuleDoorStatus::~ModuleDoorStatus()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDoorStatus::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_DOOR)
		status = value;
}

void ModuleDoorStatus::SaveAttribute()
{
	Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_DOOR, status);
}
#endif

int ModuleDoorStatus::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_DOOR) && dataValue[KEY_ATTRIBUTE_DOOR].isInt())
	{
		status = dataValue[KEY_ATTRIBUTE_DOOR].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleDoorStatus::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint8_t door;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_STATUS_DOOR_SENSOR)
	{
		if (status != data_message->door)
		{
			status = data_message->door;
#ifdef CONFIG_SAVE_ATTRIBUTE
			SaveAttribute();
#endif
		}
#ifdef __ANDROID__
		if (status == 1 || status == 0)
		{
			string content = "cảnh báo cửa mở";
			if (status == 1)
			{
				content = "cảnh báo cửa đóng";
			}

			string id = Util::genRandRQI(16);
			Json::Value tempJson = Gateway::GetInstance()->BuildJsonDataNoti(device, id, "warning", content);
			Noti *temp = new Noti(id, "warning", tempJson.toString(), to_string(time(NULL)), to_string(time(NULL)));
			Gateway::GetInstance()->CreateNoti(temp, true, true);
		}
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleDoorStatus::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_DOOR) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_DOOR].isInt())
		{
			int status = dataValue[KEY_ATTRIBUTE_DOOR].asInt();
			rs = Util::CompareNumber(op, this->status, status);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_DOOR].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_DOOR];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int status1 = listValue[0].asInt();
				int status2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->status, status1, status2);
				return true;
			}
		}
	}
	return false;
}

void ModuleDoorStatus::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_DOOR] = status;
}
