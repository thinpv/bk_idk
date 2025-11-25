#include "ModuleOnOff.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleOnOff::ModuleOnOff(Device *device, uint16_t addr, string onoffKey, uint32_t index) : Module(device, addr, index)
{
	onoff = 0;
	key = onoffKey + (index ? to_string(index + 1) : "");
}

ModuleOnOff::~ModuleOnOff()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleOnOff::InitAttribute(string attribute, double value)
{
	if (attribute == key)
		onoff = value;
}

void ModuleOnOff::SaveAttribute()
{
	Database::GetInstance()->DeviceAttributeAdd(device, key, onoff);
}
#endif

int ModuleOnOff::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(key) && dataValue[key].isInt())
	{
		onoff = dataValue[key].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleOnOff::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	int temp_onoff = 0;
	if (data[0] == 0x82)
	{
		typedef struct __attribute__((packed))
		{
			uint16_t opcode;
			uint8_t state;
			uint8_t onoff;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)data;
		if (data_message->opcode == G_ONOFF_STATUS)
		{
			if (len == 3)
			{
				temp_onoff = data_message->state;
			}
			else
			{
				temp_onoff = data_message->onoff;
			}
			if (temp_onoff != onoff)
			{
				onoff = temp_onoff;
#ifdef CONFIG_SAVE_ATTRIBUTE
				SaveAttribute();
#endif
			}
			BuildTelemetryValue(jsonValue);
			CheckTrigger(jsonValue);
			return CODE_OK;
		}
	}
	if (data[0] == RD_OPCODE_CONFIG_RSP)
	{
		typedef struct __attribute__((packed))
		{
			uint8_t opcodeVendor;
			uint16_t vendorId;
			uint16_t header;
			uint8_t data[100];
		} data_message_t;
		data_message_t *data_message = (data_message_t *)data;
		if (data_message->vendorId == RD_VENDOR_ID)
		{
			if (data_message->header == RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_4)
			{
				temp_onoff = data_message->data[index + 1];
				if (temp_onoff != onoff)
				{
					onoff = temp_onoff;
#ifdef CONFIG_SAVE_ATTRIBUTE
					SaveAttribute();
#endif
				}
				BuildTelemetryValue(jsonValue);
				CheckTrigger(jsonValue);
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}

bool ModuleOnOff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(key) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[key].isInt())
		{
			int onoff = dataValue[key].asInt();
			rs = Util::CompareNumber(op, this->onoff, onoff);
			return true;
		}
		else if (dataValue[key].isArray())
		{
			Json::Value listValue = dataValue[key];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int onoff1 = listValue[0].asInt();
				int onoff2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->onoff, onoff1, onoff2);
				return true;
			}
		}
	}
	return false;
}

void ModuleOnOff::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[key] = onoff;
	device->UpdatePropertyJsonUpdate(jsonValue);
}

int ModuleOnOff::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(key) && dataValue[key].isInt())
	{
		int onoff = dataValue[key].asInt();
		int indexType1 = (device->getType() >> 16) & 0xFF;
		int indexType2 = (device->getType() >> 8) & 0xFF;
		if ((key == KEY_ATTRIBUTE_ONOFF) && ((indexType1 == 2 && indexType2 == 2) || (indexType1 == 2 && indexType2 == 4)))
		{
			if (BleProtocol::GetInstance()->ControlRelayOfSwitch(addr, device->getType(), 255, onoff) == CODE_OK)
				return CODE_OK;
		}
		else if (onoff == 2)
		{
			if (BleProtocol::GetInstance()->SetOnOffLight(addr, this->onoff ? 0 : 1, TRANSITION_DEFAULT, true) == CODE_OK)
			{
				return CODE_OK;
			}
		}
		else
		{
			if (BleProtocol::GetInstance()->SetOnOffLight(addr, onoff, TRANSITION_DEFAULT, true) == CODE_OK)
			{
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}
