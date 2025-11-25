#include "ModuleOxyWater.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleOxyWater::ModuleOxyWater(Device *device, uint16_t addr) : Module(device, addr)
{
    percentOxy = 0;
    concentrationOxy = 0;
}

ModuleOxyWater::~ModuleOxyWater()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleOxyWater::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_PERCENT_OXY_WATER)
        percentOxy = value;
    if (attribute == KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER)
        concentrationOxy = value;
}

void ModuleOxyWater::SaveAttribute(string key)
{
    if (key == KEY_ATTRIBUTE_PERCENT_OXY_WATER)
        Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_PERCENT_OXY_WATER, percentOxy);
    if (key == KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER)
        Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER, concentrationOxy);
}
#endif

int ModuleOxyWater::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_PERCENT_OXY_WATER) && dataValue[KEY_ATTRIBUTE_PERCENT_OXY_WATER].isInt() &&
        dataValue.isMember(KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER) && dataValue[KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER].isInt())
    {
        percentOxy = dataValue[KEY_ATTRIBUTE_PERCENT_OXY_WATER].asInt();
        concentrationOxy = dataValue[KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleOxyWater::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint16_t percentOxy;
        uint16_t concentrationOxy;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_OXY_WATER_STATUS)
    {
        uint16_t tempPercentOxy = data_message->percentOxy;
        uint16_t tempConcentrationOxy = data_message->concentrationOxy;
        if (percentOxy != tempPercentOxy)
        {
            percentOxy = tempPercentOxy;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_PERCENT_OXY_WATER);
#endif
        }

        if (concentrationOxy != tempConcentrationOxy)
        {
            concentrationOxy = tempConcentrationOxy;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER);
#endif
        }
        BuildTelemetryValue(jsonValue);
        CheckTrigger(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

bool ModuleOxyWater::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
		dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue.isMember(KEY_ATTRIBUTE_PERCENT_OXY_WATER))
		{
			if (dataValue[KEY_ATTRIBUTE_PERCENT_OXY_WATER].isInt())
			{
				int percent = dataValue[KEY_ATTRIBUTE_PERCENT_OXY_WATER].asInt();
				rs = Util::CompareNumber(op, this->percentOxy, percent);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_PERCENT_OXY_WATER].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_PERCENT_OXY_WATER];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int percent1 = listValue[0].asInt();
					int percent2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->percentOxy, percent1, percent2);
					return true;
				}
			}
		}
		else if (dataValue.isMember(KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER))
		{
			if (dataValue[KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER].isInt())
			{
				int concentration = dataValue[KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER].asInt();
				rs = Util::CompareNumber(op, this->concentrationOxy, concentration);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int concentration1 = listValue[0].asInt();
					int concentration2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->concentrationOxy, concentration1, concentration2);
					return true;
				}
			}
		}
	}
	return false;
}

void ModuleOxyWater::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_PERCENT_OXY_WATER] = percentOxy;
    jsonValue[KEY_ATTRIBUTE_CONCENTRATION_OXY_WATER] = concentrationOxy;
}
