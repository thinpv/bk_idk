#include "Config.h"
#include <string.h>
#include "Storage.h"
// #include "esp_log.h"
#include "Define.h"
#include "Util.h"
#include "json.h"
#include "DeviceManager.h"
// #include "Wifi.h"
#include "Log.h"

#ifndef PREFIX_ACCESS_TOKEN
#define PREFIX_ACCESS_TOKEN "dev_"
#endif

#define SERVER_ADDR "34.126.108.182"
#define SERVER_PORT 1883

const static char *TAG = "Config";

config_t config_data =
		{
				.server = {"", SERVER_ADDR, SERVER_PORT, 0, "", "", "", 0,
				},
				.eth = { "dhcp",
				},
				.wifi = { 1, "iot", "iot@2025", 1, 0, 2, 1, "DigitalR&D", "DigitalRD@2804", "dhcp",
				},
				.serial = {
						// {.baudrate = 9600, .data_size = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bit = UART_STOP_BITS_1},
						// {.baudrate = 9600, .data_size = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bit = UART_STOP_BITS_1},
				},
				.sensor = { 200,
				},
};

static void config_default_server()
{
	string mac = "aabbcc";// Wifi::GetMacAddress();
	sprintf(config_data.server.device_id, "%s", mac.c_str());
	strcpy(config_data.server.host, SERVER_ADDR);
	config_data.server.port = SERVER_PORT;
	config_data.server.tls = 0;
	sprintf(config_data.server.client_id, PREFIX_ACCESS_TOKEN "%s_%ld", mac.c_str(), time(NULL));
	sprintf(config_data.server.username, PREFIX_ACCESS_TOKEN "%s", mac.c_str());
	strcpy(config_data.server.password, "");
	config_data.server.keep_alive = 60;
}

int config_read()
{
	char *config_json = NULL;
	int config_len = 0;
	config_default_server();
	if (Storage::GetInstance()->ReadFile("/storage/config.json", &config_json, &config_len) == CODE_OK)
	{
		LOGI("config_json: %s", config_json);
		Json::Value payloadJson;
		if (payloadJson.parse(config_json) && payloadJson.isObject())
		{
			if (payloadJson.isMember("server") && payloadJson["server"].isObject())
			{
				Json::Value serverJson = payloadJson["server"];
				if (serverJson.isMember("device_id") && serverJson["device_id"].isString())
					strcpy(config_data.server.device_id, serverJson["device_id"].asString().c_str());
				if (serverJson.isMember("host") && serverJson["host"].isString())
					strcpy(config_data.server.host, serverJson["host"].asString().c_str());
				if (serverJson.isMember("port") && serverJson["port"].isInt())
					config_data.server.port = serverJson["port"].asInt();
				if (serverJson.isMember("tls") && serverJson["tls"].isInt())
					config_data.server.tls = serverJson["tls"].asInt();
				if (serverJson.isMember("client_id") && serverJson["client_id"].isString())
					strcpy(config_data.server.client_id, serverJson["client_id"].asString().c_str());
				if (serverJson.isMember("username") && serverJson["username"].isString())
					strcpy(config_data.server.username, serverJson["username"].asString().c_str());
				if (serverJson.isMember("password") && serverJson["password"].isString())
					strcpy(config_data.server.password, serverJson["password"].asString().c_str());
				if (serverJson.isMember("keep_alive") && serverJson["keep_alive"].isInt())
					config_data.server.keep_alive = serverJson["keep_alive"].asInt();
			}
			if (payloadJson.isMember("eth") && payloadJson["eth"].isObject())
			{
				Json::Value ethJson = payloadJson["eth"];
				if (ethJson.isMember("protocol") && ethJson["protocol"].isString())
					strcpy(config_data.eth.protocol, ethJson["protocol"].asString().c_str());
				if (strcmp(config_data.eth.protocol, "static") == 0)
				{
					if (ethJson.isMember("ip") && ethJson["ip"].isString())
						strcpy(config_data.eth.ip, ethJson["ip"].asString().c_str());
					if (ethJson.isMember("mask") && ethJson["mask"].isString())
						strcpy(config_data.eth.mask, ethJson["mask"].asString().c_str());
					if (ethJson.isMember("gw") && ethJson["gw"].isString())
						strcpy(config_data.eth.gw, ethJson["gw"].asString().c_str());
					if (ethJson.isMember("dns1") && ethJson["dns1"].isString())
						strcpy(config_data.eth.dns1, ethJson["dns1"].asString().c_str());
					if (ethJson.isMember("dns2") && ethJson["dns2"].isString())
						strcpy(config_data.eth.dns2, ethJson["dns2"].asString().c_str());
				}
			}
			if (payloadJson.isMember("wifi") && payloadJson["wifi"].isObject())
			{
				Json::Value wifiJson = payloadJson["wifi"];
				if (wifiJson.isMember("ap_enable") && wifiJson["ap_enable"].isInt())
					config_data.wifi.ap_enable = wifiJson["ap_enable"].asInt();
				if (wifiJson.isMember("ap_ssid") && wifiJson["ap_ssid"].isString())
					strcpy(config_data.wifi.ap_ssid, wifiJson["ap_ssid"].asString().c_str());
				if (wifiJson.isMember("ap_password") && wifiJson["ap_password"].isString())
					strcpy(config_data.wifi.ap_password, wifiJson["ap_password"].asString().c_str());
				if (wifiJson.isMember("ap_channel") && wifiJson["ap_channel"].isInt())
					config_data.wifi.ap_channel = wifiJson["ap_channel"].asInt();
				if (wifiJson.isMember("ap_ssid_hidden") && wifiJson["ap_ssid_hidden"].isInt())
					config_data.wifi.ap_ssid_hidden = wifiJson["ap_ssid_hidden"].asInt();
				if (wifiJson.isMember("ap_max_connection") && wifiJson["ap_max_connection"].isInt())
					config_data.wifi.ap_max_connection = wifiJson["ap_max_connection"].asInt();
				if (wifiJson.isMember("sta_enable") && wifiJson["sta_enable"].isInt())
					config_data.wifi.sta_enable = wifiJson["sta_enable"].asInt();
				if (wifiJson.isMember("sta_ssid") && wifiJson["sta_ssid"].isString())
					strcpy(config_data.wifi.sta_ssid, wifiJson["sta_ssid"].asString().c_str());
				if (wifiJson.isMember("sta_password") && wifiJson["sta_password"].isString())
					strcpy(config_data.wifi.sta_password, wifiJson["sta_password"].asString().c_str());
				if (wifiJson.isMember("protocol") && wifiJson["protocol"].isString())
					strcpy(config_data.wifi.protocol, wifiJson["protocol"].asString().c_str());
				if (strcmp(config_data.wifi.protocol, "static") == 0)
				{
					if (wifiJson.isMember("ip") && wifiJson["ip"].isString())
						strcpy(config_data.wifi.ip, wifiJson["ip"].asString().c_str());
					if (wifiJson.isMember("mask") && wifiJson["mask"].isString())
						strcpy(config_data.wifi.mask, wifiJson["mask"].asString().c_str());
					if (wifiJson.isMember("gw") && wifiJson["gw"].isString())
						strcpy(config_data.wifi.gw, wifiJson["gw"].asString().c_str());
					if (wifiJson.isMember("dns1") && wifiJson["dns1"].isString())
						strcpy(config_data.wifi.dns1, wifiJson["dns1"].asString().c_str());
					if (wifiJson.isMember("dns2") && wifiJson["dns2"].isString())
						strcpy(config_data.wifi.dns2, wifiJson["dns2"].asString().c_str());
				}
			}
			if (payloadJson.isMember("sensor") && payloadJson["sensor"].isObject())
			{
				Json::Value sensorJson = payloadJson["sensor"];
				if (sensorJson.isMember("sensitive") && sensorJson["sensitive"].isInt())
					config_data.sensor.sensitive = sensorJson["sensitive"].asInt();
			}
			if (payloadJson.isMember("serial") && payloadJson["serial"].isArray())
			{
				Json::Value serialJson = payloadJson["serial"];
				for (int i = 0; i < MAX_SERIAL_NUM; i++)
				{
					if (i < serialJson.size() && serialJson[i].isObject())
					{
						Json::Value serialItem = serialJson[i];
						if (serialItem.isMember("baudrate") && serialItem["baudrate"].isInt())
							config_data.serial[i].baudrate = serialItem["baudrate"].asInt();
						if (serialItem.isMember("data_size") && serialItem["data_size"].isInt())
							config_data.serial[i].data_size = serialItem["data_size"].asInt();
						if (serialItem.isMember("parity") && serialItem["parity"].isInt())
							config_data.serial[i].parity = serialItem["parity"].asInt();
						if (serialItem.isMember("stop_bit") && serialItem["stop_bit"].isInt())
							config_data.serial[i].stop_bit = serialItem["stop_bit"].asInt();
					}
				}
			}
			if (payloadJson.isMember("device") && payloadJson["device"].isArray())
			{
				Json::Value devicesJson = payloadJson["device"];
				for (int i = 0; i < devicesJson.size(); i++)
				{
					Json::Value deviceItem = devicesJson[i];
					// DeviceManager::GetInstance()->AddNewModbusDevice(deviceItem);
				}
			}
		}
		else
		{
			LOGW("Config file is not valid JSON");
		}
		free(config_json);
	}
	return CODE_OK;
}

int config_write()
{
	Json::Value config_data_value;
	config_data_value["server"]["device_id"] = config_data.server.device_id;
	config_data_value["server"]["host"] = config_data.server.host;
	config_data_value["server"]["port"] = config_data.server.port;
	config_data_value["server"]["tls"] = config_data.server.tls;
	config_data_value["server"]["client_id"] = config_data.server.client_id;
	config_data_value["server"]["username"] = config_data.server.username;
	config_data_value["server"]["password"] = config_data.server.password;
	config_data_value["server"]["keep_alive"] = config_data.server.keep_alive;
	config_data_value["eth"]["protocol"] = config_data.eth.protocol;
	if (strcmp(config_data.eth.protocol, "static") == 0)
	{
		config_data_value["eth"]["ip"] = config_data.eth.ip;
		config_data_value["eth"]["mask"] = config_data.eth.mask;
		config_data_value["eth"]["gw"] = config_data.eth.gw;
		config_data_value["eth"]["dns1"] = config_data.eth.dns1;
		config_data_value["eth"]["dns2"] = config_data.eth.dns2;
	}
	config_data_value["wifi"]["ap_enable"] = config_data.wifi.ap_enable ? 1 : 0;
	config_data_value["wifi"]["ap_ssid"] = config_data.wifi.ap_ssid;
	config_data_value["wifi"]["ap_password"] = config_data.wifi.ap_password;
	config_data_value["wifi"]["ap_channel"] = config_data.wifi.ap_channel;
	config_data_value["wifi"]["ap_ssid_hidden"] = config_data.wifi.ap_ssid_hidden;
	config_data_value["wifi"]["ap_max_connection"] = config_data.wifi.ap_max_connection;
	config_data_value["wifi"]["sta_enable"] = config_data.wifi.sta_enable ? 1 : 0;
	config_data_value["wifi"]["sta_ssid"] = config_data.wifi.sta_ssid;
	config_data_value["wifi"]["sta_password"] = config_data.wifi.sta_password;
	config_data_value["wifi"]["protocol"] = config_data.wifi.protocol;
	if (strcmp(config_data.wifi.protocol, "static") == 0)
	{
		config_data_value["wifi"]["ip"] = config_data.wifi.ip;
		config_data_value["wifi"]["mask"] = config_data.wifi.mask;
		config_data_value["wifi"]["gw"] = config_data.wifi.gw;
		config_data_value["wifi"]["dns1"] = config_data.wifi.dns1;
		config_data_value["wifi"]["dns2"] = config_data.wifi.dns2;
	}
	config_data_value["sensor"]["sensitive"] = config_data.sensor.sensitive;
	for (int i = 0; i < MAX_SERIAL_NUM; i++)
	{
		config_data_value["serial"][i]["baudrate"] = config_data.serial[i].baudrate;
		config_data_value["serial"][i]["data_size"] = config_data.serial[i].data_size;
		config_data_value["serial"][i]["parity"] = config_data.serial[i].parity;
		config_data_value["serial"][i]["stop_bit"] = config_data.serial[i].stop_bit;
	}
	LOGI("config write : %s", config_data_value.toString().c_str());
	return Storage::GetInstance()->WriteFile("/storage/config.json", config_data_value.toString().c_str());
}

void config_print()
{
	LOGI("config_data.server.device_id: %s", config_data.server.device_id);
	LOGI("config_data.server.host: %s", config_data.server.host);
	LOGI("config_data.server.port: %d", config_data.server.port);
	LOGI("config_data.server.tls: %d", config_data.server.tls);
	LOGI("config_data.server.client_id: %s", config_data.server.client_id);
	LOGI("config_data.server.username: %s", config_data.server.username);
	LOGI("config_data.server.password: %s", config_data.server.password);
	LOGI("config_data.server.keep_alive: %d", config_data.server.keep_alive);

	LOGI("config_data.eth.protocol: %s", config_data.eth.protocol);
	if (strcmp(config_data.eth.protocol, "static") == 0)
	{
		LOGI("config_data.eth.ip: %s", config_data.eth.ip);
		LOGI("config_data.eth.mask: %s", config_data.eth.mask);
		LOGI("config_data.eth.gw: %s", config_data.eth.gw);
		LOGI("config_data.eth.dns1: %s", config_data.eth.dns1);
		LOGI("config_data.eth.dns2: %s", config_data.eth.dns2);
	}

	LOGI("config_data.wifi.ap_enable: %d", config_data.wifi.ap_enable);
	LOGI("config_data.wifi.ap_ssid: %s", config_data.wifi.ap_ssid);
	LOGI("config_data.wifi.ap_password: %s", config_data.wifi.ap_password);
	LOGI("config_data.wifi.ap_channel: %d", config_data.wifi.ap_channel);
	LOGI("config_data.wifi.ap_ssid_hidden: %d", config_data.wifi.ap_ssid_hidden);
	LOGI("config_data.wifi.ap_max_connection: %d", config_data.wifi.ap_max_connection);
	LOGI("config_data.wifi.sta_enable: %d", config_data.wifi.sta_enable);
	LOGI("config_data.wifi.sta_ssid: %s", config_data.wifi.sta_ssid);
	LOGI("config_data.wifi.sta_password: %s", config_data.wifi.sta_password);
	LOGI("config_data.wifi.protocol: %s", config_data.wifi.protocol);
	if (strcmp(config_data.wifi.protocol, "static") == 0)
	{
		LOGI("config_data.wifi.ip: %s", config_data.wifi.ip);
		LOGI("config_data.wifi.mask: %s", config_data.wifi.mask);
		LOGI("config_data.wifi.gw: %s", config_data.wifi.gw);
		LOGI("config_data.wifi.dns1: %s", config_data.wifi.dns1);
		LOGI("config_data.wifi.dns2: %s", config_data.wifi.dns2);
	}

	LOGI("config_data.sensor.sensitive: %d", config_data.sensor.sensitive);
}