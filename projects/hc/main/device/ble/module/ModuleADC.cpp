#include "ModuleADC.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleADC::ModuleADC(Device *device, uint16_t addr) : Module(device, addr)
{
	adc = 0;
}

ModuleADC::~ModuleADC()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleADC::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_ADC)
		adc = value;
}

void ModuleADC::SaveAttribute()
{
	Database->GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_ADC, adc);
}
#endif

int ModuleADC::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_ADC) && dataValue[KEY_ATTRIBUTE_ADC].isInt())
	{
		adc = dataValue[KEY_ATTRIBUTE_ADC].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleADC::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint16_t adc;
		uint16_t sceneId;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_STATUS_ADC_MODULE_INOUT)
	{
		if (adc != bswap_16(data_message->adc))
		{
			adc = bswap_16(data_message->adc);
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

bool ModuleADC::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_ADC) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_ADC].isInt())
		{
			int adc = dataValue[KEY_ATTRIBUTE_ADC].asInt();
			rs = Util::CompareNumber(op, this->adc, adc);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_ADC].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_ADC];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int adc1 = listValue[0].asInt();
				int adc2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->adc, adc1, adc2);
				return true;
			}
		}
	}
	return false;
}

void ModuleADC::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_ADC] = adc;
}
