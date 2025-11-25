#include "ModuleCalibAuto.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleCalibAuto::ModuleCalibAuto(Device *device, uint32_t addr) : Module(device, addr)
{
    time = 0;
}

ModuleCalibAuto::~ModuleCalibAuto()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleCalibAuto::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_CALIB_AUTO)
        time = value;
}

void ModuleCalibAuto::SaveAttribute()
{
    Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_CALIB_AUTO, time);
}
#endif

int ModuleCalibAuto::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_CALIB_AUTO) && dataValue[KEY_ATTRIBUTE_CALIB_AUTO].isInt())
    {
        time = dataValue[KEY_ATTRIBUTE_CALIB_AUTO].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleCalibAuto::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcodeRsp;
        uint16_t vendorId;
        uint16_t header;
        uint8_t data[6];
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    uint16_t tempTime = 0;
    if (data_message->opcodeRsp == RD_OPCODE_CONFIG_RSP)
    {
        if (data_message->header == RD_HEADER_CALIBAUTO || data_message->header == RD_HEADER_LOCK)
        {
            if (data_message->header == RD_HEADER_CALIBAUTO)
            {
                tempTime = data_message->data[0] | (data_message->data[1] << 8);
            }
            else if (data_message->header == RD_HEADER_LOCK)
            {
                tempTime = data_message->data[1] | (data_message->data[2] << 8);
            }

            if (tempTime != time)
            {
                time = tempTime;
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

bool ModuleCalibAuto::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_DISTANCE) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_DISTANCE].isInt())
        {
            int value = dataValue[KEY_ATTRIBUTE_DISTANCE].asInt();
            rs = Util::CompareNumber(op, this->time, value);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_DISTANCE].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_DISTANCE];
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

void ModuleCalibAuto::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_CALIB_AUTO] = time;
}

int ModuleCalibAuto::Do(Json::Value &dataValue)
{
    LOGV("ModuleCalibAuto Do data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_CALIB_AUTO) && dataValue[KEY_ATTRIBUTE_CALIB_AUTO].isInt())
    {
        int time = dataValue[KEY_ATTRIBUTE_CALIB_AUTO].asInt();
        if (BleProtocol::GetInstance()->CalibAuto(addr, time) == CODE_OK)
        {
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}
