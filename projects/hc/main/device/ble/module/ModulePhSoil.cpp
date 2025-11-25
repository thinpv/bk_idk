#include "ModulePhSoil.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModulePhSoil::ModulePhSoil(Device *device, uint16_t addr) : Module(device, addr)
{
    ph = 0;
}

ModulePhSoil::~ModulePhSoil()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModulePhSoil::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_PH_SOIL)
        ph = value;
}

void ModulePhSoil::SaveAttribute()
{
    Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_PH_SOIL, ph);
}
#endif

int ModulePhSoil::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_PH_SOIL) && dataValue[KEY_ATTRIBUTE_PH_SOIL].isInt())
    {
        ph = dataValue[KEY_ATTRIBUTE_PH_SOIL].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModulePhSoil::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint16_t ph;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_PH_SOIL_STATUS)
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

bool ModulePhSoil::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_PH_SOIL) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_PH_SOIL].isInt())
        {
            int ph = dataValue[KEY_ATTRIBUTE_PH_SOIL].asInt();
            rs = Util::CompareNumber(op, this->ph, ph);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_PH_SOIL].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_PH_SOIL];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int ph1 = listValue[0].asInt();
                int ph2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->ph, ph1, ph2);
                return true;
            }
        }
    }
    return false;
}

void ModulePhSoil::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_PH_SOIL] = ph;
}
