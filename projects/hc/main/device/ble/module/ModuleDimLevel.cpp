#include "ModuleDimLevel.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleDimLevel::ModuleDimLevel(Device *device, uint16_t addr, uint32_t index) : Module(device, addr, index)
{
    dimMax = 0;
    dimMin = 0;
}

ModuleDimLevel::~ModuleDimLevel()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDimLevel::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_DIM_LEVEL_MAX)
    {
        dimMax = value;
    }
    else if (attribute == KEY_ATTRIBUTE_DIM_LEVEL_MIN)
    {
        dimMin = value;
    }
}

void ModuleDimLevel::SaveAttribute(string key)
{
    if (key == KEY_ATTRIBUTE_DIM_LEVEL_MAX)
        Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_DIM_LEVEL_MAX, dimMax);
    else if (key == KEY_ATTRIBUTE_DIM_LEVEL_MIN)
        Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_DIM_LEVEL_MIN, dimMin);
}
#endif

int ModuleDimLevel::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_DIM_LEVEL_MAX) && dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MAX].isInt() &&
        dataValue.isMember(KEY_ATTRIBUTE_DIM_LEVEL_MIN) && dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MIN].isInt())
    {
        dimMax = dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MAX].asInt();
        dimMin = dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MIN].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleDimLevel::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint16_t opcode;
        uint8_t header;
        uint16_t magic;
        uint8_t dimMax;
        uint8_t dimMin;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == LIGHTNESS_LINEAR_STATUS && data_message->header == 0x08)
    {
        if (data_message->dimMax != dimMax)
        {
            dimMax = data_message->dimMax;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_DIM_LEVEL_MAX);
#endif
        }
        if (data_message->dimMin != dimMin)
        {
            dimMin = data_message->dimMin;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_DIM_LEVEL_MIN);
#endif
        }
        BuildTelemetryValue(jsonValue);
        CheckTrigger(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

bool ModuleDimLevel::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue.isMember(KEY_ATTRIBUTE_DIM_LEVEL_MAX))
        {
            if (dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MAX].isInt())
            {
                int dimMax = dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MAX].asInt();
                rs = Util::CompareNumber(op, this->dimMax, dimMax);
                return true;
            }
            else if (dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MAX].isArray())
            {
                Json::Value listValue = dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MAX];
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    int value1 = listValue[0].asInt();
                    int value2 = listValue[1].asInt();
                    rs = Util::CompareNumber(op, this->dimMax, value1, value2);
                    return true;
                }
            }
        }
        else if (dataValue.isMember(KEY_ATTRIBUTE_DIM_LEVEL_MIN))
        {
            if (dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MIN].isInt())
            {
                int dimMin = dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MIN].asInt();
                rs = Util::CompareNumber(op, this->dimMin, dimMin);
                return true;
            }
            else if (dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MIN].isArray())
            {
                Json::Value listValue = dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MIN];
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    int value1 = listValue[0].asInt();
                    int value2 = listValue[1].asInt();
                    rs = Util::CompareNumber(op, this->dimMin, value1, value2);
                    return true;
                }
            }
        }
    }
    return false;
}

void ModuleDimLevel::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_DIM_LEVEL_MAX] = dimMax;
    jsonValue[KEY_ATTRIBUTE_DIM_LEVEL_MIN] = dimMin;
}

int ModuleDimLevel::Do(Json::Value &dataValue)
{
    LOGV("Do data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_DIM_LEVEL_MAX) && dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MAX].isInt() &&
        dataValue.isMember(KEY_ATTRIBUTE_DIM_LEVEL_MIN) && dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MIN].isInt())
    {
        int dimMax = dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MAX].asInt();
        int dimMin = dataValue[KEY_ATTRIBUTE_DIM_LEVEL_MIN].asInt();
        if (BleProtocol::GetInstance()->SetLevelDim(addr, dimMax, dimMin) == CODE_OK)
        {
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}
