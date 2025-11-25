#pragma once

#include <string>
#include "Object.h"

#define BLE_NET_KEY "ble_netkey"
#define BLE_APP_KEY "ble_appkey"
#define BLE_DEVICE_KEY "ble_devicekey"
#define BLE_ADDR "ble_addr"
#define BLE_IV_INDEX "ble_iv_index"
#define LATITUDE "latitude"
#define LONGITUDE "longitude"

using namespace std;

class GatewayInfo : public Object
{
protected:
	string mac;
	string version;
	uint32_t nextGroupAddr;
	uint32_t nextSceneAddr;

private:
	string bleNetKey;
	string bleAppKey;
	string bleDeviceKey;
	uint16_t bleAddr;
	uint32_t bleIvIndex;
	string latitude;
	string longitude;

public:
	GatewayInfo(
			const string &mac,
			const string &version,
			uint32_t nextGroupAddr,
			uint32_t nextSceneAddr,
			const string &bleNetKey,
			const string &bleAppKey,
			const string &bleDeviceKey,
			uint16_t bleAddr,
			uint32_t bleIvIndex,
			const string &latitude,
			const string &longitude) : Object("dev_" + mac, "Gateway", 0, 0),
																 mac(mac),
																 version(version),
																 nextGroupAddr(nextGroupAddr),
																 nextSceneAddr(nextSceneAddr),
																 bleNetKey(bleNetKey),
																 bleAppKey(bleAppKey),
																 bleDeviceKey(bleDeviceKey),
																 bleAddr(bleAddr),
																 bleIvIndex(bleIvIndex),
																 latitude(latitude),
																 longitude(longitude)
	{
	}

	GatewayInfo(
			const string &mac)
			: Object("dev_" + mac, "", 0, 0),
				mac(mac),
				version(""),
				nextGroupAddr(1),
				nextSceneAddr(1),
				bleNetKey(""),
				bleAppKey(""),
				bleDeviceKey(""),
				bleAddr(0),
				bleIvIndex(0),
				latitude(""),
				longitude("")
	{
	}

	// Getters and Setters
	const string &getMac() const { return mac; }
	void setMac(const string &value) { mac = value; }

	const string &getVersion() const { return version; }
	void setVersion(const string &value) { version = value; }

	uint32_t getNextGroupAddr() const { return nextGroupAddr; }
	void setNextGroupAddr(uint32_t value) { nextGroupAddr = value; }

	uint32_t getNextSceneAddr() const { return nextSceneAddr; }
	void setNextSceneAddr(uint32_t value) { nextSceneAddr = value; }

	const string &getBleNetKey() const { return bleNetKey; }
	void setBleNetKey(const string &value);

	const string &getBleAppKey() const { return bleAppKey; }
	void setBleAppKey(const string &value);

	const string &getBleDeviceKey() const { return bleDeviceKey; }
	void setBleDeviceKey(const string &value);

	uint16_t getBleAddr() const { return bleAddr; }
	void setBleAddr(uint16_t value);

	uint32_t getBleIvIndex() const { return bleIvIndex; }
	void setBleIvIndex(uint32_t value);

	const string &getLatitude() const { return latitude; }
	void setLatitude(const string &value);

	const string &getLongitude() const { return longitude; }
	void setLongitude(const string &value);

	void parseDataValue(Json::Value &dataValue);
};
