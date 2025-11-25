#include "ModuleTempHumSoil.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleTempHumSoil::ModuleTempHumSoil(Device *device, uint16_t addr) : Module(device, addr)
{
	temp = 0;
	hum = 0;
}

ModuleTempHumSoil::~ModuleTempHumSoil()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleTempHumSoil::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_TEMP_SOIL)
		temp = value;
	else if (attribute == KEY_ATTRIBUTE_HUMIDITY_SOIL)
		hum = value;
}

void ModuleTempHumSoil::SaveAttribute(string key)
{
	if (key == KEY_ATTRIBUTE_TEMP_SOIL)
		Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_TEMP_SOIL, temp);
	else if (key == KEY_ATTRIBUTE_HUMIDITY_SOIL)
		Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_HUMIDITY_SOIL, hum);
}
#endif

int ModuleTempHumSoil::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_TEMP_SOIL) && dataValue[KEY_ATTRIBUTE_TEMP_SOIL].isInt() &&
		dataValue.isMember(KEY_ATTRIBUTE_HUMIDITY_SOIL) && dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].isInt())
	{
		temp = dataValue[KEY_ATTRIBUTE_TEMP_SOIL].asInt();
		hum = dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleTempHumSoil::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint16_t temp;
		uint16_t hum;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;

	if (data_message->opcode == RD_OPCODE_SENSOR_RSP)
	{
		if (data_message->header == RD_HEADER_TEMP_SOIL_STATUS)
		{
			int16_t tempValue = (int16_t)(bswap_16(data_message->temp));
			uint16_t humValue = bswap_16(data_message->hum);
			if (tempValue != temp)
			{
				temp = tempValue;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_TEMP_SOIL);
#endif
			}
			if (humValue != hum)
			{
				hum = humValue;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_HUMIDITY_SOIL);
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger(jsonValue);

			Util::SetTempOfScreenTouch(temp);
			Util::SetHumOfScreenTouch(hum);
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool ModuleTempHumSoil::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue.isMember(KEY_ATTRIBUTE_TEMP_SOIL))
		{
			if (dataValue[KEY_ATTRIBUTE_TEMP_SOIL].isInt())
			{
				int temp = dataValue[KEY_ATTRIBUTE_TEMP_SOIL].asInt();
				rs = Util::CompareNumber(op, this->temp, temp);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_TEMP_SOIL].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_TEMP_SOIL];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int temp1 = listValue[0].asInt();
					int temp2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->temp, temp1, temp2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_HUMIDITY_SOIL))
		{
			if (dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].isInt())
			{
				int hum = dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].asInt();
				rs = Util::CompareNumber(op, this->hum, hum);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_HUMIDITY_SOIL];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int hum1 = listValue[0].asInt();
					int hum2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->hum, hum1, hum2);
					return true;
				}
			}
		}
	}
	return false;
}

void ModuleTempHumSoil::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_TEMP_SOIL] = temp;
	jsonValue[KEY_ATTRIBUTE_HUMIDITY_SOIL] = hum;
}
