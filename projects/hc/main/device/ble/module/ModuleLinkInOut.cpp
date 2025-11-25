#include "ModuleLinkInOut.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Database.h"

ModuleLinkInOut::ModuleLinkInOut(Device *device, uint16_t addr, string key, uint32_t index) : Module(device, addr, index)
{
    this->indexIn = 0;
    this->key = key + (index ? to_string(index + 1) : "");
}

ModuleLinkInOut::~ModuleLinkInOut()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleLinkInOut::InitAttribute(string attribute, double value)
{
    if (attribute == key)
        indexIn = value;
}

void ModuleLinkInOut::SaveAttribute()
{
    Database->GetInstance()->DeviceAttributeAdd(device, key, indexIn);
}
#endif

int ModuleLinkInOut::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(key) && dataValue[key].isInt())
    {
        indexIn = dataValue[key].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleLinkInOut::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t vendorId;
        uint16_t header;
        uint8_t indexIn;
        uint8_t indexOut;
    } data_msg_t;
    data_msg_t *data_msg = (data_msg_t *)data;
    if (data_msg->opcode == RD_OPCODE_CONFIG_RSP && data_msg->header == RD_HEADER_CONFIG_COMBINE_MODULE_INOUT)
    {
        if (data_msg->indexOut == (this->index + 1))
        {
            this->indexIn = data_msg->indexIn;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute();
#endif
            BuildTelemetryValue(jsonValue);
            CheckTrigger(jsonValue);
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}

bool ModuleLinkInOut::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(key) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[key].isInt())
        {
            int value = dataValue[key].asInt();
            rs = Util::CompareNumber(op, this->indexIn, value);
            return true;
        }
        else if (dataValue[key].isArray())
        {
            Json::Value listValue = dataValue[key];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int value1 = listValue[0].asInt();
                int value2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->indexIn, value1, value2);
                return true;
            }
        }
    }
    return false;
}

void ModuleLinkInOut::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[key] = indexIn;
}

int ModuleLinkInOut::Do(Json::Value &dataValue)
{
    LOGV("ModuleLinkInOut Do data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() && dataValue.isMember(key) && dataValue[key].isInt())
    {
        int indexIn = dataValue[key].asInt();
        if (BleProtocol::GetInstance()->ConfigCombinInOutModuleInOut(addr, indexIn, this->index + 1) == CODE_OK)
        {
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}