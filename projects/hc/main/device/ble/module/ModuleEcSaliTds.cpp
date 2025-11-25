#include "ModuleEcSaliTds.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleEcSaliTds::ModuleEcSaliTds(Device *device, uint16_t addr) : Module(device, addr)
{
    sali = 0;
    ec = 0;
    tds = 0;
}

ModuleEcSaliTds::~ModuleEcSaliTds()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleEcSaliTds::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_SALI)
        sali = value;
    else if (attribute == KEY_ATTRIBUTE_EC_WATER)
        ec = value;
    else if (attribute == KEY_ATTRIBUTE_TDS)
        tds = value;
}

void ModuleEcSaliTds::SaveAttribute(string key)
{
    if (key == KEY_ATTRIBUTE_SALI)
        Database->GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_SALI, sali);
    else if (key == KEY_ATTRIBUTE_EC_WATER)
        Database->GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_EC_WATER, ec);
    else if (key == KEY_ATTRIBUTE_TDS)
        Database->GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_TDS, tds);
}
#endif

int ModuleEcSaliTds::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_SALI) && dataValue[KEY_ATTRIBUTE_SALI].isInt() &&
        dataValue.isMember(KEY_ATTRIBUTE_EC_WATER) && dataValue[KEY_ATTRIBUTE_EC_WATER].isInt() &&
        dataValue.isMember(KEY_ATTRIBUTE_TDS) && dataValue[KEY_ATTRIBUTE_TDS].isInt())
    {
        sali = dataValue[KEY_ATTRIBUTE_SALI].asInt();
        ec = dataValue[KEY_ATTRIBUTE_EC_WATER].asInt();
        tds = dataValue[KEY_ATTRIBUTE_TDS].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleEcSaliTds::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint16_t ec;
        uint16_t sali;
        uint16_t tds;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_EC_SALI_TDS_WATER)
    {
        uint16_t ecTemp = data_message->ec;
        uint16_t saliTemp = data_message->sali;
        uint16_t tdsTemp = data_message->tds;
        if (ec != ecTemp)
        {
            ec = ecTemp;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_EC_WATER);
#endif
        }
        if (tds != tdsTemp)
        {
            tds = tdsTemp;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_TDS);
#endif
        }
        if (sali != saliTemp)
        {
            sali = saliTemp;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_SALI);
#endif
        }
        BuildTelemetryValue(jsonValue);
        CheckTrigger(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

bool ModuleEcSaliTds::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue.isMember(KEY_ATTRIBUTE_SALI))
        {
            if (dataValue[KEY_ATTRIBUTE_SALI].isInt())
            {
                int saliCheck = dataValue[KEY_ATTRIBUTE_SALI].asInt();
                rs = Util::CompareNumber(op, this->sali, saliCheck);
                return true;
            }
            else if (dataValue[KEY_ATTRIBUTE_SALI].isArray())
            {
                Json::Value listValue = dataValue[KEY_ATTRIBUTE_SALI];
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    int value1 = listValue[0].asInt();
                    int value2 = listValue[1].asInt();
                    rs = Util::CompareNumber(op, this->sali, value1, value2);
                    return true;
                }
            }
        }
        else if (dataValue.isMember(KEY_ATTRIBUTE_EC_WATER))
        {
            if (dataValue[KEY_ATTRIBUTE_EC_WATER].isInt())
            {
                int ecCheck = dataValue[KEY_ATTRIBUTE_EC_WATER].asInt();
                rs = Util::CompareNumber(op, this->ec, ecCheck);
                return true;
            }
            else if (dataValue[KEY_ATTRIBUTE_EC_WATER].isArray())
            {
                Json::Value listValue = dataValue[KEY_ATTRIBUTE_EC_WATER];
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    int value1 = listValue[0].asInt();
                    int value2 = listValue[1].asInt();
                    rs = Util::CompareNumber(op, this->ec, value1, value2);
                    return true;
                }
            }
        }
        else if (dataValue.isMember(KEY_ATTRIBUTE_TDS))
        {
            if (dataValue[KEY_ATTRIBUTE_TDS].isInt())
            {
                int tdsCheck = dataValue[KEY_ATTRIBUTE_TDS].asInt();
                rs = Util::CompareNumber(op, this->tds, tdsCheck);
                return true;
            }
            else if (dataValue[KEY_ATTRIBUTE_TDS].isArray())
            {
                Json::Value listValue = dataValue[KEY_ATTRIBUTE_TDS];
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    int value1 = listValue[0].asInt();
                    int value2 = listValue[1].asInt();
                    rs = Util::CompareNumber(op, this->tds, value1, value2);
                    return true;
                }
            }
        }
    }
    return false;
}

void ModuleEcSaliTds::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_SALI] = sali;
    jsonValue[KEY_ATTRIBUTE_EC_WATER] = ec;
    jsonValue[KEY_ATTRIBUTE_TDS] = tds;
}
