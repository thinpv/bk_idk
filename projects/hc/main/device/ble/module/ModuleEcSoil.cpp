#include "ModuleEcSoil.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleEcSoil::ModuleEcSoil(Device *device, uint16_t addr) : Module(device, addr)
{
    ec = 0;
}

ModuleEcSoil::~ModuleEcSoil()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleEcSoil::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_EC_SOIL)
        ec = value;
}

void ModuleEcSoil::SaveAttribute()
{
    Database->GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_EC_SOIL, ec);
}
#endif

int ModuleEcSoil::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_EC_SOIL) && dataValue[KEY_ATTRIBUTE_EC_SOIL].isInt())
    {
        ec = dataValue[KEY_ATTRIBUTE_EC_SOIL].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleEcSoil::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint16_t ec;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_EC_SOIL_STATUS)
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

bool ModuleEcSoil::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_EC_SOIL) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_EC_SOIL].isInt())
        {
            int ec = dataValue[KEY_ATTRIBUTE_EC_SOIL].asInt();
            rs = Util::CompareNumber(op, this->ec, ec);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_EC_SOIL].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_EC_SOIL];
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

void ModuleEcSoil::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_EC_SOIL] = ec;
}
