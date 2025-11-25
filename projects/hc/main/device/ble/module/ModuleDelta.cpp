#include "ModuleDelta.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"
#include <math.h>

ModuleDelta::ModuleDelta(Device *device, uint16_t addr) : Module(device, addr)
{
	delta = 0;
}

ModuleDelta::~ModuleDelta()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDelta::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_DELTA)
		delta = value;
}

void ModuleDelta::SaveAttribute()
{
	Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_DELTA, delta);
}
#endif

int ModuleDelta::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_DELTA) && dataValue[KEY_ATTRIBUTE_DELTA].isInt())
	{
		delta = dataValue[KEY_ATTRIBUTE_DELTA].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleDelta::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t delta;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == RD_OPCODE_CONFIG_RSP && data_message->header == RD_HEADER_CONFIG_DELTA_ADC)
	{
		delta = data_message->delta;
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleDelta::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_DELTA) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_DELTA].isInt())
		{
			int delta = dataValue[KEY_ATTRIBUTE_DELTA].asInt();
			rs = Util::CompareNumber(op, this->delta, delta);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_DELTA].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_DELTA];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int delta1 = listValue[0].asInt();
				int delta2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->delta, delta1, delta2);
				return true;
			}
		}
	}
	return false;
}

void ModuleDelta::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_DELTA] = delta;
}

int ModuleDelta::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_DELTA) && dataValue[KEY_ATTRIBUTE_DELTA].isInt())
	{
		uint8_t delta = dataValue[KEY_ATTRIBUTE_DELTA].asInt();
		if (BleProtocol::GetInstance()->ConfigDeltaADC(addr, delta) == CODE_OK)
		{
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
