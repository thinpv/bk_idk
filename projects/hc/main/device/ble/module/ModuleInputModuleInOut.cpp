#include "ModuleInputModuleInOut.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleInputModuleInOut::ModuleInputModuleInOut(Device *device, uint16_t addr, string key, uint32_t index) : Module(device, addr, index)
{
	this->status = 0;
	this->key = key + (index ? to_string(index + 1) : "");
}

ModuleInputModuleInOut::~ModuleInputModuleInOut()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleInputModuleInOut::InitAttribute(string attribute, double value)
{
	if (attribute == key)
		status = value;
}

void ModuleInputModuleInOut::SaveAttribute()
{
	Database->GetInstance()->DeviceAttributeAdd(device, key, status);
}
#endif

int ModuleInputModuleInOut::InputData(Json::Value &dataValue, Json::Value &jsonValue)
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

int ModuleInputModuleInOut::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint8_t indexIn;
        uint8_t status;
        uint16_t sceneId;
    }data_message_t;
    data_message_t * data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_STATUS_IN_MODULE_INOUT)
    {
        if (data_message->indexIn == this->index + 1)
        {
            if (data_message->status != status)
            {
                status = data_message->status;
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

bool ModuleInputModuleInOut::CheckData(Json::Value &dataValue, bool &rs)
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

void ModuleInputModuleInOut::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[key] = status;
}
