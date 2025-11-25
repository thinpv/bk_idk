#include "ModuleModeInModuleInOut.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleModeInModuleInOut::ModuleModeInModuleInOut(Device *device, uint16_t addr, string key, uint32_t index) : Module(device, addr, index)
{
	this->mode = 0;
	this->key = key + (index ? to_string(index + 1) : "");
}

ModuleModeInModuleInOut::~ModuleModeInModuleInOut()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleModeInModuleInOut::InitAttribute(string attribute, double value)
{
	if (attribute == key)
		mode = value;
}

void ModuleModeInModuleInOut::SaveAttribute()
{
	Database::GetInstance()->DeviceAttributeAdd(device, key, mode);
}
#endif

int ModuleModeInModuleInOut::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(key) && dataValue[key].isInt())
	{
		mode = dataValue[key].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleModeInModuleInOut::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t indexIn;
		uint8_t mode;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == RD_OPCODE_CONFIG_RSP && data_message->header == RD_HEADER_CONFIG_MODE_INPUT_MODULE_INOUT)
	{
		if (data_message->indexIn == this->index + 1)
		{
			if (data_message->mode != mode)
			{
				mode = data_message->mode;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute();
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger(jsonValue);
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool ModuleModeInModuleInOut::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(key) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[key].isInt())
		{
			int mode = dataValue[key].asInt();
			rs = Util::CompareNumber(op, this->mode, mode);
			return true;
		}
		else if (dataValue[key].isArray())
		{
			Json::Value listValue = dataValue[key];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int mode1 = listValue[0].asInt();
				int mode2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->mode, mode1, mode2);
				return true;
			}
		}
	}
	return false;
}

void ModuleModeInModuleInOut::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[key] = mode;
}

int ModuleModeInModuleInOut::Do(Json::Value &dataValue)
{
	LOGV("ModuleIn ModuleInOut Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() && dataValue.isMember(key) && dataValue[key].isInt())
	{
		int modeValue = dataValue[key].asInt();
		if (BleProtocol::GetInstance()->ConfigModeInputModuleInOut(addr, this->index + 1, modeValue) == CODE_OK)
		{
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}