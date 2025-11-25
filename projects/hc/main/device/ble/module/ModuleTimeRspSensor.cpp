#include "ModuleTimeRspSensor.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleTimeRspSensor::ModuleTimeRspSensor(Device *device, uint32_t addr) : Module(device, addr)
{
    time = 0;
}

ModuleTimeRspSensor::~ModuleTimeRspSensor()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleTimeRspSensor::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_TIME_RSP)
        time = value;
}

void ModuleTimeRspSensor::SaveAttribute()
{
    Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_TIME_RSP, time);
}
#endif

int ModuleTimeRspSensor::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_TIME_RSP) && dataValue[KEY_ATTRIBUTE_TIME_RSP].isInt())
    {
        time = dataValue[KEY_ATTRIBUTE_TIME_RSP].asInt();
        // CheckTrigger();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleTimeRspSensor::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t vendorId;
        uint16_t header;
        uint16_t time;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_CONFIG_RSP)
    {
        if (data_message->header == RD_HEADER_CONFIG_TIME_RSP_SENSOR)
        {
            if (time != data_message->time)
            {
                time = data_message->time;
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

bool ModuleTimeRspSensor::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_TIME_RSP) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_TIME_RSP].isInt())
        {
            int value = dataValue[KEY_ATTRIBUTE_TIME_RSP].asInt();
            rs = Util::CompareNumber(op, this->time, value);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_TIME_RSP].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_TIME_RSP];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int value1 = listValue[0].asInt();
                int value2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->time, value1, value2);
                return true;
            }
        }
    }
    return false;
}

void ModuleTimeRspSensor::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_TIME_RSP] = time;
}

int ModuleTimeRspSensor::Do(Json::Value &dataValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_TIME_RSP) && dataValue[KEY_ATTRIBUTE_TIME_RSP].isInt())
    {
        int value = dataValue[KEY_ATTRIBUTE_TIME_RSP].asInt();
        if (BleProtocol::GetInstance()->SetTimeRspSensor(addr, value) == CODE_OK)
        {
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}
