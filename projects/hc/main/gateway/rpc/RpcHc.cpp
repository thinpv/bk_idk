#include "Gateway.h"
#include "Log.h"
// #include "Wifi.h"
#include "Base64.h"
#include "Config.h"
#include "Database.h"
// #include "Http.h"
#include <fstream>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
// #include <dirent.h>

void Gateway::InitRpcHc()
{
	OnRpcCallbackRegister("startScan", bind(&Gateway::OnStartScan, this, placeholders::_1, placeholders::_2));
	OnRpcCallbackRegister("stopScan", bind(&Gateway::OnStopScan, this, placeholders::_1, placeholders::_2));

	// 	OnRpcCallbackRegister("registerHC", bind(&Gateway::OnRegisterHc, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("controlHc", bind(&Gateway::OnControlHc, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("getHcInfo", bind(&Gateway::OnGetHcInfo, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("versionHc", bind(&Gateway::OnVersionHC, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("CreateTunnel", bind(&Gateway::OnCreateTunnel, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("DeleteAllTunnel", bind(&Gateway::OnDeleteAllTunnel, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("otaHC", bind(&Gateway::OnOtaHc, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("setAutoOta", bind(&Gateway::OnAutoOta, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("hcBackupData", bind(&Gateway::OnBackupData, this, placeholders::_1, placeholders::_2));
	// 	OnRpcCallbackRegister("hcRestoreData", bind(&Gateway::OnRestoreData, this, placeholders::_1, placeholders::_2));
}

// int Gateway::OnControlHc(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnControlHc");
// 	int rs = Do(reqValue);
// 	respValue["params"]["code"] = rs;
// 	respValue["method"] = "controlHcRsp";
// 	return CODE_OK;
// }

// int Gateway::OnRegisterHc(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnRegisterHc");
// 	// if (reqValue.isMember("mac") && reqValue["mac"].isString() &&
// 	// 		reqValue.isMember("dormitory") && reqValue["dormitory"].isString())
// 	// {
// 	// 	string macGw = reqValue["mac"].asString();
// 	// 	string dormitoryGw = reqValue["dormitory"].asString();

// 	// 	if (mac == macGw)
// 	// 	{
// 	// 		if (this->getDormitoryId() == "" || this->getDormitoryId() == dormitoryGw)
// 	// 		{
// 	// 			this->setDormitoryId(dormitoryGw);
// 	// 			// Database::GetInstance()->GatewayUpdateDormitory(dormitoryGw);
// 	// 			respValue["method"] = "registerHCRsp";
// 	// 			respValue["params"]["code"] = CODE_OK;
// 	// 			respValue["params"]["mac"] = mac;
// 	// 			respValue["params"]["ip"] = Wifi::GetIP();
// 	// 			respValue["params"]["version"] = STR(VERSION);
// 	// 			respValue["params"]["type"] = 5;
// 	// 			respValue["params"]["name"] = "HCP_" + mac;

// 	// 			if (reqValue.isMember("latitude") && reqValue["latitude"].isDouble() &&
// 	// 					reqValue.isMember("longitude") && reqValue["longitude"].isDouble())
// 	// 			{
// 	// 				Json::Value dataJson;
// 	// 				dataJson["latitude"] = reqValue["latitude"].asDouble();
// 	// 				dataJson["longitude"] = reqValue["longitude"].asDouble();
// 	// 				this->setData(dataJson.toString());
// 	// 				Database::GetInstance()->GatewayUpdateData(dataJson.toString());
// 	// 			}
// 	// 			return CODE_OK;
// 	// 		}
// 	// 		else
// 	// 		{
// 	// 			LOGW("HC have dormitory");
// 	// 		}
// 	// 	}
// 	// 	else
// 	// 	{
// 	// 		LOGW("hc don't match mac");
// 	// 	}
// 	// }
// 	// else
// 	// {
// 	// 	LOGW("data error %s", reqValue.toString().c_str());
// 	// }
// 	return CODE_ERROR;
// }

// // TODO: define type hc
// int Gateway::OnGetHcInfo(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnGetHcInfo");
// 	Json::Value dataValue;
// 	dataValue["mac"] = mac;
// 	dataValue["ip"] = Wifi::GetIP();
// 	dataValue["isConnectCloud"] = CloudProtocol::IsConnected();
// 	dataValue["name"] = "RD HC";
// 	dataValue["type"] = MODEL;
// 	dataValue["ver"] = STR(VERSION);
// 	// if (this->dormitoryId == "")
// 	// {
// 	// 	dataValue["isInHome"] = false;
// 	// }
// 	// else
// 	// {
// 	// 	dataValue["isInHome"] = true;
// 	// }
// 	respValue["params"] = dataValue;
// 	respValue["method"] = "getHcInfoRsp";
// 	return CODE_OK;
// }

int Gateway::OnStartScan(Json::Value &reqValue, Json::Value &respValue)
{
	int rsCode = CODE_OK;
#ifdef CONFIG_ENABLE_BLE
	BleProtocol::GetInstance()->SetProvisioning(true);
	BleProtocol::GetInstance()->StartScan();
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
	ZigbeeProtocol::GetInstance()->PermitJoin(120);
#endif

	respValue["params"]["code"] = rsCode;

	return CODE_OK;
}

int Gateway::OnStopScan(Json::Value &reqValue, Json::Value &respValue)
{
	int rsCode = CODE_OK;
#ifdef CONFIG_ENABLE_BLE
	BleProtocol::GetInstance()->StopScan();
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
	ZigbeeProtocol::GetInstance()->PermitJoin(0);
#endif

	respValue["params"]["code"] = rsCode;
	return CODE_OK;
}

// int Gateway::OnRpcBleStartScanPairDev(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string devId = reqValue["id"].asString();
// 		Device *device = DeviceManager::GetInstance()->GetDeviceFromId(devId);
// 		if (device)
// 		{
// #ifdef CONFIG_ENABLE_BLE
// 			uint32_t maxAddr = DeviceManager::GetInstance()->GetMaxAddrBle();
// 			LOGW("max addr: %d", maxAddr + BLE_MAX_ELEMENT);
// 			BleProtocol::GetInstance()->AddPairDevice(device->getAddr(), maxAddr + BLE_MAX_ELEMENT);
// #endif
// 		}
// 	}
// 	else
// 	{
// 		LOGW("Data scan device pair error");
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "stopScanChildBle";
// 	return CODE_OK;
// }

// int Gateway::OnRpcBleStopScanPairDev(Json::Value &reqValue, Json::Value &respValue)
// {
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string deviceId = reqValue["id"].asString();
// 		Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
// 		if (device)
// 		{
// #ifdef CONFIG_ENABLE_BLE
// 			BleProtocol::GetInstance()->ScanStopSeftPowerRemote(device->getAddr(), 0);
// #endif
// 		}
// 	}
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "stopScanChildBle";
// 	return CODE_OK;
// }

// int Gateway::OnResetHC(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGW("OnResetFactory");
// 	ResetFactory();
// 	// #ifdef __OPENWRT__
// 	// 	Wifi::SetModeApWifi();
// 	// #endif
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["method"] = "resetHcRsp";
// 	return CODE_EXIT;
// }

// int Gateway::OnVersionHC(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("Version HC");
// 	respValue["params"]["code"] = CODE_OK;
// 	respValue["params"]["version"] = STR(VERSION);
// 	respValue["method"] = "versionHcRsp";
// 	return CODE_OK;
// }

// int Gateway::OnCreateTunnel(Json::Value &reqValue, Json::Value &respValue)
// {
// 	int err = 0;
// 	LOGD("Create Tunnel");
// 	if (reqValue.isMember("params") && reqValue["params"].isObject())
// 	{
// 		Json::Value params = reqValue["params"];
// 		if (params.isMember("type") && params["type"].isString() &&
// 				params.isMember("key") && params["key"].isString() &&
// 				params.isMember("user") && params["user"].isString() &&
// 				params.isMember("host") && params["host"].isString() &&
// 				params.isMember("serverPort") && params["serverPort"].isInt() &&
// 				params.isMember("forwardPort") && params["forwardPort"].isInt())
// 		{
// 			string key = "";
// 			string type = params["type"].asString();
// 			string user = params["user"].asString();
// 			string host = params["host"].asString();
// 			uint32_t serverPort = params["serverPort"].asInt();
// 			uint32_t forwardPort = params["forwardPort"].asInt();

// 			uint32_t localPort = 22;
// 			if (params.isMember("localPort") && params["localPort"].isInt())
// 			{
// 				localPort = params["localPort"].asInt();
// 			}
// 			LOGI("Create Tunnel %d --> %s:%d", localPort, host.c_str(), forwardPort);
// 			if (type == "base64")
// 			{
// 				string keyBase64 = params["key"].asString();
// 				string decode = macaron::Base64::Decode(keyBase64, key);
// 				if (decode != "")
// 				{
// 					err = 1;
// 					LOGW("Base64 decode err: %s", decode.c_str());
// 				}
// 			}
// 			else
// 			{
// 				key = params["key"].asString();
// 			}

// 			if (err == 0)
// 			{
// 				// save key file
// 				system("rm " TMP_FOLDER "key.txt");
// 				system("rm " TMP_FOLDER "output.txt");
// 				ofstream keyFile(TMP_FOLDER "key.txt");
// 				keyFile << key;
// 				keyFile.close();

// 				system("chmod 600 " TMP_FOLDER "key.txt");
// 				// system("killall ssh");
// 				string cmd = "ssh -i " TMP_FOLDER "key.txt -o StrictHostKeyChecking=no -f -N -T -R" + to_string(forwardPort) + ":localhost:" + to_string(localPort) + " " + user + "@" + host + " -p " + to_string(serverPort);
// 				cmd += " >> " TMP_FOLDER "output.txt 2>&1";
// 				LOGI("cmd: %s", cmd.c_str());
// 				system(cmd.c_str());
// 				sleep(2);
// 				bool err = false;
// 				FILE *fp = fopen("" TMP_FOLDER "output.txt", "r");
// 				char path[512] = {0};
// 				if (fp)
// 				{
// 					while (fgets(path, sizeof(path), fp) != NULL)
// 					{
// 						if (strlen(path) > 1)
// 						{
// 							LOGW("SSH err: %s", path);
// 							err = true;
// 							break;
// 						}
// 					}
// 					fclose(fp);
// 				}
// 				if (err)
// 				{
// 					respValue["msg"] = string(path);
// 					respValue["code"] = 1;
// 				}
// 				else
// 				{
// 					respValue["code"] = 0;
// 				}
// 				return CODE_OK;
// 			}
// 		}
// 		else
// 		{
// 			LOGE("Error data");
// 			respValue["data"]["code"] = CODE_FORMAT_ERROR;
// 		}
// 	}
// 	else
// 	{
// 		LOGW("Error");
// 	}

// 	respValue["code"] = err;
// 	return CODE_OK;
// }

// int Gateway::OnDeleteAllTunnel(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("Delete all Tunnel");
// 	system("killall ssh");
// 	respValue["code"] = 0;
// 	return CODE_OK;
// }

// int Gateway::OnOtaHc(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OTA HC");
// 	respValue["cmd"] = "otaHcRsp";
// 	if (reqValue.isMember("url") && reqValue["url"].isString() && reqValue.isMember("checksum") && reqValue["checksum"].isString())
// 	{
// #ifndef ESP_PLATFORM
// 		string url = URL_PRO + reqValue["url"].asString();
// 		string sha = reqValue["checksum"].asString();
// 		LOGD("url: %s", url.c_str());
// 		LOGD("sha: %s", sha.c_str());

// 		string cmd = "rm " TMP_FOLDER "rd.tar.gz";
// 		LOGD("%s", cmd.c_str());
// 		system(cmd.c_str());

// 		cmd = "rm -r " TMP_FOLDER "rd";
// 		LOGD("%s", cmd.c_str());
// 		system(cmd.c_str());

// 		cmd = "wget -P " TMP_FOLDER " " + url;
// 		LOGD("%s", cmd.c_str());
// 		system(cmd.c_str());

// 		string folderDownload = TMP_FOLDER "rd.tar.gz";
// 		if (Util::calculateSHA256Checksum(folderDownload) != sha)
// 		{
// 			LOGW("checksum not match");
// 			cmd = "rm " TMP_FOLDER "rd.tar.gz";
// 			system(cmd.c_str());
// 		}
// 		else
// 		{
// 			cmd = "tar -xzf " TMP_FOLDER "rd.tar.gz -C " TMP_FOLDER;
// 			LOGD("%s", cmd.c_str());
// 			system(cmd.c_str());

// 			string fileConfigOta = TMP_FOLDER "rd/ota.sh";
// 			struct stat st;
// 			if (stat(fileConfigOta.c_str(), &st) == 0)
// 			{
// 				cmd = "chmod +x " TMP_FOLDER "rd/ota.sh";
// 				LOGD("%s", cmd.c_str());
// 				system(cmd.c_str());
// #ifdef __ANDROID__
// 				cmd = "su";
// 				LOGD("%s", cmd.c_str());
// 				system(cmd.c_str());
// #endif
// 				string versionCurrent = STR(VERSION);
// 				cmd = TMP_FOLDER "rd/ota.sh " + versionCurrent;
// 				LOGD("%s", cmd.c_str());
// 				system(cmd.c_str());
// 				respValue["data"]["code"] = CODE_OK;
// #ifdef __ANDROID__
// 				return CODE_REBOOT;
// #endif
// 				return CODE_EXIT;
// 			}
// 			else
// 			{
// 				LOGW("Not found ota file");
// 			}
// 		}
// #else
// 		string url = URL_PRO + reqValue["url"].asString();
// 		string sha = reqValue["checksum"].asString();
// 		LOGD("info ota url: %s, checksum: %s", url.c_str(), sha.c_str());
// 		// Config::GetInstance()->SetUrlOta(url);
// 		// Config::GetInstance()->SetCheckSumOta(sha);
// 		respValue["data"]["code"] = CODE_OK;
// 		return CODE_EXIT;
// #endif
// 	}
// 	else
// 	{
// 		LOGW("Data ota error %s", reqValue.toString().c_str());
// 	}

// 	respValue["data"]["code"] = CODE_ERROR;
// 	return CODE_ERROR;
// }

// /*
// {
// 	"cmd" : "setAutoOta",
// 	"rqi" : "abc123",
// 	"data" : {
// 		"isAutoOta" : true/false
// 	}
// }

// {
// 	"cmd" : "setAutoOtaRsp",
// 	"rqi" : "abc123",
// 	"data" : {
// 		"code" : 1
// 	}
// }
// */
// int Gateway::OnAutoOta(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnAutoOta");
// 	respValue["method"] = "setAutoOtaRsp";
// 	int rs = CODE_ERROR;
// 	if (reqValue.isMember("isAutoOta") && reqValue["isAutoOta"].isBool())
// 	{
// 		bool otaStt = reqValue["isAutoOta"].asBool();
// 		// Json::Value dataJson;
// 		// dataJson.parse(this->data);
// 		// if (dataJson.isObject())
// 		// {
// 		// 	dataJson["isAutoOta"] = otaStt;
// 		// 	this->setAutoOta(otaStt);
// 		// 	this->setData(dataJson.toString());
// 		// 	Database::GetInstance()->GatewayUpdateData(dataJson.toString());
// 		// 	rs = CODE_OK;
// 		// }
// 	}
// 	respValue["params"]["code"] = rs;
// 	return CODE_OK;
// }

// /*
// {
// 	"cmd" : "hcBackupData",
// 	"data":
// 	{
// 		"id" :"hcid"
// 	}
// }

// {
// 	"cmd" : "hcBackupDataRsp",
// 	"data":
// 	{
// 		"code" :0
// 	}
// }
// */
// int Gateway::OnBackupData(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnBackupData");
// 	// respValue["method"] = "hcBackupDataRsp";
// 	int rs = CODE_ERROR;
// 	// string hcId;
// 	// if (reqValue.isMember("id") && reqValue["id"].isString())
// 	// {
// 	// 	hcId = reqValue["id"].asString();
// 	// }

// 	// HTTPRequest *httpRequest = new HTTPRequest();
// 	// httpRequest->setUrl(string(URL_PRO) + string(RENEW_TOKEN));
// 	// httpRequest->setMethod("POST");

// 	// if (Gateway::GetInstance()->getDormitoryId() == "" || Gateway::GetInstance()->getRefreshToken() == "")
// 	// {
// 	// 	LOGW("Gateway does not have info dormitory,refresh token");
// 	// }
// 	// string token = httpRequest->GetToken(Gateway::GetInstance()->getRefreshToken(), Gateway::GetInstance()->getDormitoryId());
// 	// if (token != "")
// 	// {
// 	// 	httpRequest->setToken(token);
// 	// 	httpRequest->setUrl(string(URL_PRO) + string(HC_BACKUP_FILE_URL));
// 	// 	httpRequest->setMethod("POST");
// 	// 	string resultUpload = httpRequest->UploadFile(Gateway::GetInstance()->getRefreshToken(), Gateway::GetInstance()->getDormitoryId(), DB_NAME);
// 	// 	LOGD("%s", resultUpload.c_str());
// 	// 	if (resultUpload != "")
// 	// 	{
// 	// 		Json::Value payloadJson;
// 	// 		if (payloadJson.parse(resultUpload) && payloadJson.isObject() && payloadJson.isMember("url"))
// 	// 		{
// 	// 			httpRequest->setUrl(string(URL_PRO) + string(HC_CREATE_BACKUP));
// 	// 			httpRequest->setMethod("POST");
// 	// 			string urlUploadFile = payloadJson["url"].asString();
// 	// 			LOGD("url %s", urlUploadFile.c_str());
// 	// 			if (httpRequest->CreateBackup(Gateway::GetInstance()->getRefreshToken(), Gateway::GetInstance()->getDormitoryId(), Gateway::GetInstance()->getMac(), Gateway::GetInstance()->getVersion(), "", urlUploadFile, hcId))
// 	// 			{
// 	// 				rs = CODE_OK;
// 	// 			}
// 	// 			else
// 	// 			{
// 	// 				LOGW("CreateBackup error");
// 	// 			}
// 	// 		}
// 	// 		else
// 	// 		{
// 	// 			LOGW("url does not available");
// 	// 		}
// 	// 	}
// 	// 	else
// 	// 	{
// 	// 		LOGW("upload file failed");
// 	// 	}
// 	// }
// 	// else
// 	// {
// 	// 	LOGW("Get token failed");
// 	// }

// 	// delete httpRequest;

// 	// respValue["params"]["code"] = rs;

// 	return rs;
// }

// /*
// {
// 	"cmd" : "hcRestoreData",
// 	"data":
// 	{
// 		"id" : "hcid",
// 		"dormitoryId" : "dor",
// 		"backupId" : "backupId",
// 		"backupUrl" : "backupUrl"
// 	}
// }

// {
// 	"cmd" : "hcRestoreDataRsp",
// 	"data":
// 	{
// 		"code" : 0
// 	}
// }
// */

// int Gateway::OnRestoreData(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnRestoreData");
// // 	if (reqValue.isMember("backupUrl") && reqValue["backupUrl"].isString())
// // 	{
// // 		respValue["method"] = "hcRestoreDataRsp";
// // 		string url = reqValue["backupUrl"].asString();
// // 		HTTPRequest *httpRequest = new HTTPRequest();
// // 		httpRequest->setUrl(string(URL_PRO) + string(RENEW_TOKEN));
// // 		httpRequest->setMethod("POST");

// // 		if (Gateway::GetInstance()->getDormitoryId() == "" || Gateway::GetInstance()->getRefreshToken() == "")
// // 		{
// // 			LOGW("Gateway does not have info dormitory,refresh token");
// // 		}

// // 		string token = httpRequest->GetToken(Gateway::GetInstance()->getRefreshToken(), Gateway::GetInstance()->getDormitoryId());
// // 		if (token != "")
// // 		{
// // 			httpRequest->setToken(token);

// // 			httpRequest->setUrl(string(URL_PRO) + string(url));
// // 			httpRequest->setMethod("POST");
// // 			string dataRestore = httpRequest->DownloadFile(Gateway::GetInstance()->getDormitoryId());
// // 			if (dataRestore != "")
// // 			{
// // #ifdef __ANDROID__
// // 				system("su");
// // #endif
// // 				LOGD("dataRestore: %s", dataRestore.c_str());
// // #ifndef ESP_PLATFORM
// // 				string cmd = "rm " DB_NAME "1";
// // 				LOGD("cmd : %s", cmd.c_str());
// // 				system(cmd.c_str());
// // 				std::ofstream outFile(DB_NAME "1");
// // 				if (!outFile)
// // 				{
// // 					respValue["params"]["code"] = CODE_ERROR;
// // 				}

// // 				outFile << dataRestore;
// // 				outFile.close();
// // 				// Doi ten file db
// // 				cmd = "mv " DB_NAME " " TMP_FOLDER "temp.sqlite";
// // 				LOGD("cmd : %s", cmd.c_str());
// // 				system(cmd.c_str());
// // 				cmd = "mv " DB_NAME "1 " DB_NAME;
// // 				LOGD("cmd : %s", cmd.c_str());
// // 				system(cmd.c_str());
// // 				cmd = "mv " TMP_FOLDER "temp.sqlite " DB_NAME "1";
// // 				LOGD("cmd : %s", cmd.c_str());
// // 				system(cmd.c_str());

// // 				respValue["params"]["code"] = CODE_OK;
// // #else
// // 				string newDb = DB_NAME "1";
// // 				string oldDb = DB_NAME;
// // 				string temp = "/storage/temp.sqlite";
// // 				LOGD("newDb : %s", newDb.c_str());
// // 				LOGD("oldDb : %s", oldDb.c_str());
// // 				LOGD("tempDb : %s", temp.c_str());

// // 				// TODO: check delete database
// // 				// if (database)
// // 				// 	delete database;

// // 				if (remove(newDb.c_str()) != 0)
// // 				{
// // 					LOGE("Failed to delete file");
// // 					respValue["params"]["code"] = CODE_ERROR;
// // 				}

// // 				std::ofstream outFile(DB_NAME "1");
// // 				if (!outFile)
// // 				{
// // 					respValue["params"]["code"] = CODE_ERROR;
// // 				}
// // 				outFile << dataRestore;
// // 				outFile.close();

// // 				if (rename(newDb.c_str(), temp.c_str()) != 0)
// // 				{
// // 					LOGE("Rename file failed");
// // 					respValue["params"]["code"] = CODE_ERROR;
// // 					return CODE_EXIT;
// // 				}
// // 				if (rename(oldDb.c_str(), newDb.c_str()) != 0)
// // 				{
// // 					LOGE("Rename file failed");
// // 					respValue["params"]["code"] = CODE_ERROR;
// // 					return CODE_EXIT;
// // 				}
// // 				if (rename(temp.c_str(), oldDb.c_str()) != 0)
// // 				{
// // 					respValue["params"]["code"] = CODE_ERROR;
// // 					return CODE_EXIT;
// // 					LOGE("Rename file failed");
// // 				}
// // 				respValue["params"]["code"] = CODE_OK;
// // #endif
// // 				return CODE_EXIT;
// // 			}
// // 			else
// // 				LOGW("Download error");
// // 		}
// // 		else
// // 			LOGW("get token failed");
// // 	}
// // 	else
// // 		LOGW("Data restore error %s", reqValue.toString().c_str());

// // 	respValue["params"]["code"] = CODE_ERROR;
// 	return CODE_EXIT;
// }

// /*
// {
// 	"cmd": "setPasswordMqtt",
// 	"rqi": "abc-xyz-mnl",
// 	"data": {
// 	"password": "ABC123456"
// 	}
// }
// */

// int Gateway::OnSetPasswordMqtt(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("Set password mqtt");
// 	// 	respValue["method"] = "setPasswordMqttRsp";
// 	// 	if (reqValue.isMember("password") && reqValue["password"].isString())
// 	// 	{
// 	// 		string password = reqValue["password"].asString();
// 	// 		string client_id = "hc-" + mac;
// 	// 		string username = "hc-" + mac;

// 	// #ifdef __ANDROID__
// 	// 		system("su");
// 	// 		string cmd = "mount -o rw,remount /system";
// 	// 		system(cmd.c_str());
// 	// #endif

// 	// 		if (Config::GetInstance()->SetHost("34.126.108.182"))
// 	// 		{
// 	// 			if (Config::GetInstance()->SetPort(1883))
// 	// 			{
// 	// 				if (Config::GetInstance()->SetClientId(client_id))
// 	// 				{
// 	// 					if (Config::GetInstance()->SetUsername(username))
// 	// 					{
// 	// 						if (Config::GetInstance()->SetPassword(password))
// 	// 						{
// 	// 							respValue["params"]["code"] = CODE_OK;
// 	// 							return CODE_EXIT;
// 	// 						}
// 	// 						else
// 	// 							LOGW("set password error");
// 	// 					}
// 	// 					else
// 	// 						LOGW("set username error");
// 	// 				}
// 	// 				else
// 	// 					LOGW("set client error");
// 	// 			}
// 	// 			else
// 	// 				LOGW("set port error");
// 	// 		}
// 	// 		else
// 	// 			LOGW("set host error");
// 	// 	}
// 	// 	else
// 	// 		LOGW("format error: %s", reqValue.toString().c_str());
// 	respValue["params"]["code"] = CODE_ERROR;
// 	return CODE_ERROR;
// }

// #ifdef __ANDROID__
// int Gateway::OnGetNotify(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnGetNotify");
// 	if (reqValue.isMember("groupType") && reqValue["groupType"].isString() &&
// 			reqValue.isMember("startIndex") && reqValue["startIndex"].isInt() &&
// 			reqValue.isMember("endIndex") && reqValue["endIndex"].isInt())
// 	{
// 		string groupType = reqValue["groupType"].asString();
// 		int startIndex = reqValue["startIndex"].asInt();
// 		int endIndex = reqValue["endIndex"].asInt();
// 		int countIndex = 0;
// 		for (auto temp : notiList)
// 		{
// 			string tempType = temp.second->getType();
// 			if (tempType == groupType)
// 			{
// 				if (countIndex >= startIndex)
// 				{
// 					Json::Value payloadJson;
// 					payloadJson.parse(temp.second->GetContent());
// 					payloadJson["isRead"] = temp.second->GetIsRead();
// 					respValue["params"].append(payloadJson);
// 				}
// 				countIndex++;
// 				if (countIndex > endIndex)
// 				{
// 					break;
// 				}
// 			}
// 		}
// 	}
// 	respValue["method"] = "getNotifyRsp";
// 	return CODE_OK;
// }

// int Gateway::OnUpdateReadNotify(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnUpdateReadNotify");
// 	if (reqValue.isMember("id") && reqValue["id"].isString())
// 	{
// 		string id = reqValue["id"].asString();
// 		Noti *noti = getNotifromId(id);
// 		if (noti)
// 		{
// 			noti->UpdateNoti(true);
// 			respValue["params"]["code"] = CODE_OK;
// 		}
// 		respValue["method"] = "readNotifyRsp";
// 	}
// 	return CODE_OK;
// }

// int Gateway::OnDelNotify(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnDelReadNotify");
// 	if (reqValue.isMember("listNotify") && reqValue["listNotify"].isArray())
// 	{
// 		Json::Value listNotify = reqValue["listNotify"];
// 		for (auto &noti : listNotify)
// 		{
// 			if (noti.isObject() && noti.isMember("id") && noti["id"].isString())
// 			{
// 				string id = noti["id"].asString();
// 				Noti *noti = getNotifromId(id);
// 				if (noti)
// 				{
// 					DelNoti(noti);
// 				}
// 				else
// 				{
// 					LOGW("Noti %s not found", id.c_str());
// 				}
// 			}
// 		}
// 	}
// 	else
// 	{
// 		LOGW("Data error %s", reqValue.toString().c_str());
// 	}
// 	respValue["params"]["id"] = CODE_OK;
// 	respValue["method"] = "deleteNotifyRsp";
// 	return CODE_OK;
// }

// int Gateway::OnStartAppTest(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnStartAppTest");
// 	system("am start -n vn.com.rd.testhardwareapp/vn.com.rd.testhardwareapp.MainActivity");
// 	respValue["method"] = "startAppTestRsp";
// 	respValue["params"]["code"] = CODE_OK;
// 	return CODE_OK;
// }

// int Gateway::OnStopAppTest(Json::Value &reqValue, Json::Value &respValue)
// {
// 	LOGD("OnStopAppTest");
// 	system("am force-stop vn.com.rd.testhardwareapp");
// 	respValue["method"] = "stopAppTestRsp";
// 	respValue["params"]["code"] = CODE_OK;
// 	return CODE_OK;
// }
// #endif
