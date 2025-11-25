#include "DeviceBle.h"
#include "Log.h"
#include "BleProtocol.h"

#define FAST2ROOM_VER_LIGHT 0x0300
#define FAST2ROOM_VER_SWITCH_RGB_V1 0x010C
#define FAST2ROOM_VER_SWITCH_RGB_V2 0x0100
#define FAST2ROOM_VER_SWITCH_ELECTRICAL_V2 0x0101
#define FAST2ROOM_VER_SWITCH_CURTAIN_V2 0x0100
#define FAST2ROOM_VER_SWITCH_CELING 0x0100

DeviceBle::DeviceBle(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson) : Device(id, name, mac, type, addr, version, dataJson)
{
	protocol = BLE_DEVICE;
	countElement = 1;
	deviceKey = GetDeviceKey(dataJson);
	this->timesUpdateInPeriod = 0;
	this->lastTimeCheckIsolate = time(NULL);
}

DeviceBle::~DeviceBle()
{
	for (auto &module : modules)
		delete module;
}

string DeviceBle::GetDeviceKey(Json::Value *dataJson)
{
	if (dataJson && dataJson->isObject())
	{
		if (dataJson->isMember("devicekey") && (*dataJson)["devicekey"].isString())
		{
			deviceKey = (*dataJson)["devicekey"].asString();
		}
	}
	return deviceKey;
}

bool DeviceBle::CheckAddr(uint16_t addr)
{
	return ((this->addr <= addr) && (this->addr + countElement - 1 >= addr));
}

string DeviceBle::GetDeviceKey()
{
	return deviceKey;
}

int DeviceBle::BuildTelemetryValue(Json::Value &pushDataValue)
{
	for (auto &module : modules)
	{
		module->BuildTelemetryValue(pushDataValue);
	}
	int temp = isOnline();
	pushDataValue["stt"] = temp;
	return CODE_OK;
}

void DeviceBle::Isolate()
{
	this->timesUpdateInPeriod++;
	time_t timeCheck = time(NULL);
	if (((timeCheck - this->lastTimeCheckIsolate) >= BLE_PERIOD_ISOLATE) && (timeCheck - this->lastTimeCheckIsolate) <= (BLE_PERIOD_ISOLATE + 10))
	{
		if (this->timesUpdateInPeriod >= BLE_TIMES_CHECK_ISOLATE)
		{
			LOGW("Isolate device %s", this->id.c_str());
			BleProtocol::GetInstance()->ResetDev(addr);
		}
		this->lastTimeCheckIsolate = timeCheck;
		this->timesUpdateInPeriod = 0;
		return;
	}
	if ((timeCheck - this->lastTimeCheckIsolate) > (BLE_PERIOD_ISOLATE + 10))
	{
		this->lastTimeCheckIsolate = timeCheck;
		this->timesUpdateInPeriod = 0;
	}
}

void DeviceBle::InputData(Json::Value &dataValue, bool isPushTelemety)
{
	values = Json::Value::null;
	for (auto &module : modules)
	{
		module->InputData(dataValue, values);
	}
	if (!values.isNull() && isPushTelemety)
		PushTelemetry(values);
}

void DeviceBle::InputData(uint8_t *data, int len, uint16_t addr)
{
	values = Json::Value::null;
	bool isCheckIsolate = true;
	for (auto &module : modules)
	{
		if (module->CheckAddr(addr))
		{
			if (isCheckIsolate)
			{
				isCheckIsolate = false;
				Isolate();
			}
			if (module->InputData(data, len, values) == CODE_OK)
				break;
		}
	}
	PushTelemetry(values);
	Device::InputData(data, len, addr);
}

bool DeviceBle::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	for (auto &module : modules)
	{
		if (module->CheckData(dataValue, rs))
			return true;
	}
	return false;
}

int DeviceBle::GetNumElement()
{
	return countElement;
}

int DeviceBle::AddToGroup(uint16_t epId, uint16_t groupAddr)
{
	return BleProtocol::GetInstance()->GroupAddDevice(addr, addr + epId, groupAddr + BLE_GROUP_OFFSET);
}

int DeviceBle::RemoveFromGroup(uint16_t epId, uint16_t groupAddr)
{
	return BleProtocol::GetInstance()->GroupDelDevice(addr, addr + epId, groupAddr + BLE_GROUP_OFFSET);
}

int DeviceBle::AddToScene(uint16_t epId, uint16_t sceneAddr)
{
	return BleProtocol::GetInstance()->SceneAddDevice(addr + epId, sceneAddr, 0);
}

int DeviceBle::RemoveFromScene(uint16_t epId, uint16_t sceneAddr)
{
	return BleProtocol::GetInstance()->SceneDelDevice(addr + epId, sceneAddr);
}

int DeviceBle::AddToScene(uint16_t sceneAddr, Json::Value &dataValue, uint16_t groupAddr)
{
	int modeRgb = 0;
	if (dataValue.isObject() && dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
	{
		modeRgb = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
	}

	int indexType1 = (this->getType() >> 16) & 0xFF;
	int indexType2 = (this->getType() >> 8) & 0xFF;
	// Cảnh cho đèn, switch on/off
	if ((indexType1 == 1) ||
			(indexType1 == 2 && indexType2 == 1))
	{
		return BleProtocol::GetInstance()->SceneAddDevice(addr, sceneAddr, modeRgb);
	} // Cảnh cho công tắc: cài từng element
	else if ((indexType1 == 2 && indexType2 == 2) ||
					 (indexType1 == 2 && indexType2 == 4) ||
					 (indexType1 == 2 && indexType2 == 6))
	{
		int rs = CODE_OK;
		for (int i = 0; i < this->GetNumElement(); i++)
		{
			if (dataValue.isMember(KEY_ATTRIBUTE_BUTTON + ((i) ? to_string(i + 1) : "")))
			{
				if (BleProtocol::GetInstance()->SceneAddDevice(addr + i, sceneAddr, 0) != CODE_OK)
					rs = CODE_ERROR;
			}
		}
		return rs;
	}
	else
		LOGW("Type device %d not supported config scene", this->getType());

	return CODE_ERROR;
}

int DeviceBle::RemoveFromScene(uint16_t sceneAddr, Json::Value &dataValue, uint16_t groupAddr)
{
	int indexType1 = (this->getType() >> 16) & 0xFF;
	int indexType2 = (this->getType() >> 8) & 0xFF;
	// Cảnh cho đèn, switch on/off
	if ((indexType1 == 1) ||
			(indexType1 == 2 && indexType2 == 1))
	{
		return BleProtocol::GetInstance()->SceneDelDevice(addr, sceneAddr);
	} // Cảnh cho công tắc: xóa từng element
	else if ((indexType1 == 2 && indexType2 == 2) ||
					 (indexType1 == 2 && indexType2 == 4) ||
					 (indexType1 == 2 && indexType2 == 6))
	{
		int rs = CODE_OK;
		for (int i = 0; i < this->GetNumElement(); i++)
		{
			if (dataValue.isMember(KEY_ATTRIBUTE_BUTTON + ((i) ? to_string(i + 1) : "")))
			{
				if (BleProtocol::GetInstance()->SceneDelDevice(addr + i, sceneAddr) != CODE_OK)
					rs = CODE_ERROR;
			}
		}
		return rs;
	}
	else
		LOGW("Type device %d not supported config scene", this->getType());
	return CODE_ERROR;
}

int DeviceBle::EditInScene(uint16_t sceneAddr, Json::Value &dataValue)
{
	int indexType1 = (this->getType() >> 16) & 0xFF;
	int indexType2 = (this->getType() >> 8) & 0xFF;
	// edit cảnh cho công tắc, phải xóa data trong cảnh trước đó
	if ((indexType1 == 2 && indexType2 == 2) ||
			(indexType1 == 2 && indexType2 == 4) ||
			(indexType1 == 2 && indexType2 == 6))
	{
		int rs = CODE_OK;
		for (int i = 0; i < this->GetNumElement(); i++)
		{
			if (dataValue.isMember(KEY_ATTRIBUTE_BUTTON + ((i) ? to_string(i + 1) : "")))
			{
				if (BleProtocol::GetInstance()->SceneDelDevice(addr + i, sceneAddr) != CODE_OK)
					rs = CODE_ERROR;
			}
		}
		return rs;
	}
	return CODE_ERROR;
}

int DeviceBle::Do(Json::Value &dataValue)
{
	for (auto &module : modules)
	{
		module->Do(dataValue);
	}
	return CODE_OK;
}

int DeviceBle::InitAttribute(string attribute, double value)
{
	for (auto &module : modules)
	{
		module->InitAttribute(attribute, value);
	}
	return CODE_OK;
}
