#include "ModuleCct.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleCct::ModuleCct(Device *device, uint16_t addr) : Module(device, addr)
{
	cct = 0;
}

ModuleCct::~ModuleCct()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleCct::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_CCT)
		cct = value;
}

void ModuleCct::SaveAttribute()
{
	Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_CCT, cct);
}
#endif

int ModuleCct::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
	{
		cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleCct::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint16_t opcode;
		uint16_t cct_first;
		uint16_t magic;
		uint16_t cct;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == LIGHT_CTL_TEMP_STATUS)
	{
		int temp_cct;
		if (len <= 6)
		{
			temp_cct = (data_message->cct_first - 800) / 192;
		}
		else
		{
			temp_cct = (data_message->cct - 800) / 192;
		}
		if (temp_cct != cct)
		{
			cct = temp_cct;
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

bool ModuleCct::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_CCT) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_CCT].isInt())
		{
			int cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
			rs = Util::CompareNumber(op, this->cct, cct);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_CCT].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_CCT];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int cct1 = listValue[0].asInt();
				int cct2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->cct, cct1, cct2);
				return true;
			}
		}
	}
	return false;
}

void ModuleCct::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_CCT] = cct;
	device->UpdatePropertyJsonUpdate(jsonValue);
}

int ModuleCct::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
	{
		int cct = dataValue[KEY_ATTRIBUTE_CCT].asInt();
		if (BleProtocol::GetInstance()->SetCctLight(addr, (cct * 192) + 800, TRANSITION_DEFAULT, true) == CODE_OK)
		{
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
