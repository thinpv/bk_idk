#include "ModuleTempHum.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleTempHum::ModuleTempHum(Device *device, uint16_t addr) : Module(device, addr)
{
	temp = 0;
	hum = 0;
}

ModuleTempHum::~ModuleTempHum()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleTempHum::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_TEMP)
		temp = value;
	else if (attribute == KEY_ATTRIBUTE_HUMIDITY)
		hum = value;
}

void ModuleTempHum::SaveAttribute(string key)
{
	if (key == KEY_ATTRIBUTE_TEMP)
		Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_TEMP, temp);
	else if (key == KEY_ATTRIBUTE_HUMIDITY)
		Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_HUMIDITY, hum);
}
#endif

int ModuleTempHum::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_TEMP) && dataValue[KEY_ATTRIBUTE_TEMP].isInt() &&
		dataValue.isMember(KEY_ATTRIBUTE_HUMIDITY) && dataValue[KEY_ATTRIBUTE_HUMIDITY].isInt())
	{
		temp = dataValue[KEY_ATTRIBUTE_TEMP].asInt();
		hum = dataValue[KEY_ATTRIBUTE_HUMIDITY].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleTempHum::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint8_t value1[2];
		uint8_t value2[2];
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;

	if (data_message->opcode == RD_OPCODE_SENSOR_RSP)
	{
		if (data_message->header == RD_HEADER_TEMP_HUM_AIR_STATUS_1)
		{
			int temp_temp = (((data_message->value1[0] & 0x7F) << 8) | data_message->value1[1]) & 0x7FFF;
			if (data_message->value1[0] & 0x80)
				temp_temp = (-1) * temp_temp;
			int temp_hum = (data_message->value2[0] << 8) | data_message->value2[1];
			if (temp_temp != temp)
			{
				temp = temp_temp;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_TEMP);
#endif
			}
			if (temp_hum != hum)
			{
				hum = temp_hum;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_HUMIDITY);
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger(jsonValue);

			Util::SetTempOfScreenTouch(temp);
			Util::SetHumOfScreenTouch(hum);
			return CODE_OK;
		}
		else if (data_message->header == RD_HEADER_STATUS_TEMP_HUM_OF_PM && len >= 9)
		{
			int temp_hum = (data_message->value1[0] << 8) | data_message->value1[1];
			int temp_temp = (data[7] << 8) | data[8];
			if (data[5] == 0xff)
				temp_temp = (-1) * temp_temp;
			if (temp_temp != temp)
			{
				temp = temp_temp;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_TEMP);
#endif
			}
			if (temp_hum != hum)
			{
				hum = temp_hum;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_HUMIDITY);
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger(jsonValue);

			return CODE_OK;
		}
		else if (data_message->header == RD_HEADER_TEMP_HUM_AIR_STATUS_2)
		{
			int16_t tempValue = (int16_t)((data_message->value1[0] << 8) | data_message->value1[1]);
			uint16_t humValue = (data_message->value2[0] << 8) | data_message->value2[1];
			if (tempValue != temp)
			{
				temp = tempValue;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_TEMP);
#endif
			}
			if (humValue != hum)
			{
				hum = humValue;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute(KEY_ATTRIBUTE_HUMIDITY);
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger(jsonValue);

			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool ModuleTempHum::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue.isMember(KEY_ATTRIBUTE_TEMP))
		{
			if (dataValue[KEY_ATTRIBUTE_TEMP].isInt())
			{
				int temp = dataValue[KEY_ATTRIBUTE_TEMP].asInt();
				rs = Util::CompareNumber(op, this->temp, temp);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_TEMP].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_TEMP];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int temp1 = listValue[0].asInt();
					int temp2 = listValue[1].asInt();
					LOGE("temp1 = %d, temp2 = %d, temp = %d", temp1, temp2, this->temp);
					rs = Util::CompareNumber(op, this->temp, temp1, temp2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_HUMIDITY))
		{
			if (dataValue[KEY_ATTRIBUTE_HUMIDITY].isInt())
			{
				int hum = dataValue[KEY_ATTRIBUTE_HUMIDITY].asInt();
				rs = Util::CompareNumber(op, this->hum, hum);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_HUMIDITY].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_HUMIDITY];
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

void ModuleTempHum::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_TEMP] = temp;
	jsonValue[KEY_ATTRIBUTE_HUMIDITY] = hum;
}
