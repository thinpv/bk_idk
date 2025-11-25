#include "ModuleDim.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"
#include <math.h>

ModuleDim::ModuleDim(Device *device, uint16_t addr) : Module(device, addr)
{
	dim = 0;
}

ModuleDim::~ModuleDim()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDim::InitAttribute(string attribute, double value)
{
	if (attribute == KEY_ATTRIBUTE_DIM)
		dim = value;
}

void ModuleDim::SaveAttribute()
{
	Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_DIM, dim);
}
#endif

int ModuleDim::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
	{
		dim = dataValue[KEY_ATTRIBUTE_DIM].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleDim::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint16_t opcode;
		uint16_t dim_first;
		uint16_t dim;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == LIGHTNESS_STATUS)
	{
		int temp_dim;
		if (len <= 5)
		{
			temp_dim = ceil(data_message->dim_first * 100 / 65535.0);
		}
		else
		{
			temp_dim = ceil(data_message->dim * 100 / 65535.0);
		}
		if (temp_dim != dim)
		{
			dim = temp_dim;
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

bool ModuleDim::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_DIM) &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_DIM].isInt())
		{
			int dim = dataValue[KEY_ATTRIBUTE_DIM].asInt();
			rs = Util::CompareNumber(op, this->dim, dim);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_DIM].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_DIM];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int dim1 = listValue[0].asInt();
				int dim2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->dim, dim1, dim2);
				return true;
			}
		}
	}
	return false;
}

void ModuleDim::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_DIM] = dim;
	if (dim > 0)
		jsonValue[KEY_ATTRIBUTE_ONOFF] = 1;

	device->UpdatePropertyJsonUpdate(jsonValue);
}

int ModuleDim::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
	{
		int dim = dataValue[KEY_ATTRIBUTE_DIM].asInt();
		if (BleProtocol::GetInstance()->SetDimmingLight(addr, dim * 65535 / 100, TRANSITION_DEFAULT, true) == CODE_OK)
		{
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}
