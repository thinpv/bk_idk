#include "ModuleTempWater.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleTempWater::ModuleTempWater(Device *device, uint16_t addr) : Module(device, addr)
{
    temp = 0;
}

ModuleTempWater::~ModuleTempWater()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleTempWater::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_TEMP_WATER)
        temp = value;
}

void ModuleTempWater::SaveAttribute()
{
    Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_TEMP_WATER, temp);
}
#endif

int ModuleTempWater::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_TEMP_WATER) && dataValue[KEY_ATTRIBUTE_TEMP_WATER].isInt())
    {
        temp = dataValue[KEY_ATTRIBUTE_TEMP_WATER].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleTempWater::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint16_t temp;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_TEMP_WATER_STATUS)
    {
        int16_t tp = (int16_t)data_message->temp;
        if (temp != tp)
        {
            temp = tp;
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

bool ModuleTempWater::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_TEMP_WATER) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_TEMP_WATER].isInt())
        {
            int temp = dataValue[KEY_ATTRIBUTE_TEMP_WATER].asInt();
            rs = Util::CompareNumber(op, this->temp, temp);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_TEMP_WATER].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_TEMP_WATER];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int value1 = listValue[0].asInt();
                int value2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->temp, value1, value2);
                return true;
            }
        }
    }
    return false;
}

void ModuleTempWater::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_TEMP_WATER] = temp;
}
