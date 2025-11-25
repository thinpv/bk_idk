#include "ModuleModeWifi.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleModeWifi::ModuleModeWifi(Device *device, uint32_t addr) : Module(device, addr)
{
	mode = 0;
}

ModuleModeWifi::~ModuleModeWifi()
{
}

int ModuleModeWifi::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_MODE_WIFI) && dataValue[KEY_ATTRIBUTE_MODE_WIFI].isInt())
	{
		mode = dataValue[KEY_ATTRIBUTE_MODE_WIFI].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleModeWifi::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t mode;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)&data[1];
	if (data_message->opcode == RD_OPCODE_CONFIG_RSP &&
		data_message->header == RD_HEADER_MODE_WIFI)
	{
		mode = data_message->mode;
		BuildTelemetryValue(jsonValue);
		CheckTrigger(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleModeWifi::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());

	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_MODE_INPUT) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_MODE_INPUT].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_MODE_INPUT].asInt();
			rs = Util::CompareNumber(op, this->mode, value);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_MODE_INPUT].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_MODE_INPUT];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int value1 = listValue[0].asInt();
				int value2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->mode, value1, value2);
				return true;
			}
		}
	}
	return false;
}

void ModuleModeWifi::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_MODE_WIFI] = mode;
}

int ModuleModeWifi::Do(Json::Value &dataValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_MODE_WIFI) && dataValue[KEY_ATTRIBUTE_MODE_WIFI].isInt())
	{
		int mode = dataValue[KEY_ATTRIBUTE_MODE_WIFI].asInt();
		if (BleProtocol::GetInstance()->SetModeWifi(addr, mode) == CODE_OK)
		{
			this->mode = mode;
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}