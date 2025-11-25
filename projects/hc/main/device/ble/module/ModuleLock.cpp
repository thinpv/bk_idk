#include "ModuleLock.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Database.h"

ModuleLock::ModuleLock(Device *device, uint16_t addr) : Module(device, addr)
{
    lock = 0;
}

ModuleLock::~ModuleLock()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleLock::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_LOCK)
        lock = value;
}

void ModuleLock::SaveAttribute()
{
    Database::GetInstance()->DeviceAttributeAdd(device, KEY_ATTRIBUTE_LOCK, lock);
}
#endif

int ModuleLock::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_LOCK) && dataValue[KEY_ATTRIBUTE_LOCK].isInt())
    {
        lock = dataValue[KEY_ATTRIBUTE_LOCK].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleLock::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t vendorId;
        uint16_t header;
        uint8_t lock;
    } data_msg_t;
    data_msg_t *data_msg = (data_msg_t *)data;
    if (data_msg->opcode == RD_OPCODE_CONFIG_RSP && data_msg->header == RD_HEADER_LOCK)
    {
        if (lock != data_msg->lock)
        {
            lock = data_msg->lock;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute();
#endif
        }
        BuildTelemetryValue(jsonValue);
        CheckTrigger(jsonValue);
        return CODE_OK;
    }
    else
    {
        typedef struct __attribute__((packed))
        {
            uint8_t opcode;
            uint16_t header;
            uint8_t lock;
        } data_message_t;
        data_message_t *data_message = (data_message_t *)data;
        if (data_message->opcode == RD_OPCODE_SENSOR_RSP && data_message->header == RD_HEADER_LOCK)
        {
            if (lock != data_message->lock)
            {
                lock = data_message->lock;
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

bool ModuleLock::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_LOCK) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_LOCK].isInt())
        {
            int value = dataValue[KEY_ATTRIBUTE_LOCK].asInt();
            rs = Util::CompareNumber(op, this->lock, value);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_LOCK].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_LOCK];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int value1 = listValue[0].asInt();
                int value2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->lock, value1, value2);
                return true;
            }
        }
    }
    return false;
}

void ModuleLock::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_LOCK] = lock;
}

int ModuleLock::Do(Json::Value &dataValue)
{
    LOGV("ModuleLock Do data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_LOCK) && dataValue[KEY_ATTRIBUTE_LOCK].isInt())
    {
        int sttLock = dataValue[KEY_ATTRIBUTE_LOCK].asInt();
        if (BleProtocol::GetInstance()->LockDevice(addr, sttLock) == CODE_OK)
        {
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}