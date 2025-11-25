#include "ModuleDoorHangOn.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleDoorHangOn::ModuleDoorHangOn(Device *device, uint16_t addr) : Module(device, addr)
{
	hangOn = 0;
}

ModuleDoorHangOn::~ModuleDoorHangOn()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDoorHangOn::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_HANGON)
		hangOn = value;
}

void ModuleDoorHangOn::SaveAttribute()
{
	Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_HANGON, hangOn);
}
#endif

int ModuleDoorHangOn::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_HANGON) && dataValue[KEY_ATTRIBUTE_HANGON].isInt())
	{
		hangOn = dataValue[KEY_ATTRIBUTE_HANGON].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleDoorHangOn::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint8_t hangOn;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_STATUS_HANGON_DOOR_SENSOR)
	{
		if (hangOn != data_message->hangOn)
		{
			hangOn = data_message->hangOn;
#ifdef CONFIG_SAVE_ATTRIBUTE
			SaveAttribute();
#endif
		}
		BuildTelemetryValue(jsonValue);
		CheckTrigger(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleDoorHangOn::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_HANGON) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_HANGON].isInt())
		{
			int hangOn = dataValue[KEY_ATTRIBUTE_HANGON].asInt();
			rs = Util::CompareNumber(op, this->hangOn, hangOn);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_HANGON].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_HANGON];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int hangOn1 = listValue[0].asInt();
				int hangOn2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->hangOn, hangOn1, hangOn2);
				return true;
			}
		}
	}
	return false;
}

void ModuleDoorHangOn::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_HANGON] = hangOn;
}
