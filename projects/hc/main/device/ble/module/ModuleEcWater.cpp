#include "ModuleEcWater.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleEcWater::ModuleEcWater(Device *device, uint16_t addr) : Module(device, addr)
{
    ec = 0;
}

ModuleEcWater::~ModuleEcWater()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleEcWater::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_EC_WATER)
        ec = value;
}

void ModuleEcWater::SaveAttribute()
{
    Database->GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_EC_WATER, ec);
}
#endif

int ModuleEcWater::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_EC_WATER) && dataValue[KEY_ATTRIBUTE_EC_WATER].isInt())
    {
        ec = dataValue[KEY_ATTRIBUTE_EC_WATER].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleEcWater::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint16_t ec;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_EC_WATER_STATUS)
    {
        uint16_t tempEc = data_message->ec;
        if (ec != tempEc)
        {
            ec = tempEc;
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

bool ModuleEcWater::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_EC_WATER) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_EC_WATER].isInt())
        {
            int ec = dataValue[KEY_ATTRIBUTE_EC_WATER].asInt();
            rs = Util::CompareNumber(op, this->ec, ec);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_EC_WATER].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_EC_WATER];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int ec1 = listValue[0].asInt();
                int ec2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->ec, ec1, ec2);
                return true;
            }
        }
    }
    return false;
}

void ModuleEcWater::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_EC_WATER] = ec;
}
