#include "ModulePhWater.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModulePhWater::ModulePhWater(Device *device, uint16_t addr) : Module(device, addr)
{
    ph = 0;
}

ModulePhWater::~ModulePhWater()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModulePhWater::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_PH_WATER)
        ph = value;
}

void ModulePhWater::SaveAttribute()
{
    Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_PH_WATER, ph);
}
#endif

int ModulePhWater::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_PH_WATER) && dataValue[KEY_ATTRIBUTE_PH_WATER].isInt())
    {
        ph = dataValue[KEY_ATTRIBUTE_PH_WATER].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModulePhWater::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint16_t ph;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_PH_WATER_STATUS)
    {
        uint16_t tempPh = data_message->ph;
        if (ph != tempPh)
        {
            ph = tempPh;
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

bool ModulePhWater::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_PH_WATER) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_PH_WATER].isInt())
        {
            int ph = dataValue[KEY_ATTRIBUTE_PH_WATER].asInt();
            rs = Util::CompareNumber(op, this->ph, ph);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_PH_WATER].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_PH_WATER];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int value1 = listValue[0].asInt();
                int value2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->ph, value1, value2);
                return true;
            }
        }
    }
    return false;
}

void ModulePhWater::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_PH_WATER] = ph;
}
