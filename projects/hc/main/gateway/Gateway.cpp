#include "Gateway.h"
#include "Log.h"
#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <fstream>
#include <iostream>
#include <thread>
#include "json.h"
#include "Database.h"
#include "Util.h"
// #include "Wifi.h"
#include "Ota.h"
#include "Base64.h"
#include "Config.h"
// #include "TimerSchedule.h"
// #include "Http.h"
#include "DeviceManager.h"

#ifdef ESP_PLATFORM
#include "Led.h"
#endif

#define TIME_CHECK_OTA 10800

char sClientId[128] = "dev_";

Gateway *Gateway::GetInstance()
{
	static Gateway *gateway = NULL;
	if (!gateway)
	{
		string mac = "aabbcc";// Wifi::GetMacAddress();
		sprintf(sClientId, "dev_%s", mac.c_str());
		gateway = new Gateway(
				mac,
				config_data.server.host,
				config_data.server.port,
				sClientId,
				sClientId,
				config_data.server.password,
				config_data.server.keep_alive,
				config_data.server.tls);
	}
	return gateway;
}

Gateway::Gateway(string mac, string address, int port, const char *clientId, string username, string password, int keepalive, bool ssl)
		: CloudProtocol(address, port, clientId, username, password, keepalive, ssl),
			GatewayInfo(mac)
{
	this->isAutoOta = true;
	this->lastTimePingGwBle = time(NULL);
}

Gateway::~Gateway()
{
}

#ifdef ESP_PLATFORM
static void startCheckOnlineThread(void *arg)
{
	Gateway *gateway = static_cast<Gateway *>(arg);
	if (gateway)
	{
		gateway->CheckOnlineThread();
	}

	vTaskDelete(NULL);
}
#endif

void Gateway::init()
{
	CloudProtocol::init();
	Device::InitDeviceModelList();

	InitRpc();
	InitRpcHc();
	InitRpcDevice();
	// InitRpcGroup();
	// InitRpcScene();
	// InitRpcRule();

	Database::GetInstance()->GatewayRead();
	Database::GetInstance()->DeviceRead();
#ifdef CONFIG_SAVE_ATTRIBUTE
	Database::GetInstance()->DeviceAttributeRead();
#endif
	Database::GetInstance()->GroupRead();
	Database::GetInstance()->DeviceInGroupRead();
	Database::GetInstance()->SceneRead();
	Database::GetInstance()->DeviceInSceneRead();
	Database::GetInstance()->RuleRead();

	if (getBleNetKey().empty() || getBleNetKey().length() != 32)
	{
		uint8_t u8Buff[16];
		for (int i = 0; i < 16; i++)
		{
			u8Buff[i] = (uint8_t)(rand() & 0xFF);
		}
		setBleNetKey(Util::uuidToStr(u8Buff));
		Database::GetInstance()->GatewayUpdateData();
	}

	if (getBleAppKey().empty() || getBleAppKey().length() != 32)
	{
		uint8_t u8Buff[16];
		for (int i = 0; i < 16; i++)
		{
			u8Buff[i] = (uint8_t)(rand() & 0xFF);
		}
		setBleAppKey(Util::uuidToStr(u8Buff));
		Database::GetInstance()->GatewayUpdateData();
	}

	if (getName() == "")
	{
		setName("Gateway");
		Database::GetInstance()->GatewayAdd();
		Database::GetInstance()->GatewayRead();
	}

	string firmwareVer = STR(VERSION);
	if (getVersion() != firmwareVer)
	{
		setVersion(firmwareVer);
		Database::GetInstance()->GatewayUpdateVersion();
	}

	Connect();

	// ListDeviceMatterInit();
	// GetMatterCommission();
	// GetMatterFabric();

	// #ifdef ESP_PLATFORM
	// 	if (xTaskCreate(startCheckOnlineThread, "CheckOnlineThread", 5120, this, 7, NULL) != pdPASS)
	// 	{
	// 		LOGE("Create CheckOnlineThread failed");
	// 		SetLedService(false);
	// 	}
	// 	vTaskDelay(10);
	// #else
	// 	thread checkOnlineThread(bind(&Gateway::CheckOnlineThread, this));
	// 	checkOnlineThread.detach();
	// #endif
}

void Gateway::CheckSharedDeviceAttrite(Json::Value &payloadJson, bool updateParams)
{
	CloudProtocol::CheckSharedDeviceAttrite(payloadJson, updateParams);
	if (updateParams)
	{
		if (payloadJson.isObject())
		{
			if (payloadJson.isMember("groups") && payloadJson["groups"].isObject())
				GroupManager::GetInstance()->AddGroup(payloadJson);

			if (payloadJson.isMember("scenes") && payloadJson["scenes"].isObject())
				SceneManager::GetInstance()->AddScene(payloadJson);

			if (payloadJson.isMember("rules") && payloadJson["rules"].isObject())
				RuleManager::GetInstance()->AddRule(payloadJson);
		}
	}
	else
	{
		GroupManager::GetInstance()->AddGroup(payloadJson);
		SceneManager::GetInstance()->AddScene(payloadJson);
		RuleManager::GetInstance()->AddRule(payloadJson);
	}
}

bool Gateway::FirmwareCheckNeedUpdate(string fwVersion)
{
	LOGI("FirmwareCheckNeedUpdate fwVersion: %s", fwVersion.c_str());
	return Ota::CheckNeedUpdate(fwVersion);
}

int Gateway::FirmwareUpdate(string fwUrl, string fwChecksumAlgorithm, string fwChecksum)
{
	LOGI("FirmwareUpdate fwUrl: %s", fwUrl.c_str());
	return Ota::Update(fwUrl, fwChecksumAlgorithm, fwChecksum);
}

int Gateway::FirmwareUpdate(int fwSize, string fwChecksumAlgorithm, string fwChecksum)
{
	LOGI("FirmwareUpdate fwSize: %d, fwChecksumAlgorithm: %s, fwChecksum: %s", fwSize, fwChecksumAlgorithm.c_str(), fwChecksum.c_str());
	return Ota::Update(fwSize, fwChecksumAlgorithm, fwChecksum);
}

int Gateway::FirmwareUpdateChunk(int chunkId, uint8_t *data, int dataLen)
{
	LOGI("FirmwareUpdateChunk chunkId: %d, dataLen: %d", chunkId, dataLen);
	return Ota::UpdateChunk(chunkId, data, dataLen);
}

int Gateway::FirmwareUpdateFinish()
{
	LOGI("FirmwareUpdateFinish");
	return Ota::UpdateFinish();
}

void Gateway::OnConnect(int rc)
{
	CloudProtocol::OnConnect(rc);

	LOGI("OnConnect: %d", isConnected);
	Util::LedInternet(true);
	Json::Value macAttribute;
	macAttribute["mac"] = mac;
	GatewayAttribute(macAttribute);
	// CheckDataSync();
}

void Gateway::OnDisconnect(int rc)
{
	CloudProtocol::OnDisconnect(rc);

	LOGI("OnDisconnect: %d", isConnected);
	Util::LedInternet(false);
}

void Gateway::OnSubscribeDone()
{
	LOGI("OnSubscribeDone");
	CheckDataSync();
}

void Gateway::CheckDataSync()
{
	DeviceAttributeRequest("{\"sharedKeys\":\"groups,scenes,rules,fw_version,fw_size,fw_checksum_algorithm,fw_checksum,fw_url\"}");
}

void Gateway::ResetFactory()
{
	LOGI("ResetFactory");
#ifdef CONFIG_ENABLE_BLE
	BleProtocol::GetInstance()->ResetDelAll();
	BleProtocol::GetInstance()->ResetFactory();
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
	ZigbeeProtocol::GetInstance()->ResetFactory();
	ZigbeeProtocol::GetInstance()->CommissionFormation();
#endif

	Database::GetInstance()->DelDatabase();
}

int Gateway::RestartBleGw()
{
	LOGW("Restart Gateway Ble");
#ifdef __ANDROID__
	Util::ExecuteCMD("su");
	Util::ExecuteCMD("echo 100 > /sys/class/gpio/export");
	Util::ExecuteCMD("echo out > /sys/class/gpio/gpio100/direction");
	Util::ExecuteCMD("echo 1 > /sys/class/gpio/gpio100/value");
	sleep(1);
	Util::ExecuteCMD("echo 0 > /sys/class/gpio/gpio100/value");
	Util::ExecuteCMD("echo 100 > /sys/class/gpio/unexport");
#elif defined(__OPENWRT__)
	Util::ExecuteCMD("echo '0' > /sys/class/gpio/gpio1/value");
	sleep(1);
	Util::ExecuteCMD("echo '1' > /sys/class/gpio/gpio1/value");
#elif defined(ESP_PLATFORM)
	SetGpioResetGwBle();
#endif
	return CODE_OK;
}

static string ST_array_icon[18] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d", "13d", "50d", "01n", "02n", "03n", "04n", "09n", "10n", "11n", "13n", "50n"};

int Gateway::CheckOnlineThread()
{
	LOGI("Start CheckOnlineThread");
	time_t currentTime = 0;
	// time_t oldTime = 0;
	// time_t oldTimeCheckStatus = 0;
	uint32_t allTimeCheck = 0; // time total in a loop check
	bool deviceStateChange = false;

	Json::Value onlineValue;
	Json::Value offlineValue;
	offlineValue["stt"] = 0;
	onlineValue["stt"] = 1;

	while (1)
	{
		// if ((time(NULL) - oldTime) > 1800)
		// {
		// 	oldTime = time(NULL);
		// 	HTTPRequest *httpRequest = new HTTPRequest();
		// 	string dataWeather = httpRequest->GetWeather(getLatitude(), getLongitude());
		// 	delete httpRequest;
		// 	LOGI("dataWeather:%s", dataWeather.c_str());

		// 	uint8_t status = 254;
		// 	uint16_t temp = 65534;

		// 	Json::Value dataWeatherJson;
		// 	if (dataWeatherJson.parse(dataWeather) && dataWeatherJson.isObject())
		// 	{
		// 		if (dataWeatherJson.isMember("weather") && dataWeatherJson["weather"].isArray() &&
		// 				dataWeatherJson.isMember("main") && dataWeatherJson["main"].isObject())
		// 		{
		// 			Json::Value weather = dataWeatherJson["weather"][0];
		// 			Json::Value main = dataWeatherJson["main"];
		// 			if (weather.isMember("icon") && weather["icon"].isString() && main.isMember("temp") && main["temp"].isDouble())
		// 			{
		// 				string icon = weather["icon"].asString();
		// 				for (int i = 0; i < 17; i++)
		// 				{
		// 					if (icon.compare(ST_array_icon[i]) == 0)
		// 					{
		// 						status = i;
		// 						break;
		// 					}
		// 				}
		// 				temp = main["temp"].asInt();
		// 				Util::SetStatusWeatherOutdoor(status);
		// 				Util::SetTempWeatherOutdoor(temp);
		// 			}
		// 		}
		// 	}
		// }

#ifdef CONFIG_ENABLE_BLE
		allTimeCheck = 0;
		vector<Device *> listDevBle;
		DeviceManager::GetInstance()->ForEach([&](Device *device)
																					{
												if (device->getProtocol() == BLE_DEVICE)
												{
													allTimeCheck++;
													listDevBle.push_back(device);
												} });
		allTimeCheck = allTimeCheck * 4;
		if (allTimeCheck > 0)
		{
			for (auto &device : listDevBle)
			{
				if (device->getProtocol() == BLE_DEVICE)
				{
					if (!BleProtocol::GetInstance()->IsProvision() && !CloudProtocol::IsBusy() && BleProtocol::GetInstance()->isInitKey)
					{
						currentTime = time(NULL);
						deviceStateChange = false;
						if (device->lastOnlineState) // online
						{
							// neu thiet bi ho tro ban tin check trang thai online/offline
							if (device->isNeedCheckOnline())
							{
								// thoi gian lan cuoi cung nhan ban tin hoac lan cuoi cung check qua 1 chu ky
								if ((device->lastTimeActive + allTimeCheck) <= currentTime && (device->lastTimeCheckActive + allTimeCheck) <= currentTime)
								{
									BleProtocol::GetInstance()->SendOnlineCheck(device->getAddr(), device->getType(), device->getSwVer());
									device->lastTimeCheckActive = currentTime;
								}
								// 2 chu ky khong co ban tin phan hoi thi bao offline
								if ((device->lastTimeActive + allTimeCheck * 2 + 1) < currentTime)
								{
									LOGI("Device %s addr 0x%04X offline", device->getName().c_str(), device->getAddr());
									device->lastOnlineState = false;
									deviceStateChange = true;
								}
							}
							// neu thiet bi khong ho tro ban tin check trang thai online/offline
							else
							{
								// 1 ngay khong co ban tin moi thi bao offline
								if ((device->lastTimeActive + 60 * 60 * 24) < currentTime)
								{
									LOGI("Device %s addr 0x%04X offline", device->getName().c_str(), device->getAddr());
									device->lastOnlineState = false;
									deviceStateChange = true;
								}
							}
						}
						else
						{
							if (device->isNeedCheckOnline())
							{
								// thoi gian check qua 1 chu ky thi check lai
								if ((device->lastTimeCheckActive + allTimeCheck) <= currentTime)
								{
									BleProtocol::GetInstance()->SendOnlineCheck(device->getAddr(), device->getType(), device->getSwVer());
									device->lastTimeCheckActive = currentTime;
								}
								// neu co ban tin moi trong vong 2 chu ky check thi bao online
								if ((device->lastTimeActive + allTimeCheck * 2) >= currentTime)
								{
									LOGI("Device %s addr 0x%04X online", device->getName().c_str(), device->getAddr());
									device->lastOnlineState = true;
									deviceStateChange = true;
								}
							}
							else
							{
								// trong ngay co ban tin thi online
								if ((device->lastTimeActive + 60 * 60 * 24) >= currentTime)
								{
									LOGI("Device %s addr 0x%04X online", device->getName().c_str(), device->getAddr());
									device->lastOnlineState = true;
									deviceStateChange = true;
								}
							}
						}

						// send device state to server
						if (deviceStateChange)
						{
							if (device->lastOnlineState)
								device->PushTelemetry(onlineValue);
							else
								device->PushTelemetry(offlineValue);
						}
					}
				}
			}
			// currentTime = time(NULL);
			// if (currentTime - getLastTimePingGwBle() > allTimeCheck)
			// {
			// 	setLastTimePingGwBle(time(NULL));
			// 	RestartBleGw();
			// }
		}

#endif
		sleep(1);
	}
	return CODE_OK;
}

bool Gateway::getAutoOta()
{
	return this->isAutoOta;
}

void Gateway::setAutoOta(bool isAutoOta)
{
	this->isAutoOta = isAutoOta;
}

int Gateway::Do(Json::Value &dataValue)
{
	LOGD("Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject())
	{
#ifdef CONFIG_TTGO_TAUDIO
		if (dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
		{
			Led::RGB_Dim(dataValue[KEY_ATTRIBUTE_DIM].asInt());
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_R) && dataValue[KEY_ATTRIBUTE_R].isInt() &&
				dataValue.isMember(KEY_ATTRIBUTE_G) && dataValue[KEY_ATTRIBUTE_G].isInt() &&
				dataValue.isMember(KEY_ATTRIBUTE_B) && dataValue[KEY_ATTRIBUTE_B].isInt())
		{
			Led::RGB_Color(dataValue[KEY_ATTRIBUTE_R].asInt(), dataValue[KEY_ATTRIBUTE_G].asInt(), dataValue[KEY_ATTRIBUTE_B].asInt());
		}
		else
		{
			if (dataValue.isMember(KEY_ATTRIBUTE_R) && dataValue[KEY_ATTRIBUTE_R].isInt())
			{
				Led::RGB_ColorR(dataValue[KEY_ATTRIBUTE_R].asInt());
			}
			if (dataValue.isMember(KEY_ATTRIBUTE_G) && dataValue[KEY_ATTRIBUTE_G].isInt())
			{
				Led::RGB_ColorG(dataValue[KEY_ATTRIBUTE_G].asInt());
			}
			if (dataValue.isMember(KEY_ATTRIBUTE_B) && dataValue[KEY_ATTRIBUTE_B].isInt())
			{
				Led::RGB_ColorB(dataValue[KEY_ATTRIBUTE_B].asInt());
			}
		}
#elif __ANDROID__
		if (dataValue.isMember(KEY_ATTRIBUTE_RELAY "0") && dataValue[KEY_ATTRIBUTE_RELAY "0"].isInt())
		{
			int onoff = dataValue[KEY_ATTRIBUTE_RELAY "0"].asInt();
			if (onoff)
			{
				Util::ExecuteCMD("echo 1 > /sys/class/st_relay/relay1");
			}
			else
			{
				Util::ExecuteCMD("echo 0 > /sys/class/st_relay/relay1");
			}
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_RELAY "1") && dataValue[KEY_ATTRIBUTE_RELAY "1"].isInt())
		{
			int onoff = dataValue[KEY_ATTRIBUTE_RELAY "1"].asInt();
			if (onoff)
			{
				Util::ExecuteCMD("echo 1 > /sys/class/st_relay/relay2");
			}
			else
			{
				Util::ExecuteCMD("echo 0 > /sys/class/st_relay/relay2");
			}
		}
#endif
	}
	return CODE_FORMAT_ERROR;
}

int Gateway::pushDeviceUpdateCloud(Json::Value &dataValue)
{
	return PublishToCloudMessage("deviceUpdate", dataValue, "deviceUpdateRsp", NULL);
}

int Gateway::pushNewDeviceCloud(Json::Value &dataValue)
{
	return PublishToCloudMessage("newDev", dataValue, "newDevRsp", NULL);
}

time_t Gateway::getLastTimePingGwBle()
{
	return this->lastTimePingGwBle;
}

void Gateway::setLastTimePingGwBle(time_t timeUpdate)
{
	this->lastTimePingGwBle = timeUpdate;
}

uint32_t Gateway::getNextAndIncreaseGroupAddr()
{
	LOGI("getNextAndIncreaseGroupAddr: %d", nextGroupAddr);
	uint32_t oldNextGroupAddr = nextGroupAddr;
	++nextGroupAddr;
	Database::GetInstance()->GatewayUpdateNextGroupAddr();
	return oldNextGroupAddr;
}

uint32_t Gateway::getNextAndIncreaseSceneAddr()
{
	LOGI("getNextAndIncreaseSceneAddr: %d", nextSceneAddr);
	uint32_t oldNextSceneAddr = nextSceneAddr;
	++nextSceneAddr;
	Database::GetInstance()->GatewayUpdateNextSceneAddr();
	return oldNextSceneAddr;
}