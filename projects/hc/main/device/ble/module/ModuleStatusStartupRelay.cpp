#include "ModuleStatusStartupRelay.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleStatusStartupRelay::ModuleStatusStartupRelay(Device *device, uint16_t addr, string keyString, uint32_t index) : Module(device, addr, index)
{
	status = 0;
	key = keyString + (index ? to_string(index + 1) : "");
}

ModuleStatusStartupRelay::~ModuleStatusStartupRelay()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleStatusStartupRelay::InitAttribute(string attribute, double value)
{
	if (attribute == key)
		status = value;
}

void ModuleStatusStartupRelay::SaveAttribute()
{
	Database::GetInstance()->DeviceAttributeAdd(device, key, status);
}
#endif

int ModuleStatusStartupRelay::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(key) && dataValue[key].isInt())
	{
		status = dataValue[key].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleStatusStartupRelay::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcodeVendor;
		uint16_t vendorId;
		uint16_t header;
		uint8_t relay;
		uint8_t status;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->vendorId == RD_VENDOR_ID)
	{
		if (data_message->header == RD_HEADER_CONFIG_STATUS_STARTUP_RELAY)
		{
			if (data_message->relay == (this->index + 1))
			{
				status = data_message->status;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute();
#endif
				BuildTelemetryValue(jsonValue);
				CheckTrigger(jsonValue);
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}

bool ModuleStatusStartupRelay::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(key) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[key].isInt())
		{
			int status = dataValue[key].asInt();
			rs = Util::CompareNumber(op, this->status, status);
			return true;
		}
		else if (dataValue[key].isArray())
		{
			Json::Value listValue = dataValue[key];
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

void ModuleStatusStartupRelay::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[key] = status;
}

int ModuleStatusStartupRelay::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() && dataValue.isMember(key) && dataValue[key].isInt())
	{
		int status = dataValue[key].asInt();
		if (BleProtocol::GetInstance()->ConfigStatusStartupRelay(addr, this->index + 1, status) == CODE_OK)
			return CODE_OK;
	}
	return CODE_ERROR;
}
