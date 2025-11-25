#include "GatewayInfo.h"
#include "Log.h"

void GatewayInfo::setBleNetKey(const string &value)
{
	LOGI("setBleNetKey: %s", value.c_str());
	if (!value.empty())
	{
		bleNetKey = value;
		dataValue[BLE_NET_KEY] = value;
	}
}

void GatewayInfo::setBleAppKey(const string &value)
{
	LOGI("setBleAppKey: %s", value.c_str());
	if (!value.empty())
	{
		bleAppKey = value;
		dataValue[BLE_APP_KEY] = value;
	}
}

void GatewayInfo::setBleDeviceKey(const string &value)
{
	LOGI("setBleDeviceKey: %s", value.c_str());
	if (!value.empty())
	{
		bleDeviceKey = value;
		dataValue[BLE_DEVICE_KEY] = value;
	}
}

void GatewayInfo::setBleAddr(uint16_t value)
{
	LOGI("setBleAddr: %d", value);
	bleAddr = value;
	dataValue[BLE_ADDR] = value;
}

void GatewayInfo::setBleIvIndex(uint32_t value)
{
	LOGI("setBleIvIndex: %d", value);
	bleIvIndex = value;
	dataValue[BLE_IV_INDEX] = (Json::UInt)value;
}

void GatewayInfo::setLatitude(const string &value)
{
	LOGI("setLatitude: %s", value.c_str());
	if (!value.empty())
	{
		latitude = value;
		dataValue[LATITUDE] = value;
	}
}

void GatewayInfo::setLongitude(const string &value)
{
	LOGI("setLongitude: %s", value.c_str());
	if (!value.empty())
	{
		longitude = value;
		dataValue[LONGITUDE] = value;
	}
}

void GatewayInfo::parseDataValue(Json::Value &dataValue)
{
	if (dataValue.isObject())
	{
		if (dataValue.isMember(BLE_NET_KEY) && dataValue[BLE_NET_KEY].isString())
		{
			bleNetKey = dataValue[BLE_NET_KEY].asString();
		}

		if (dataValue.isMember(BLE_APP_KEY) && dataValue[BLE_APP_KEY].isString())
		{
			bleAppKey = dataValue[BLE_APP_KEY].asString();
		}

		if (dataValue.isMember(BLE_DEVICE_KEY) && dataValue[BLE_DEVICE_KEY].isString())
		{
			bleDeviceKey = dataValue[BLE_DEVICE_KEY].asString();
		}

		if (dataValue.isMember(BLE_ADDR) && dataValue[BLE_ADDR].isInt())
		{
			bleAddr = dataValue[BLE_ADDR].asUInt();
		}

		if (dataValue.isMember(BLE_IV_INDEX) && dataValue[BLE_IV_INDEX].isInt())
		{
			bleIvIndex = dataValue[BLE_IV_INDEX].asUInt();
		}

		if (dataValue.isMember(LATITUDE) && dataValue[LATITUDE].isString())
		{
			latitude = dataValue[LATITUDE].asString();
		}

		if (dataValue.isMember(LONGITUDE) && dataValue[LONGITUDE].isString())
		{
			longitude = dataValue[LONGITUDE].asString();
		}
	}
}