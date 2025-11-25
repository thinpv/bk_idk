#include "BleProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include "Log.h"
#include "Util.h"
#include <string.h>
#include <algorithm>
#include "Gateway.h"
#include "Database.h"
#include "BleOpCode.h"
#include "DeviceBle.h"
#include "DeviceBleSwitchScene6ACRgb.h"
#include "DeviceBleSeftPowerRemote.h"
#include "AES.h"
#include "DeviceManager.h"
#ifdef ESP_PLATFORM
#include "Led.h"
#endif

static uint8_t keyAes[] = {0x44, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x40, 0x32, 0x38, 0x31, 0x31, 0x32, 0x38, 0x30, 0x34};
static uint8_t plaintext[] = {0x24, 0x02, 0x28, 0x04, 0x28, 0x11, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t appKeyDefault[] = {0x60, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48};

static BleProtocol *bleProtocol = NULL;

BleProtocol *BleProtocol::GetInstance()
{
    if (!bleProtocol)
    {
        bleProtocol = new BleProtocol();
    }
    return bleProtocol;
}

BleProtocol::BleProtocol()
{
    nextAddr = 0;
    haveNewMac = false;
    haveGetMacRsp = true;
    isProvisioning = false;
    isInitKey = false;
}

BleProtocol::~BleProtocol()
{
}

static void AddDeviceThread(void *data)
{
    // BleProtocol *bleProtocol = (BleProtocol *)data;
    int timeout = 0;
    while (1)
    {
        if (BleProtocol::GetInstance()->IsProvision())
        {
            scan_device_message_t scan_device_message;
            if (BleProtocol::GetInstance()->haveNewMac)
            {
                timeout = 0;
                memcpy(&scan_device_message, &BleProtocol::GetInstance()->scanDeviceMessage, sizeof(scan_device_message_t));
                BleProtocol::GetInstance()->AddDevice(&scan_device_message);
                BleProtocol::GetInstance()->haveGetMacRsp = true;
                BleProtocol::GetInstance()->haveNewMac = false;
            }
            else
            {
                timeout++;
                if (timeout >= 300)
                {
                    timeout = 0;
                    BleProtocol::GetInstance()->SetProvisioning(false);
                    BleProtocol::GetInstance()->StopScan();
                    Gateway::GetInstance()->SendTimeoutProvisionDev();
                }
            }
        }
        SLEEP_MS(100);
    }
}

static void HandleOpcodeBle(void *data)
{
    // BleProtocol *bleProtocol = (BleProtocol *)data;
    message_rsp_st *message_rsp = NULL;
    while (1)
    {
        if (BleProtocol::GetInstance()->GetOpcodeExceptionMessage(&message_rsp) == CODE_OK)
        {
            BleProtocol::GetInstance()->CheckOpcodeException(message_rsp);
            free(message_rsp);
        }
        SLEEP_MS(100);
    }
}

void BleProtocol::init()
{
    // #ifdef ESP_PLATFORM
    // 	if (xTaskCreate(AddDeviceThread, "AddDeviceThread", 8192, this, 10, NULL) != pdPASS)
    // 	{
    // 		LOGE("Failed to create task");
    // 		SetLedService(false);
    // 	}
    // 	if (xTaskCreate(HandleOpcodeBle, "HandleOpcodeBle", 10240, this, 10, NULL) != pdPASS)
    // 	{
    // 		LOGE("Failed to create task");
    // 		SetLedService(false);
    // 	}
    // #else
    // 	thread addDeviceThreadThread(AddDeviceThread, this);
    // 	addDeviceThreadThread.detach();
    // 	thread handleOpcodeBleThread(HandleOpcodeBle, this);
    // 	handleOpcodeBleThread.detach();
    // #endif

    SLEEP_MS(100); // wait for thread start
}

void BleProtocol::initKey(string netKeyStr, string appKeyStr)
{
    while (GetNetKey())
    {
#ifdef ESP_PLATFORM
        FlashLedInternet();
#else
        sleep(4);
#endif
    }
    CheckKeyBle();
    isInitKey = true;
}

void BleProtocol::CheckKeyBle()
{
    string bleAppkey = Gateway::GetInstance()->getBleAppKey();
    string bleNetkey = Gateway::GetInstance()->getBleNetKey();
    string bleDevicekey = Gateway::GetInstance()->getBleDeviceKey();
    srand(time(NULL));

    string netkeyStr = Util::uuidToStr((uint8_t *)netKey);

    // LOGW("bleNetkey: %s, netKey: %s", bleNetkey.c_str(), netkeyStr.c_str());

    if (bleAppkey != "")
    {
        string tempAppKey = bleAppkey;
        LOGD("Appkey: %s", tempAppKey.c_str());
        tempAppKey.erase(std::remove(tempAppKey.begin(), tempAppKey.end(), '-'), tempAppKey.end());
        if (tempAppKey.size() == 32)
        {
            for (size_t i = 0; i < tempAppKey.length(); i += 2)
            {
                std::string hexByte = tempAppKey.substr(i, 2);
                appKey[i / 2] = std::stoi(hexByte, nullptr, 16);
            }
        }
    }

    if (bleAppkey == "" || bleDevicekey == "" || bleNetkey == "" || bleNetkey != netkeyStr)
    {
        ResetBle();

        this->UpdateDeviceKeyGateway(Gateway::GetInstance()->getBleAddr(), Gateway::GetInstance()->getBleDeviceKey());
        vector<Device *> listDevs;
        DeviceManager::GetInstance()->ForEach([&](Device *device)
                                              {
												  if (device->getProtocol() == BLE_DEVICE)
												  {
													  listDevs.push_back(device);
												  } });
        for (auto &device : listDevs)
        {
            this->UpdateDeviceKeyDev(device->getAddr(), device->GetDeviceKey());
        }

        if (bleNetkey != "")
        {
            string tempNetKey = bleNetkey;
            LOGD("Netkey: %s", tempNetKey.c_str());
            tempNetKey.erase(std::remove(tempNetKey.begin(), tempNetKey.end(), '-'), tempNetKey.end());
            if (tempNetKey.size() == 32)
            {
                for (int i = 0; i < tempNetKey.length(); i += 2)
                {
                    std::string hexByte = tempNetKey.substr(i, 2);
                    netKey[i / 2] = std::stoi(hexByte, nullptr, 16);
                }
            }
        }
        else
        {
            for (int i = 0; i < 16; i++)
            {
                netKey[i] = rand() % 256;
            }
            netkeyStr = Util::uuidToStr((uint8_t *)netKey);
            bleNetkey = netkeyStr;
            // Database::GetInstance()->GatewayUpdateNetKey(bleNetkey);
            Gateway::GetInstance()->setBleNetKey(bleNetkey);
            Database::GetInstance()->GatewayUpdateData();
        }
        LOGD("New ble_netkey: %s", bleNetkey.c_str());
        SetNetKey();

        if (bleDevicekey != "")
        {
            string tempDeviceKey = bleDevicekey;
            LOGD("Gwkey: %s", tempDeviceKey.c_str());
            tempDeviceKey.erase(std::remove(tempDeviceKey.begin(), tempDeviceKey.end(), '-'), tempDeviceKey.end());
            if (tempDeviceKey.size() == 32)
            {
                for (int i = 0; i < tempDeviceKey.length(); i += 2)
                {
                    std::string hexByte = tempDeviceKey.substr(i, 2);
                    gwKey[i / 2] = std::stoi(hexByte, nullptr, 16);
                }
            }
        }
        else
        {
            for (int i = 0; i < 16; i++)
            {
                gwKey[i] = rand() % 256;
            }
            bleDevicekey = Util::uuidToStr((uint8_t *)gwKey);
            Gateway::GetInstance()->setBleDeviceKey(bleDevicekey);
            Database::GetInstance()->GatewayUpdateData();
        }

        LOGD("New ble_devicekey: %s", bleDevicekey.c_str());
        SetGwKey();

        if (bleAppkey != "")
        {
            string tempAppKey = bleAppkey;
            LOGD("Appkey: %s", tempAppKey.c_str());
            tempAppKey.erase(std::remove(tempAppKey.begin(), tempAppKey.end(), '-'), tempAppKey.end());
            if (tempAppKey.size() == 32)
            {
                for (int i = 0; i < tempAppKey.length(); i += 2)
                {
                    std::string hexByte = tempAppKey.substr(i, 2);
                    appKey[i / 2] = std::stoi(hexByte, nullptr, 16);
                }
            }
        }
        else
        {
            for (int i = 0; i < 16; i++)
            {
                appKey[i] = appKeyDefault[i];
                // appKey[i] = rand() % 256;
            }
            bleAppkey = Util::uuidToStr((uint8_t *)appKey);
            Gateway::GetInstance()->setBleAppKey(bleAppkey);
            Database::GetInstance()->GatewayUpdateData();
        }
        LOGD("New ble_appkey: %s", bleAppkey.c_str());
        UpdateAppKey(bleAppkey);
    }
}

static void GetDataUpdateLight(uint8_t *data, int len, Json::Value &dataValues)
{
    typedef struct __attribute__((packed))
    {
        uint16_t opcode;
        uint8_t header;
        uint8_t status_mode;
        uint16_t value1;
        uint16_t value2;
        uint16_t value3;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;

    if (data_message->opcode == LIGHTNESS_LINEAR_STATUS && data_message->header == 2)
    {
        dataValues[KEY_ATTRIBUTE_ONOFF] = (data_message->status_mode >> 4) & 0x0F;
        if ((data_message->status_mode & 0x0F) == 1)
        {
            dataValues[KEY_ATTRIBUTE_DIM] = (data_message->value1 * 100) / 65535;
            dataValues[KEY_ATTRIBUTE_CCT] = (data_message->value2 - 800) / 192;
        }
        else if ((data_message->status_mode & 0x0F) == 0)
        {
            if (data_message->value2 > 0 && data_message->value1 > 0 && data_message->value3 > 0)
            {
                dataValues[KEY_ATTRIBUTE_HUE] = data_message->value2;
                dataValues[KEY_ATTRIBUTE_SATURATION] = data_message->value3;
                dataValues[KEY_ATTRIBUTE_LUMINANCE] = data_message->value1;
            }
        }
    }
}

static void GetDataUpdateSwitch(uint8_t *data, int len, Json::Value &dataValues)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t vendorId;
        uint16_t header;
        uint8_t element;
        uint8_t bt;
        uint8_t bt2;
        uint8_t bt3;
        uint8_t bt4;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;

    for (int i = 0; i < data_message->element; i++)
    {
        dataValues[KEY_ATTRIBUTE_BUTTON + ((i) ? to_string(i + 1) : "")] = ((uint8_t *)data_message)[i + offsetof(data_message_t, bt)];
    }
}

static void GetDataUpdateCurtain(uint8_t *data, int len, Json::Value &dataValues)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t vendorId;
        uint16_t header;
        uint8_t type;
        uint8_t curtain;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;

    uint8_t temp = data_message->type;
    switch (temp)
    {
    case 1:
        dataValues[KEY_ATTRIBUTE_CURTAIN_OPEN] = 1;
        dataValues[KEY_ATTRIBUTE_CURTAIN_CLOSE] = 0;
        dataValues[KEY_ATTRIBUTE_CURTAIN_PAUSE] = 0;
        break;
    case 0:
        dataValues[KEY_ATTRIBUTE_CURTAIN_CLOSE] = 1;
        dataValues[KEY_ATTRIBUTE_CURTAIN_OPEN] = 0;
        dataValues[KEY_ATTRIBUTE_CURTAIN_PAUSE] = 0;
        break;
    case 2:
        if (data_message->vendorId == RD_HEADER_REQUEST_STATUS_CURTAIN)
        {
            dataValues[KEY_ATTRIBUTE_CURTAIN_OPENED] = data_message->curtain;
        }
        dataValues[KEY_ATTRIBUTE_CURTAIN_PAUSE] = 1;
        dataValues[KEY_ATTRIBUTE_CURTAIN_OPEN] = 0;
        dataValues[KEY_ATTRIBUTE_CURTAIN_CLOSE] = 0;
        break;
    case 3:
        dataValues[KEY_ATTRIBUTE_CURTAIN_OPENED] = data_message->curtain;
        break;
    }
}

int BleProtocol::GetOpcodeExceptionMessage(message_rsp_st **data)
{
    int rs = CODE_ERROR;
    vectorCheckOpcodeMtx.lock();
    if (messageCheckOpcodeList.size() > 0)
    {
        *data = messageCheckOpcodeList[0];
        messageCheckOpcodeList.erase(messageCheckOpcodeList.begin());
        rs = CODE_OK;
    }
    vectorCheckOpcodeMtx.unlock();
    return rs;
}

void BleProtocol::CheckOpcodeException(message_rsp_st *message_rsp)
{
    // Gateway::GetInstance()->setLastTimePingGwBle(time(NULL));
    // switch (message_rsp->opcode)
    // {
    // case HCI_GATEWAY_CMD_UPDATE_MAC:
    // 	if (IsProvision() && !haveNewMac)
    // 	{
    // 		memcpy(&scanDeviceMessage, message_rsp->data, sizeof(scan_device_message_t));
    // 		haveNewMac = true;
    // 		haveGetMacRsp = false;
    // 	}
    // 	break;

    // case HCI_GATEWAY_RSP_OP_CODE:
    // {
    // 	typedef struct __attribute__((packed))
    // 	{
    // 		uint16_t dev_addr;
    // 		uint16_t gw_addr;
    // 		uint8_t data[100];
    // 	} data_message_t;
    // 	data_message_t *data_message = (data_message_t *)message_rsp->data;
    // 	// LOGV("Device addr 0x%04X", data_message->dev_addr);
    // 	uint16_t opcode = data_message->data[0] | (data_message->data[1] << 8);
    // 	uint16_t header = data_message->data[3] | (data_message->data[4] << 8);
    // 	uint16_t vendorId = data_message->data[1] | (data_message->data[2] << 8);
    // 	DeviceBle *deviceBle = DeviceManager::GetInstance()->GetDeviceBleFromAddr(data_message->dev_addr);
    // 	if (deviceBle)
    // 	{
    // 		deviceBle->UpdateLastTimeActive();
    // 		if (opcode == LIGHTNESS_LINEAR_STATUS && data_message->data[2] == 2)
    // 		{
    // 			Json::Value dataValues = Json::objectValue;
    // 			GetDataUpdateLight(data_message->data, message_rsp->len - 6, dataValues);
    // 			deviceBle->SetPropertyJsonUpdate(dataValues);
    // 		}
    // 		else if (data_message->data[0] == RD_OPCODE_CONFIG_RSP && vendorId == RD_VENDOR_ID && header == RD_HEADER_REQUEST_STATUS_SWITCH)
    // 		{
    // 			Json::Value dataValues = Json::objectValue;
    // 			GetDataUpdateSwitch(data_message->data, message_rsp->len - 6, dataValues);
    // 			deviceBle->SetPropertyJsonUpdate(dataValues);
    // 		}
    // 		else if (data_message->data[0] == RD_OPCODE_CONFIG_RSP && vendorId == RD_VENDOR_ID && header == RD_HEADER_REQUEST_STATUS_CURTAIN)
    // 		{
    // 			Json::Value dataValues = Json::objectValue;
    // 			GetDataUpdateCurtain(data_message->data, message_rsp->len - 6, dataValues);
    // 			deviceBle->SetPropertyJsonUpdate(dataValues);
    // 		}
    // 		else if (data_message->data[0] == RD_OPCODE_CONFIG_RSP && vendorId == RD_VENDOR_ID && header == RD_HEADER_SEFTPOWER_REMOTE_PRESS)
    // 		{
    // 			DeviceBle *deviceBleChild = DeviceManager::GetInstance()->GetDeviceBleFromAddr(data_message->data[5] | (data_message->data[6] << 8));
    // 			if (deviceBleChild)
    // 			{
    // 				deviceBleChild->DeviceInputData(data_message->data, message_rsp->len - 6, data_message->data[5] | (data_message->data[6] << 8));
    // 			}
    // 		}
    // 		else
    // 		{
    // 			deviceBle->DeviceInputData(data_message->data, message_rsp->len - 6, data_message->dev_addr);
    // 		}

    // 		if (deviceBle->getType() == BLE_AC_SCENE_SCREEN_TOUCH || deviceBle->getType() == BLE_SWITCH_KNOB)
    // 		{
    // 			if (header == RD_HEADER_SCREEN_TOUCH_REQUEST_TIME)
    // 			{
    // 				SendDate(deviceBle->getAddr(), Util::GetYearsCurrent(), Util::GetMonthsCurrent(), Util::GetDateCurrent(), Util::GetDaysCurrent());
    // 				SendTime(deviceBle->getAddr(), Util::GetHoursCurrent(), Util::GetMinutesCurrent(), Util::GetSecondsCurrent());
    // 			}
    // 			else if (header == RD_HEADER_SCREEN_TOUCH_REQUEST_TEMP)
    // 			{
    // 				SendWeatherOutdoor(deviceBle->getAddr(), Util::GetStatusWeatherOutdoor(), Util::GetTempWeatherOutdoor());
    // 				SendWeatherIndoor(deviceBle->getAddr(), Util::GetTempOfScreenTouch() / 10, Util::GetHumOfScreenTouch() / 10, 0);
    // 			}
    // 		}
    // 		break;
    // 	}
    // 	else
    // 	{
    // 		LOGW("Not found device addr: 0x%04X", data_message->dev_addr);
    // 	}
    // 	break;
    // }

    // case HCI_GATEWAY_CMD_SEND_NODE_INFO:
    // {
    // 	for (int i = 0; i < 16; i++)
    // 	{
    // 		deviceKey[i] = message_rsp->data[i + 4];
    // 	}
    // 	break;
    // }
    // default:
    // 	break;
    // }
}

int BleProtocol::OnMessage(unsigned char *data, int len)
{
    // LOGD("OnMessage len: %d", len);
    uint8_t *d = data;
    int l = len;
    message_rsp_st *message_rsp = NULL;
    message_rsp_st *old_message_rsp = NULL;
    bool is_dupplicate = false;
    bool match;
    Util::LedBle(false);
    Util::LedServiceLock();
#ifdef ESP_PLATFORM
    SetLedService(false);
#endif
    while (l >= 5)
    {
        message_rsp = (message_rsp_st *)d;
        if (message_rsp->len >= 3 && message_rsp->len <= 36)
        {
            if (message_rsp->magic == 0x80 ||
                message_rsp->magic == 0x90 ||
                message_rsp->magic == 0x91 ||
                message_rsp->magic == 0x92 ||
                message_rsp->magic == 0xfa)
            {
                if (haveGetMacRsp || (!haveGetMacRsp && message_rsp->opcode != HCI_GATEWAY_CMD_UPDATE_MAC))
                {
                    uint16_t packageLen = message_rsp->len + 2;
                    is_dupplicate = false;
                    if (old_message_rsp && message_rsp->len == old_message_rsp->len)
                    {
                        is_dupplicate = true;
                        for (int i = 0; i < message_rsp->len; i++)
                        {
                            if (message_rsp->data[i] != old_message_rsp->data[i])
                            {
                                is_dupplicate = false;
                                break;
                            }
                        }
                    }
                    if (!is_dupplicate)
                    {
                        if (message_rsp->len >= 3 && message_rsp->len <= l - 2)
                        {
                            // LOGD("onMessage opcode: 0x%02X, len: %d", message_rsp->opcode, message_rsp->len);
                            messageRespListMtx.lock();
                            for (auto &messageResp : messageRespList)
                            {
                                if (message_rsp->opcode == messageResp->opcode)
                                {
                                    match = true;
                                    if (messageResp->compare_data)
                                    {
                                        for (int i = 0; i < messageResp->compare_len; i++)
                                        {
                                            if (message_rsp->data[messageResp->compare_position + i] != messageResp->compare_data[i])
                                                match = false;
                                        }
                                    }
                                    if (match)
                                    {
                                        messageResp->status = true;
                                        if (messageResp->len)
                                        {
                                            *(messageResp->len) = message_rsp->len - 2;
                                            if (*(messageResp->len) > 0)
                                                if (messageResp->data)
                                                    memcpy(messageResp->data, message_rsp->data, *messageResp->len);
                                        }
                                    }
                                }
                            }
                            messageRespListMtx.unlock();
                            vectorCheckOpcodeMtx.lock();
                            if (messageCheckOpcodeList.size() < BLE_CHECK_OPCODE_BUFFER_MAX_SIZE)
                            {
#ifdef ESP_PLATFORM
                                message_rsp_st *messageCheckOpcode = (message_rsp_st *)heap_caps_malloc_prefer(packageLen, 2, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
#else
                                // CheckOpcodeException(message_rsp);
                                message_rsp_st *messageCheckOpcode = (message_rsp_st *)malloc(packageLen);
#endif
                                if (messageCheckOpcode)
                                {
                                    memcpy(messageCheckOpcode, message_rsp, packageLen);
                                    messageCheckOpcodeList.push_back(messageCheckOpcode);
                                }
                                else
                                {
                                    LOGE("malloc failed at OnMessage with len=%d", packageLen);
                                }
                            }
                            vectorCheckOpcodeMtx.unlock();
                        }
                        else if (message_rsp->len < 3 || message_rsp->len > 36)
                        {
                            LOGW("Wrong uart data");
                            l = 0;
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                    old_message_rsp = message_rsp;
                    l -= packageLen;
                    d += packageLen;
                }
                else
                {
                    d++;
                    l--;
                }
            }
            else
            {
                d++;
                l--;
            }
        }
        else
        {
            d++;
            l--;
        }
        usleep(300);
    }
    Util::LedBle(true);
    Util::LedServiceUnlock();
#ifdef ESP_PLATFORM
    SLEEP_MS(70);
    SetLedService(true);
#endif
    return l;
}

int BleProtocol::SendMessage(uint16_t opReq, uint8_t *dataReq, int lenReq, uint8_t opRsp, uint8_t *dataRsp, int *lenRsp, uint32_t timeout, uint8_t *compare_data, int compare_position, int compare_len)
{
    // uint32_t countDelay = timeout / 10;
    // mtxWaitSendUart.lock();
    // int rs = CODE_OK;
    // message_rsp_list_st message_rsp_list = {
    //     .status = false,
    //     .opcode = opRsp,
    //     .len = lenRsp,
    //     .data = dataRsp,
    //     .compare_data = compare_data,
    //     .compare_position = compare_position,
    //     .compare_len = compare_len,
    // };
    // if (opRsp)
    // {
    //     messageRespListMtx.lock();
    //     messageRespList.push_back(&message_rsp_list);
    //     messageRespListMtx.unlock();
    // }
    // message_req_st message_req = {
    //     .opcode = opReq,
    // };
    // for (int i = 0; i < lenReq; i++)
    // {
    //     message_req.data[i] = dataReq[i];
    // }
    // Write((uint8_t *)&message_req, lenReq + 2);
    // if (opRsp)
    // {
    //     while (!message_rsp_list.status && countDelay--)
    //     {
    //         SLEEP_MS(10);
    //     }
    //     if (!message_rsp_list.status)
    //     {
    //         rs = CODE_ERROR;
    //     }
    //     messageRespListMtx.lock();
    //     messageRespList.erase(remove(messageRespList.begin(), messageRespList.end(), &message_rsp_list), messageRespList.end());
    //     messageRespListMtx.unlock();
    // }
    // else
    // {
    //     usleep(1000 * timeout);
    // }
    // mtxWaitSendUart.unlock();
    // return rs;
    // return Write(dataReq, lenReq);
    return -1;
}

/*
string BleProtocol::GetAppKey()
{
    string appkeyStr = Gateway::GetInstance()->getBleAppKey();
    if (appkeyStr.compare("") == 0)
    {
        LOGD("Appkey null");
        // srand((int)time(0));
        for (int i = 0; i < 16; i++)
        {
            appKey[i] = appKeyDefault[i];
        }
        string appkey = Util::uuidToStr((uint8_t *)appKey);
        LOGD("New ble_appkey: %s", appkey.c_str());
        Database::GetInstance()->GatewayUpdateAppKey(appkey);
        Gateway::GetInstance()->setBleAppkey(appkey);
    }
    else
    {
        LOGD("Appkey: %s", appkeyStr.c_str());
        appkeyStr.erase(appkeyStr.begin() + 8, appkeyStr.begin() + 9);
        appkeyStr.erase(appkeyStr.begin() + 12, appkeyStr.begin() + 13);
        appkeyStr.erase(appkeyStr.begin() + 16, appkeyStr.begin() + 17);
        appkeyStr.erase(appkeyStr.begin() + 20, appkeyStr.begin() + 21);
        if (appkeyStr.length() % 2 == 0 && appkeyStr.length() > 0)
        {
            for (int i = 0; i < appkeyStr.length(); i += 2)
            {
                std::string hexByte = appkeyStr.substr(i, 2);
                appKey[i / 2] = std::stoi(hexByte, nullptr, 16);
            }
        }
        else
        {
            LOGW("App key error");
        }
    }
    return appkeyStr;
}
*/

int BleProtocol::GetNetKey()
{
    LOGD("GetNetKey");
    // uint8_t d = HCI_GATEWAY_CMD_GET_PRO_SELF_STS;
    // uint8_t dataRsp[100];
    // int lenRsp;
    // int rs = SendMessage(SYSTEM_REQ, &d, 1, HCI_GATEWAY_CMD_PRO_STS_RSP, dataRsp, &lenRsp, 5000);
    // if (rs == CODE_OK)
    // {
    // 	// pro_net_info = (pro_net_info_t *)&dataRsp[1];
    // 	memcpy(&pro_net_info.netKey[0], &dataRsp[1], sizeof(pro_net_info_t));

    // 	uint32_t ivIndex = bswap_32(pro_net_info.iv_index);
    // 	if ((ivIndex == 0x11223344))
    // 	{
    // 		// Database::GetInstance()->GatewayUpdateIvIndex(ivIndex);
    // 		for (int i = 0; i < 16; i++)
    // 		{
    // 			netKey[i] = pro_net_info.netKey[i];
    // 		}

    // 		nextAddr = pro_net_info.unicast_address;
    // 		if (nextAddr == 0)
    // 			nextAddr = 2;
    // 		uint32_t maxAddr = DeviceManager::GetInstance()->GetMaxAddrBle();
    // 		if (nextAddr <= maxAddr)
    // 			nextAddr = maxAddr + BLE_MAX_ELEMENT;
    // 		LOGW("nextAddr: 0x%04X - %d", nextAddr, nextAddr);
    // 	}
    // 	else
    // 	{
    // 		srand((int)time(0));
    // 		for (int i = 0; i < 16; i++)
    // 		{
    // 			netKey[i] = rand() % 256;
    // 		}
    // 		string netkeyStr = Util::uuidToStr((uint8_t *)netKey);
    // 	}
    // }
    // else
    // {
    // 	LOGE("Send GetNetKey error, rs: %d", rs);
    // 	rs = CODE_ERROR;
    // }
    // return rs;
    return CODE_ERROR;
}

int BleProtocol::SetNetKey()
{
    LOGD("SetNetKey");
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint8_t netKey[16];
        uint8_t rev[3];
        uint32_t magic;
        uint16_t addr;
    } set_netkey_message_t;
    set_netkey_message_t set_netkey_message;
    memset(&set_netkey_message, 0x00, sizeof(set_netkey_message));
    set_netkey_message.opcode = HCI_GATEWAY_CMD_SET_PRO_PARA;
    for (int i = 0; i < 16; i++)
    {
        set_netkey_message.netKey[i] = netKey[i];
    }
    set_netkey_message.magic = 0x44332211;
    set_netkey_message.addr = 0x01;
    Gateway::GetInstance()->setBleAddr(0x01);
    Gateway::GetInstance()->setBleIvIndex(bswap_32(set_netkey_message.magic));
    Database::GetInstance()->GatewayUpdateData();
    return SendMessage(SYSTEM_REQ, (uint8_t *)&set_netkey_message, sizeof(set_netkey_message_t), HCI_GATEWAY_CMD_SEND_IVI, 0, 0, 1000);
}

int BleProtocol::SetGwKey()
{
    LOGD("SetGwKey");
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint8_t addr[2];
        uint8_t gwKey[16];
    } set_gwkey_message_t;
    set_gwkey_message_t set_gwkey_message;
    set_gwkey_message.opcode = 0x0D;
    set_gwkey_message.addr[0] = 0x01;
    set_gwkey_message.addr[1] = 0x00;
    for (int i = 0; i < 16; i++)
    {
        set_gwkey_message.gwKey[i] = gwKey[i];
    }
    return SendMessage(SYSTEM_REQ, (uint8_t *)&set_gwkey_message, sizeof(set_gwkey_message_t), 0, 0, 0, 1000);
}

int BleProtocol::StartScan()
{
    LOGD("StartScan BLE");
    uint8_t d = HCI_GATEWAY_CMD_START;
    // SetProvisioning(true);
    int rs = SendMessage(SYSTEM_REQ, &d, 1, 0, 0, 0, 0);
    if (rs)
    {
        LOGE("Send start scan error, rs: %d", rs);
    }
    return rs;
}

int BleProtocol::StopScan()
{
    LOGD("StopScan");
    uint8_t d = HCI_GATEWAY_CMD_STOP;
    SetProvisioning(false);
    int rs = SendMessage(SYSTEM_REQ, &d, 1, 0, 0, 0, 0);
    if (rs)
    {
        LOGE("Send stop scan error, rs: %d", rs);
    }
    return rs;
}

int BleProtocol::ResetBle()
{
    LOGD("ResetBle");
    uint8_t d = HCI_GATEWAY_CMD_RESET;
    int rs = SendMessage(SYSTEM_REQ, &d, 1, 0, 0, 0, 8000);
    if (rs)
    {
        LOGE("Send reset factory error, rs: %d", rs);
    }
    return rs;
}

int BleProtocol::ResetFactory()
{
    LOGD("ResetFactory");
    initKey(Gateway::GetInstance()->getBleNetKey(), Gateway::GetInstance()->getBleAppKey());
    Database::GetInstance()->GatewayRead();
    return CODE_OK;
}

bool BleProtocol::IsProvision()
{
    return isProvisioning;
}

void BleProtocol::SetProvisioning(bool isProvision)
{
    this->isProvisioning = isProvision;
}

int BleProtocol::AddDevice(scan_device_message_t *scan_device_message)
{
    // TODO: convert to non-blocking func
    LOGD("AddDevice");
    // #ifdef ESP_PLATFORM
    // 	Led::TaskLedService(MODE_BLINK);
    // #endif
    uint16_t version = 0;
    uint32_t deviceType = 0;
    uuid_t *uuid = (uuid_t *)scan_device_message->uuid;
    string mac = Util::ConvertU32ToHexString(scan_device_message->mac, sizeof(scan_device_message->mac));
    LOGI("Scan device mac 0x%s, rssi: %i", mac.c_str(), scan_device_message->rssi);
    int rs = CODE_ERROR;
    if (IsProvision() && !SelectMac(scan_device_message->mac))
    {
        if (IsProvision() && !GetNetKey())
        {
            if (IsProvision() && !Provision(nextAddr))
            {
                if (IsProvision() && !BindingAll())
                {
                    if (IsProvision() && !SetGwAddr(nextAddr, Gateway::GetInstance()->getBleAddr()))
                    {
                        if (IsProvision() && !GetDeviceType(scan_device_message->mac, nextAddr, deviceType, version))
                        {
                            Json::Value dataJson;
                            dataJson["devicekey"] = Util::uuidToStr((uint8_t *)deviceKey);
                            Device *device = DeviceManager::GetInstance()->AddDevice(mac, deviceType, nextAddr, version, &dataJson);
                            if (device)
                            {
                                Json::Value onlineValue;
                                onlineValue["stt"] = 1;
                                Database::GetInstance()->DeviceAdd(device);
                                if (Gateway::GetInstance()->DeviceAddNew(device) == CODE_OK)
                                {
                                    device->PushTelemetry(onlineValue);
                                }
                                // if (device->getType() == MATTER_DOWNLIGHT_RGBCW ||
                                // 	device->getType() == MATTER_DOOR_SENSOR ||
                                // 	device->getType() == MATTER_DOWNLIGHT_SMT ||
                                // 	device->getType() == MATTER_SWITCH_1_V2 ||
                                // 	device->getType() == MATTER_SWITCH_1_V2)
                                // 	Gateway::GetInstance()->DeviceMatterRegister(device);
                                rs = CODE_OK;
                            }
                        }
                    }
                }
                if (rs != CODE_OK)
                {
                    ResetDev(nextAddr);
                }
            }
        }
    }

    if (IsProvision())
    {
        sleep(1);
        if (IsProvision())
            StartScan();
    }

    // #ifdef ESP_PLATFORM
    // 	Led::SetModeLedService(MODE_ON);
    // 	Led::Led::ServiceLedOnOff(MODE_ON);
    // #endif

    return rs;
}

int BleProtocol::AddPairDevice(uint32_t parentAddr, uint32_t childAddr)
{
    // if (ScanStopSeftPowerRemote(parentAddr, 1) == CODE_OK)
    // {
    // 	uint16_t adrMax = DeviceManager::GetInstance()->GetMaxAddrBle() + BLE_MAX_ELEMENT;
    // 	LOGW("Max address is %d", adrMax);
    // 	if (SaveSeftPowerRemote(scanDevicePairMessage, adrMax) == CODE_OK)
    // 	{
    // 		uint16_t version = 0;
    // 		uint32_t deviceType = 0;
    // 		char tempId[100] = {0};
    // 		sprintf((char *)tempId, "%02x%02x%02x%02x-0000-0000-0000-000000000000", scanDevicePairMessage.mac[0], scanDevicePairMessage.mac[1], scanDevicePairMessage.mac[2], scanDevicePairMessage.mac[3]);
    // 		tempId[36] = '\0';
    // 		std::string uuid(tempId);

    // 		char tempMac[100] = {0};
    // 		sprintf((char *)tempMac, "%02x%02x%02x%02x0000", scanDevicePairMessage.mac[0], scanDevicePairMessage.mac[1], scanDevicePairMessage.mac[2], scanDevicePairMessage.mac[3]);
    // 		tempMac[12] = '\0';
    // 		std::string mac(tempMac);

    // 		switch (scanDevicePairMessage.type)
    // 		{
    // 		case 1:
    // 			deviceType = BLE_SEFTPOWER_REMOTE_1;
    // 			break;
    // 		case 2:
    // 			deviceType = BLE_SEFTPOWER_REMOTE_2;
    // 			break;
    // 		case 3:
    // 			deviceType = BLE_SEFTPOWER_REMOTE_3;
    // 			break;
    // 		case 6:
    // 			deviceType = BLE_SEFTPOWER_REMOTE_6;
    // 			break;
    // 		}
    // 		Device *device = DeviceManager::GetInstance()->AddDevice(mac, deviceType, childAddr, 257);
    // 		if (device)
    // 		{
    // 			Database::GetInstance()->DeviceAdd(device);
    // 			Gateway::GetInstance()->DeviceAddNew(device);
    // 			// Device *parent = DeviceManager::GetInstance()->GetDeviceBleFromAddr(parentAddr);
    // 			// DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(device);
    // 			// if (parent && deviceBleSeftPowerRemote)
    // 			// {
    // 			// 	deviceBleSeftPowerRemote->SetParentDev(parent);
    // 			// }
    // 			// if (deviceBleSeftPowerRemote->GetParent())
    // 			// {
    // 			// 	Database::GetInstance()->DeviceBleChildAdd(deviceBleSeftPowerRemote, parent, "");
    // 			// }
    // 		}
    // 	}
    // 	// else
    // 	// ScanStopSeftPowerRemote(parentAddr, 0);
    // }
    // else
    // 	ScanStopSeftPowerRemote(parentAddr, 0);
    return CODE_OK;
}

int BleProtocol::SelectMac(uint8_t *mac)
{
    LOGD("SelectMac");
    uint8_t data[7];
    data[0] = HCI_GATEWAY_CMD_SET_ADV_FILTER;
    for (int i = 0; i < 6; i++)
    {
        data[i + 1] = mac[i];
    }
    return SendMessage(SYSTEM_REQ, data, sizeof(data), 0, 0, 0, 1000);
}

int BleProtocol::Provision(uint16_t deviceAddr)
{
    LOGD("Provision: deviceAddr 0x%x", deviceAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        pro_net_info_t data_pro;
    } provision_message_t;
    provision_message_t provision_message;
    memset(&provision_message, 0x00, sizeof(provision_message));
    provision_message.opcode = HCI_GATEWAY_CMD_SET_NODE_PARA;
    memcpy(&provision_message.data_pro.netKey[0], &pro_net_info.netKey[0], sizeof(pro_net_info_t));
    provision_message.data_pro.unicast_address = deviceAddr;
    int rs = SendMessage(SYSTEM_REQ, (uint8_t *)&provision_message, sizeof(provision_message_t), HCI_GATEWAY_CMD_PROVISION_EVT, dataRsp, &lenRsp, 30000);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint8_t status;
            uint8_t data[24];
        } provision_rsp_message_t;
        provision_rsp_message_t *provision_rsp_message = (provision_rsp_message_t *)dataRsp;
        if (provision_rsp_message->status)
        {
            LOGD("Provision OK");
            return CODE_OK;
        }
        else
        {
            LOGW("Must hard reset Ble module");
        }
    }
    LOGW("Provision err");
    return CODE_ERROR;
}

int BleProtocol::BindingAll()
{
    LOGD("BindingAll");
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint8_t rev[3];
        uint8_t appKey[16];
    } binding_all_message_t;
    binding_all_message_t binding_all_message;
    memset(&binding_all_message, 0x00, sizeof(binding_all_message));
    binding_all_message.opcode = HCI_GATEWAY_CMD_START_KEYBIND;
    for (int i = 0; i < 16; i++)
    {
        binding_all_message.appKey[i] = appKey[i];
    }
    int rs = SendMessage(SYSTEM_REQ, (uint8_t *)&binding_all_message, sizeof(binding_all_message_t), HCI_GATEWAY_CMD_KEY_BIND_EVT, dataRsp, &lenRsp, 30000);
    if (rs == CODE_OK)
    {
        if (lenRsp == 1 && dataRsp[0] == 1)
        {
            LOGD("BindingAll OK");
            return CODE_OK;
        }
    }
    LOGW("BindingAll err");
    return CODE_ERROR;
}

int BleProtocol::SetGwAddr(uint16_t devAddr, uint16_t gwAddrSet)
{
    LOGD("SetGwAddr");
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t setGwAddrHeader[] = {RD_OPCODE_PROVISION_RSP, 0x11, 0x02, 0x02, 0x00};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t gwAddr;
        uint8_t data[4];
    } set_gw_addr_message_t;
    set_gw_addr_message_t set_gw_addr_message;
    memset(&set_gw_addr_message, 0x00, sizeof(set_gw_addr_message));
    set_gw_addr_message.ble_message_header.devAddr = devAddr;
    set_gw_addr_message.opcodeVendor = RD_OPCODE_PROVISION;
    set_gw_addr_message.vendorId = RD_VENDOR_ID;
    set_gw_addr_message.opcodeRsp = RD_OPCODE_PROVISION_RSP;
    set_gw_addr_message.header = RD_HEADER_PROVISION_SET_GW_ADDR;
    set_gw_addr_message.gwAddr = gwAddrSet;
    int rs = SendMessage(APP_REQ, (uint8_t *)&set_gw_addr_message, sizeof(set_gw_addr_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 5000, setGwAddrHeader, 4, 5);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcode;
            uint16_t vendorId;
            uint16_t header;
            uint8_t rev[6];
        } set_gw_addr_rsp_message_t;
        set_gw_addr_rsp_message_t *set_gw_addr_rsp_message = (set_gw_addr_rsp_message_t *)dataRsp;
        if (set_gw_addr_rsp_message->devAddr == devAddr)
        {
            if (set_gw_addr_rsp_message->opcode == RD_OPCODE_PROVISION_RSP &&
                set_gw_addr_rsp_message->vendorId == RD_VENDOR_ID &&
                set_gw_addr_rsp_message->header == RD_HEADER_PROVISION_SET_GW_ADDR)
            {
                LOGD("SetGwAddr OK");
                return CODE_OK;
            }
        }
    }
    LOGW("SetGwAddr err");
    return CODE_ERROR;
}

/**
 * @brief Gen Ble security key (6 bytes)
 *
 * @param mac mac of destination device
 * @param devAddr unicast addr of destination device
 * @param out out buffer to write key
 */
static void genSecurityKey(uint8_t *mac, uint16_t devAddr, uint8_t *out)
{
    AES aes(AESKeyLength::AES_128);
    memcpy(plaintext + 8, mac, 6);
    memcpy(plaintext + 14, (uint8_t *)&devAddr, 2);
    unsigned char *outAes = aes.EncryptECB(plaintext, 32, keyAes);
    for (int i = 0; i < 6; i++)
    {
        out[i] = outAes[i + 10];
    }
    free(outAes);
}

int BleProtocol::GetDeviceType(uint8_t *mac, uint16_t devAddr, uint32_t &deviceType, uint16_t &deviceVersion)
{
    LOGD("GetDeviceType");
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t checkTypeHeader[] = {0xe1, 0x11, 0x02, 0x03, 0x00};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t data[6];
    } check_type_message_t;
    check_type_message_t check_type_message;
    memset(&check_type_message, 0x00, sizeof(check_type_message));
    check_type_message.ble_message_header.devAddr = devAddr;
    check_type_message.opcodeVendor = RD_OPCODE_PROVISION;
    check_type_message.vendorId = RD_VENDOR_ID;
    check_type_message.opcodeRsp = RD_OPCODE_PROVISION_RSP;
    check_type_message.header = RD_HEADER_PROVISION_GET_DEV_TYPE;
    genSecurityKey(mac, devAddr, &check_type_message.data[0]);
    int rs = SendMessage(APP_REQ, (uint8_t *)&check_type_message, sizeof(check_type_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 5000, checkTypeHeader, 4, 5);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcode;
            uint16_t vendorId;
            uint16_t header;
            uint8_t deviceType[3];
            uint8_t magic;
            uint8_t version[2];
        } check_type_rsp_message_t;
        check_type_rsp_message_t *check_type_rsp_message = (check_type_rsp_message_t *)dataRsp;
        if (check_type_rsp_message->devAddr == devAddr)
        {
            if (check_type_rsp_message->opcode == RD_OPCODE_PROVISION_RSP &&
                check_type_rsp_message->vendorId == RD_VENDOR_ID &&
                check_type_rsp_message->header == RD_HEADER_PROVISION_GET_DEV_TYPE)
            {
                deviceType = (check_type_rsp_message->deviceType[0] << 16) | (check_type_rsp_message->deviceType[1] << 8) | check_type_rsp_message->deviceType[2];
                deviceVersion = (check_type_rsp_message->version[0] << 8) | (check_type_rsp_message->version[1]);
                LOGD("GetDeviceType OK, deviceType: 0x%06X, version: %d", deviceType, deviceVersion);
                return CODE_OK;
            }
        }
    }
    LOGW("GetDeviceType err");
    return CODE_ERROR;
}

int BleProtocol::ResetDev(uint16_t devAddr)
{
    LOGD("Reset dev addr: 0x%04X", devAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
    } reset_message_t;
    reset_message_t reset_message = {0};
    memset(&reset_message, 0x00, sizeof(reset_message));
    uint8_t resetHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x80, 0x4a};
    reset_message.ble_message_header.devAddr = devAddr;
    reset_message.opcode = NODE_RESET;
    int rs = SendMessage(APP_REQ, (uint8_t *)&reset_message, sizeof(reset_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, resetHeader, 0, 6);
    if (rs == CODE_OK)
    {
        return CODE_OK;
    }
    return CODE_ERROR;
}

int BleProtocol::ResetDelAll()
{
    LOGD("Reset all dev addr");
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t data[6];
    } reset_message_t;
    reset_message_t reset_message = {0};
    memset(&reset_message, 0x00, sizeof(reset_message));
    reset_message.ble_message_header.devAddr = 0xFFFF;
    reset_message.opcodeVendor = RD_OPCODE_PROVISION;
    reset_message.vendorId = RD_VENDOR_ID;
    reset_message.opcodeRsp = RD_OPCODE_PROVISION_RSP;
    reset_message.header = 0xFFFF;
    for (int i = 0; i < 6; i++)
    {
        reset_message.data[i] = i + 1;
    }

    int rs = SendMessage(APP_REQ, (uint8_t *)&reset_message, sizeof(reset_message_t), 0, 0, 0, 1000);
    if (rs == CODE_OK)
    {
        return CODE_OK;
    }
    return CODE_ERROR;
}

int BleProtocol::GetTTL(uint16_t devAddr)
{
    LOGD("GetTTL");
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
    } ttl_message_t;
    ttl_message_t ttl_message = {0};
    memset(&ttl_message, 0x00, sizeof(ttl_message));
    uint8_t getOnOffHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x80, 0x0e};
    ttl_message.ble_message_header.devAddr = devAddr;
    ttl_message.opcode = CFG_DEFAULT_TTL_GET;
    int rs = SendMessage(APP_REQ, (uint8_t *)&ttl_message, sizeof(ttl_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 800, getOnOffHeader, 0, 6);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint16_t opcode;
            uint8_t data[1];
        } ttl_rsp_message_t;
        ttl_rsp_message_t *ttl_rsp_message = (ttl_rsp_message_t *)dataRsp;
        if (ttl_rsp_message->opcode == CFG_DEFAULT_TTL_STATUS)
        {
            return CODE_OK;
        }
    }
    LOGW("Get TTL error");
    return CODE_ERROR;
}

int BleProtocol::SendOnlineCheck(uint16_t devAddr, uint32_t typeDev, uint16_t version)
{
    // LOGV("SendOnlineCheck addr: 0x%04X", devAddr);
    // #ifndef CONFIG_SAVE_ATTRIBUTE
    switch (typeDev)
    {
    case BLE_LED_CHIEU_TRANH:
    case BLE_LED_CHIEU_GUONG:
    case BLE_DEN_BAN:
    case BLE_DOWNLIGHT_SMT:
    case BLE_DOWNLIGHT_COB_GOC_HEP:
    case BLE_DOWNLIGHT_COB_GOC_RONG:
    case BLE_DOWNLIGHT_COB_TRANG_TRI:
    case BLE_LED_FLOOD:
    case BLE_LED_AT39:
    case BLE_LED_AT40:
    case BLE_LED_AT41:
    case BLE_LED_DAY_LINEAR:
    case BLE_LED_OP_TRAN:
    case BLE_LED_OP_TUONG:
    case BLE_LED_OP_TRAN_LOA:
    case BLE_PANEL_TRON:
    case BLE_PANEL_VUONG:
    case BLE_TRACKLIGHT:
    case BLE_LED_THA_TRAN:
    case BLE_LED_TUBE_M16:
    case BLE_LED_RLT03_06W:
    case BLE_LED_RLT02_10W:
    case BLE_LED_RLT02_20W:
    case BLE_LED_RLT01_10W:
    case BLE_LED_TRL08_20W:
    case BLE_LED_TRL08_10W:
    case BLE_LED_RLT03_12W:
    case BLE_DOWNLIGHT_RGBCW:
    case BLE_LED_DAY_RGBCW:
    case BLE_LED_BULB:
    case BLE_LED_DAY_RGB:
        if (version > 256)
            BleProtocol::UpdateLights(devAddr);
        else
            BleProtocol::GetOnOffLight(devAddr);
        break;
    case BLE_SWITCH_ONOFF:
    case BLE_SWITCH_ONOFF_V2:
        BleProtocol::GetOnOffLight(devAddr);
        break;
    case BLE_SWITCH_RGB_1:
    case BLE_SWITCH_RGB_1_SQUARE:
    case BLE_SWITCH_RGB_WATER_HEATER:
    case BLE_SWITCH_RGB_2:
    case BLE_SWITCH_RGB_2_SQUARE:
    case BLE_SWITCH_RGB_3:
    case BLE_SWITCH_RGB_3_SQUARE:
    case BLE_SWITCH_RGB_4:
    case BLE_SWITCH_RGB_4_SQUARE:
    case BLE_SWITCH_RGB_1_V2:
    case BLE_SWITCH_RGB_2_V2:
    case BLE_SWITCH_RGB_3_V2:
    case BLE_SWITCH_RGB_4_V2:
    case BLE_SWITCH_RGB_1_SQUARE_V2:
    case BLE_SWITCH_RGB_2_SQUARE_V2:
    case BLE_SWITCH_RGB_3_SQUARE_V2:
    case BLE_SWITCH_RGB_4_SQUARE_V2:
    case BLE_SWITCH_ELECTRICAL_1:
    case BLE_SWITCH_ELECTRICAL_2:
    case BLE_SWITCH_ELECTRICAL_3:
    case BLE_SWITCH_ELECTRICAL_4:
    case BLE_SWITCH_ELECTRICAL_WATER_HEATER:
    case BLE_SWITCH_ELECTRICAL_1_V2:
    case BLE_SWITCH_ELECTRICAL_2_V2:
    case BLE_SWITCH_ELECTRICAL_3_V2:
    case BLE_SWITCH_1:
    case BLE_SWITCH_WATER_HEATER:
    case BLE_SWITCH_2:
    case BLE_SWITCH_3:
    case BLE_SWITCH_4:
    case BLE_WIFI_SWITCH_1:
    case BLE_WIFI_SWITCH_2:
    case BLE_WIFI_SWITCH_3:
    case BLE_WIFI_SWITCH_4:
    case BLE_WIFI_SWITCH_1_SQUARE:
    case BLE_WIFI_SWITCH_2_SQUARE:
    case BLE_WIFI_SWITCH_3_SQUARE:
    case BLE_WIFI_SWITCH_4_SQUARE:
    case BLE_WIFI_SWITCH_ELECTRICAL_1:
    case BLE_WIFI_SWITCH_ELECTRICAL_2:
    case BLE_WIFI_SWITCH_ELECTRICAL_3:
    case BLE_SWITCH_KNOB:
        BleProtocol::UpdateStatusRelaySwitch(devAddr, typeDev);
        break;
    case BLE_SWITCH_CURTAIN:
    case BLE_SWITCH_RGB_CURTAIN:
    case BLE_SWITCH_RGB_CURTAIN_SQUARE:
    case BLE_SWITCH_RGB_CURTAIN_HCN:
    case BLE_SWITCH_RGB_CURTAIN_SQUARE_V2:
    case BLE_SWITCH_ROOLING_DOOR:
    case BLE_SWITCH_ROOLING_DOOR_V2:
    case BLE_SWITCH_ROOLING_DOOR_SQUARE:
    case BLE_WIFI_SWITCH_CURTAIN:
    case BLE_WIFI_SWITCH_CURTAIN_SQUARE:
    case BLE_WIFI_SWITCH_ROOLING_DOOR:
    case BLE_WIFI_SWITCH_ROOLING_DOOR_SQUARE:
        BleProtocol::UpdateStatusCurtain(devAddr);
        break;
    default:
        BleProtocol::GetTTL(devAddr);
        break;
    }
    return CODE_OK;
}

int BleProtocol::SetOnOffLight(uint16_t devAddr, uint8_t onoff, uint16_t transition, bool ack)
{
    LOGD("Set OnOff addr: 0x%04X value %d", devAddr, onoff);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint8_t onoff;
        uint8_t rev2;
        uint16_t transition;
    } onoff_message_t;
    onoff_message_t onoff_message = {0};
    memset(&onoff_message, 0x00, sizeof(onoff_message));
    if (ack)
    {
        struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint16_t opcodeRsp;
        } turnOnOffHeader = {
            .devAddr = devAddr,
            .gwAddr = 0x0001,
            .opcodeRsp = G_ONOFF_STATUS,
        };
        onoff_message.ble_message_header.devAddr = devAddr;
        onoff_message.opcode = G_ONOFF_SET;
        onoff_message.onoff = onoff;
        onoff_message.rev2 = 0;
        onoff_message.transition = transition;
        int rs = 0;
        rs = SendMessage(APP_REQ, (uint8_t *)&onoff_message, sizeof(onoff_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, (uint8_t *)&turnOnOffHeader, 0, sizeof(turnOnOffHeader));
        if (rs == CODE_OK)
        {
            typedef struct __attribute__((packed))
            {
                uint16_t devAddr;
                uint16_t gwAddr;
                uint16_t opcode;
                uint8_t data[3];
            } onoff_rsp_message_t;
            onoff_rsp_message_t *onoff_rsp_message = (onoff_rsp_message_t *)dataRsp;
            if (onoff_rsp_message->opcode == G_ONOFF_STATUS)
            {
                if (lenRsp == 7)
                {
                    if (onoff == onoff_rsp_message->data[0])
                        return CODE_OK;
                }
                else
                {
                    if (onoff == onoff_rsp_message->data[1])
                        return CODE_OK;
                }
                LOGW("Onoff resp state not match with input control");
            }
        }
    }
    else
    {
        onoff_message.ble_message_header.devAddr = devAddr;
        onoff_message.opcode = G_ONOFF_SET_NOACK;
        onoff_message.onoff = onoff;
        onoff_message.rev2 = 0;
        onoff_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&onoff_message, sizeof(onoff_message_t), 0, dataRsp, &lenRsp, 1000);
        if (rs == CODE_OK)
        {
            return CODE_OK;
        }
    }
    LOGW("SetOnOff err");
    return CODE_ERROR;
}

int BleProtocol::GetOnOffLight(uint16_t devAddr)
{
    LOGD("Get OnOff addr: 0x%04X", devAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
    } onoff_message_t;
    onoff_message_t onoff_message = {0};
    memset(&onoff_message, 0x00, sizeof(onoff_message));
    uint8_t getOnOffHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x04};
    onoff_message.ble_message_header.devAddr = devAddr;
    onoff_message.opcode = G_ONOFF_GET;
    int rs = SendMessage(APP_REQ, (uint8_t *)&onoff_message, sizeof(onoff_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 800, getOnOffHeader, 0, 6);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint16_t opcode;
            uint8_t data[3];
        } onoff_rsp_message_t;
        onoff_rsp_message_t *onoff_rsp_message = (onoff_rsp_message_t *)dataRsp;
        if (onoff_rsp_message->opcode == G_ONOFF_STATUS)
        {
            return CODE_OK;
        }
    }

    LOGW("GetOnOff err");
    return CODE_ERROR;
}

int BleProtocol::SetDimmingLight(uint16_t devAddr, uint16_t dim, uint16_t transition, bool ack)
{
    LOGD("Dimming addr: 0x%04X value %d", devAddr, dim);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t dim;
        uint8_t offset;
        uint16_t transition;
    } dim_message_t;
    dim_message_t dim_message;
    memset(&dim_message, 0x00, sizeof(dim_message));
    if (ack)
    {
        uint8_t dimmingHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x4E};

        dim_message.ble_message_header.devAddr = devAddr;
        dim_message.opcode = LIGHTNESS_SET;
        dim_message.dim = dim;
        dim_message.offset = 0;
        dim_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&dim_message, sizeof(dim_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, dimmingHeader, 0, 6);
        if (rs == CODE_OK)
        {
            typedef struct __attribute__((packed))
            {
                uint16_t devAddr;
                uint16_t gwAddr;
                uint16_t opcode;
                uint8_t data[8];
            } dim_rsp_message_t;
            dim_rsp_message_t *dim_rsp_message = (dim_rsp_message_t *)dataRsp;
            if (lenRsp == 8)
            {
                if ((dim_rsp_message->data[0] | dim_rsp_message->data[1] << 8) == dim)
                {
                    return CODE_OK;
                }
            }
            else if (lenRsp > 8)
            {
                if ((dim_rsp_message->data[2] | dim_rsp_message->data[3] << 8) == dim)
                {
                    return CODE_OK;
                }
            }
        }
    }
    else
    {
        dim_message.ble_message_header.devAddr = devAddr;
        dim_message.opcode = LIGHTNESS_SET_NOACK;
        dim_message.dim = dim;
        dim_message.offset = 0;
        dim_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&dim_message, sizeof(dim_message_t), 0, dataRsp, &lenRsp, 1000);
        if (rs == CODE_OK)
        {
            return CODE_OK;
        }
    }
    LOGW("Dimming err");
    return CODE_ERROR;
}

int BleProtocol::SetCctLight(uint16_t devAddr, uint16_t cct, uint16_t transition, bool ack)
{
    LOGD("Set Cct addr: 0x%04X value %d", devAddr, cct);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t cct;
        uint8_t offset[3];
        uint16_t transition;
    } cct_message_t;
    cct_message_t cct_message;
    memset(&cct_message, 0x00, sizeof(cct_message));
    if (ack)
    {
        uint8_t cctHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 00, 0x82, 0x66};
        cct_message.ble_message_header.devAddr = devAddr;
        cct_message.opcode = LIGHT_CTL_TEMP_SET;
        cct_message.cct = cct;
        for (int count = 0; count < 3; count++)
        {
            cct_message.offset[count] = 0;
        }
        cct_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&cct_message, sizeof(cct_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, cctHeader, 0, 6);
        if (rs == CODE_OK)
        {
            typedef struct __attribute__((packed))
            {
                uint16_t devAddr;
                uint16_t gwAddr;
                uint16_t opcode;
                uint8_t data[8];
            } cct_rsp_message_t;
            cct_rsp_message_t *cct_rsp_message = (cct_rsp_message_t *)dataRsp;
            if (lenRsp == 10)
            {
                if (cct == (cct_rsp_message->data[0] | (cct_rsp_message->data[1] << 8)))
                {
                    return CODE_OK;
                }
            }
            else if (lenRsp > 10)
            {
                if (cct == (cct_rsp_message->data[4] | (cct_rsp_message->data[5] << 8)))
                {
                    return CODE_OK;
                }
            }
        }
    }
    else if (ack == false)
    {
        cct_message.ble_message_header.devAddr = devAddr;
        cct_message.opcode = LIGHT_CTL_TEMP_SET_NOACK;
        cct_message.cct = cct;
        for (int count = 0; count < 3; count++)
        {
            cct_message.offset[count] = 0;
        }
        cct_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&cct_message, sizeof(cct_message_t), 0, dataRsp, &lenRsp, 1000);
        if (rs == CODE_OK)
        {
            return CODE_OK;
        }
    }
    LOGW("Cct err");
    return CODE_ERROR;
}

int BleProtocol::SetHSLLight(uint16_t devAddr, uint16_t H, uint16_t S, uint16_t L, uint16_t transition, bool ack)
{
    LOGD("HSL addr: 0x%04X value HSL: %d-%d-%d", devAddr, H, S, L);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t l;
        uint16_t h;
        uint16_t s;
        uint8_t offset;
        uint16_t transition;
    } hsl_message_t;
    hsl_message_t hsl_message = {0};
    memset(&hsl_message, 0x00, sizeof(hsl_message));
    if (ack)
    {
        uint8_t hslHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x78};
        hsl_message.ble_message_header.devAddr = devAddr;
        hsl_message.opcode = LIGHT_HSL_SET;
        hsl_message.l = L;
        hsl_message.h = H;
        hsl_message.s = S;
        hsl_message.offset = 0;
        hsl_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&hsl_message, sizeof(hsl_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, hslHeader, 0, 6);
        if (rs == CODE_OK)
        {
            typedef struct __attribute__((packed))
            {
                uint16_t devAddr;
                uint16_t gwAddr;
                uint16_t opcode;
                uint16_t l;
                uint16_t h;
                uint16_t s;
            } hsl_rsp_message_t;
            hsl_rsp_message_t *hsl_rsp_message = (hsl_rsp_message_t *)dataRsp;
            if (hsl_rsp_message->h == H && hsl_rsp_message->l == L && hsl_rsp_message->s == S)
            {
                return CODE_OK;
            }
            LOGW("hsl resp state not match with input control");
        }
    }
    else
    {
        hsl_message.ble_message_header.devAddr = devAddr;
        hsl_message.opcode = LIGHT_HSL_SET_NOACK;
        hsl_message.l = L;
        hsl_message.h = H;
        hsl_message.s = S;
        hsl_message.offset = 0;
        hsl_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&hsl_message, sizeof(hsl_message_t), 0, dataRsp, &lenRsp, 1000);
        if (rs == CODE_OK)
        {
            return CODE_OK;
        }
    }
    LOGW("Set hsl err");
    return CODE_ERROR;
}

int BleProtocol::SetCctDimLight(uint16_t devAddr, uint16_t cct, uint16_t dim, uint16_t transition, bool ack)
{
    LOGD("Set Dim cct addr: 0x%04X", devAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t dim;
        uint16_t cct;
        uint8_t offset;
        uint16_t transition;
    } dimcct_message_t;
    dimcct_message_t dimcct_message = {0};
    memset(&dimcct_message, 0x00, sizeof(dimcct_message));
    if (ack)
    {
        uint8_t dimcctHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x60};
        dimcct_message.ble_message_header.devAddr = devAddr;
        dimcct_message.opcode = LIGHT_CTL_SET;
        dimcct_message.dim = dim;
        dimcct_message.cct = cct;
        dimcct_message.offset = 0;
        dimcct_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&dimcct_message, sizeof(dimcct_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, dimcctHeader, 0, 6);
        if (rs == CODE_OK)
        {
            typedef struct __attribute__((packed))
            {
                uint16_t devAddr;
                uint16_t gwAddr;
                uint16_t opcode;
                uint16_t dim;
                uint16_t cct;
            } dimcct_rsp_message_t;
            dimcct_rsp_message_t *dimcct_rsp_message = (dimcct_rsp_message_t *)dataRsp;
            if (dimcct_rsp_message->dim == dim && dimcct_rsp_message->cct == cct)
            {
                return CODE_OK;
            }
            LOGW("dim cct resp state not match with input control");
        }
    }
    else
    {
        dimcct_message.ble_message_header.devAddr = devAddr;
        dimcct_message.opcode = LIGHT_CTL_SET_NOACK;
        dimcct_message.dim = dim;
        dimcct_message.cct = cct;
        dimcct_message.offset = 0;
        dimcct_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&dimcct_message, sizeof(dimcct_message_t), 0, dataRsp, &lenRsp, 1000);
        if (rs == CODE_OK)
        {
            return CODE_OK;
        }
    }
    LOGW("Set dim cct err");
    return CODE_ERROR;
}

int BleProtocol::SetLevelDim(uint16_t devAddr, uint8_t dimMax, uint8_t dimMin)
{
    LOGD("Set Level Dim addr: 0x%04X, dimMax: %d, dimMin: %d", devAddr, dimMax, dimMin);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint8_t header;
        uint16_t magic;
        uint8_t dimMax;
        uint8_t dimMin;
    } dim_message_t;
    dim_message_t dim_message = {0};
    memset(&dim_message, 0x00, sizeof(dim_message));

    uint8_t dimHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x52};
    dim_message.ble_message_header.devAddr = devAddr;
    dim_message.opcode = LIGHTNESS_LINEAR_SET;
    dim_message.header = 8;
    dim_message.dimMax = dimMax;
    dim_message.dimMin = dimMin;
    int rs = SendMessage(APP_REQ, (uint8_t *)&dim_message, sizeof(dim_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, dimHeader, 0, 6);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint16_t opcode;
            uint8_t header;
            uint16_t magic;
            uint8_t dimMax;
            uint8_t dimMin;
        } dim_rsp_message_t;
        dim_rsp_message_t *dim_rsp_message = (dim_rsp_message_t *)dataRsp;
        if (dim_rsp_message->opcode == LIGHTNESS_LINEAR_STATUS && dim_rsp_message->dimMax == dimMax && dim_rsp_message->dimMin == dimMin)
        {
            return CODE_OK;
        }
        LOGW("dim level resp state not match with input control");
    }
    LOGW("Set dim level err");
    return CODE_ERROR;
}

int BleProtocol::GroupAddDevice(uint16_t devAddr, uint16_t element, uint16_t group)
{
    LOGD("Add dev addr: 0x%04X  with element: 0x%04x to group: 0x%04X", devAddr, element, group);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t addGroupHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x80, 0x1f};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t element;
        uint16_t group;
        uint16_t offset;
    } addgroup_message_t;
    addgroup_message_t addgroup_message = {0};
    memset(&addgroup_message, 0x00, sizeof(addgroup_message));
    addgroup_message.ble_message_header.devAddr = devAddr;
    addgroup_message.opcode = CFG_MODEL_SUB_ADD;
    addgroup_message.element = element;
    addgroup_message.group = group;
    addgroup_message.offset = 0x1000;
    int rs = SendMessage(APP_REQ, (uint8_t *)&addgroup_message, sizeof(addgroup_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 2000, addGroupHeader, 0, 6);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint16_t opcode;
            uint8_t offset;
            uint16_t element;
            uint16_t group;
        } addgroup_rsp_message_t;
        addgroup_rsp_message_t *addgroup_rsp_message = (addgroup_rsp_message_t *)dataRsp;
        if (element == addgroup_rsp_message->element && group == addgroup_rsp_message->group)
        {
            return CODE_OK;
        }
        LOGW("add group resp state not match with input control");
    }
    LOGW("Add group err");
    return CODE_ERROR;
}

int BleProtocol::GroupDelDevice(uint16_t devAddr, uint16_t element, uint16_t group)
{
    LOGD("Del dev addr: 0x%04X  with element: 0x%04x to group: 0x%04X", devAddr, element, group);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t delGroupHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x80, 0x1f};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t element;
        uint16_t group;
        uint16_t offset;
    } delgroup_message_t;
    delgroup_message_t delgroup_message = {0};
    memset(&delgroup_message, 0x00, sizeof(delgroup_message));
    delgroup_message.ble_message_header.devAddr = devAddr;
    delgroup_message.opcode = CFG_MODEL_SUB_DEL;
    delgroup_message.element = element;
    delgroup_message.group = group;
    delgroup_message.offset = 0x1000;
    int rs = SendMessage(APP_REQ, (uint8_t *)&delgroup_message, sizeof(delgroup_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 2000, delGroupHeader, 0, 6);
    if (rs == CODE_OK)
    {
        return CODE_OK;
    }
    LOGW("Del group err");
    return CODE_ERROR;
}

int BleProtocol::SceneAddDevice(uint16_t devAddr, uint16_t scene, uint8_t modeRgb)
{
    LOGD("Set scene addr: 0x%04X to scene: 0x%04X, modergb: %d", devAddr, scene, modeRgb);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t setSceneHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x45};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t scene;
        uint8_t modeRgb;
        uint16_t offset;
    } setscene_message_t;
    setscene_message_t setscene_message = {0};
    memset(&setscene_message, 0x00, sizeof(setscene_message));
    setscene_message.ble_message_header.devAddr = devAddr;
    setscene_message.opcode = SCENE_STORE;
    setscene_message.scene = scene;
    setscene_message.modeRgb = modeRgb;
    int rs = SendMessage(APP_REQ, (uint8_t *)&setscene_message, sizeof(setscene_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, setSceneHeader, 0, 6);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint16_t opcode;
            uint8_t offset;
            uint16_t scene;
        } setscene_rsp_message_t;
        setscene_rsp_message_t *setscene_rsp_message = (setscene_rsp_message_t *)dataRsp;
        if (setscene_rsp_message->scene == scene)
        {
            return CODE_OK;
        }
        else
        {
            LOGW("set scene resp state not match with input control");
        }
    }
    LOGW("Set scene err");
    return CODE_ERROR;
}

// TODO: BelProtocol SceneDelDevice
int BleProtocol::SceneDelDevice(uint16_t devAddr, uint16_t scene)
{
    LOGD("Del scene addr: 0x%04X to scene: 0x%04X", devAddr, scene);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t delSceneHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x45};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t scene;
    } delscene_message_t;
    delscene_message_t delscene_message = {0};
    memset(&delscene_message, 0x00, sizeof(delscene_message));
    delscene_message.ble_message_header.devAddr = devAddr;
    delscene_message.opcode = SCENE_DEL;
    delscene_message.scene = scene;
    int rs = SendMessage(APP_REQ, (uint8_t *)&delscene_message, sizeof(delscene_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, delSceneHeader, 0, 6);
    if (rs == CODE_OK)
    {
        return CODE_OK;
    }
    LOGW("Del scene err");
    return CODE_ERROR;
}

// TODO: BelProtocol ActiveScene
// add delay time
int BleProtocol::SceneRecall(uint16_t devAddr, uint16_t scene, uint16_t transition, bool ack)
{
    LOGD("Call scene: 0x%04X", scene);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t callSceneHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x5e, 0x00};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t scene;
        uint8_t offset;
        uint16_t transition;
    } callscene_message_t;
    callscene_message_t callscene_message = {0};
    memset(&callscene_message, 0x00, sizeof(callscene_message));
    if (ack)
    {
        callscene_message.ble_message_header.devAddr = devAddr;
        callscene_message.opcode = SCENE_RECALL;
        callscene_message.scene = scene;
        callscene_message.offset = 0;
        callscene_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&callscene_message, sizeof(callscene_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, callSceneHeader, 0, 6);
        if (rs == CODE_OK)
        {
            typedef struct __attribute__((packed))
            {
                uint16_t devAddr;
                uint16_t gwAddr;
                uint16_t opcode;
                uint8_t data[8];
            } callscene_rsp_message_t;
            callscene_rsp_message_t *callscene_rsp_message = (callscene_rsp_message_t *)dataRsp;
            if (lenRsp == 11 || lenRsp == 13)
            {
                if (scene == (callscene_rsp_message->data[2] | (callscene_rsp_message->data[3] << 8)))
                {
                    return CODE_OK;
                }
                else
                {
                    LOGW("call scene resp state not match with input control");
                }
            }
            else
            {
                if (scene == (callscene_rsp_message->data[0] | (callscene_rsp_message->data[1] << 8)))
                {
                    return CODE_OK;
                }
                else
                {
                    LOGW("call scene resp state not match with input control");
                }
            }
        }
    }
    else
    {
        callscene_message.ble_message_header.devAddr = devAddr;
        callscene_message.opcode = SCENE_RECALL_NOACK;
        callscene_message.scene = scene;
        callscene_message.offset = 0;
        callscene_message.transition = transition;
        int rs = SendMessage(APP_REQ, (uint8_t *)&callscene_message, sizeof(callscene_message_t), 0, dataRsp, &lenRsp, 400);
        if (rs == CODE_OK)
        {
            return CODE_OK;
        }
    }

    LOGW("Call scene err");
    return CODE_ERROR;
}

int BleProtocol::CallModeRgb(uint16_t devAddr, uint8_t modeRgb)
{
    LOGD("Call modeRgb: %d, addr: 0x%04X ", modeRgb, devAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t modeRgbHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x52};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint16_t header;
        uint8_t mode;
    } modergb_message_t;
    modergb_message_t modergb_message = {0};
    memset(&modergb_message, 0x00, sizeof(modergb_message));
    modergb_message.ble_message_header.devAddr = devAddr;
    modergb_message.opcode = LIGHTNESS_LINEAR_SET;
    modergb_message.header = HEADER_CALLMODE_RGB;
    modergb_message.mode = modeRgb;
    int rs = SendMessage(APP_REQ, (uint8_t *)&modergb_message, sizeof(modergb_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, modeRgbHeader, 0, 6);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint16_t opcode;
            uint16_t header;
            uint8_t mode;
        } modergb_rsp_message_t;
        modergb_rsp_message_t *modergb_rsp_message = (modergb_rsp_message_t *)dataRsp;
        if (modergb_rsp_message->header == HEADER_CALLMODE_RGB)
        {
            if (modergb_rsp_message->mode == modeRgb)
            {
                return CODE_OK;
            }
            LOGW("call mode rgb resp state not match with input control");
        }
    }
    LOGW("call mode rgb err");
    return CODE_ERROR;
}

int BleProtocol::UpdateLights(uint16_t devAddr)
{
    LOGI("Update lights addr: 0x%04X ", devAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t updateHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x52};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint16_t opcode;
        uint8_t header;
        uint8_t data[7];
    } update_message_t;
    update_message_t update_message = {0};
    memset(&update_message, 0x00, sizeof(update_message));
    update_message.ble_message_header.devAddr = devAddr;
    update_message.opcode = LIGHTNESS_LINEAR_SET;
    update_message.header = 0x02;
    int rs = SendMessage(APP_REQ, (uint8_t *)&update_message, sizeof(update_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 800, updateHeader, 0, 6);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint16_t opcode;
            uint8_t header;
        } update_rsp_message_t;
        update_rsp_message_t *update_rsp_message = (update_rsp_message_t *)dataRsp;
        if (update_rsp_message->header == 0x02)
        {
            return CODE_OK;
        }
        LOGW("update lights resp state not match with input control");
    }
    LOGW("update lights err");
    return CODE_ERROR;
}

int BleProtocol::UpdateStatusSensorsPm(uint16_t devAddr)
{
    LOGD("Update status sensor addr: 0x%04X ", devAddr);
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t data[6];
    } update_message_t;
    update_message_t update_message = {0};
    memset(&update_message, 0x00, sizeof(update_message));
    update_message.ble_message_header.devAddr = devAddr;
    update_message.opcodeVendor = RD_OPCODE_CONFIG;
    update_message.vendorId = RD_VENDOR_ID;
    update_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    update_message.header = GET_STATUS_SENSOR_PM;
    int rs = SendMessage(APP_REQ, (uint8_t *)&update_message, sizeof(update_message_t), HCI_GATEWAY_RSP_OP_CODE, 0, 0, 1000);
    if (rs == CODE_OK)
    {
        return CODE_OK;
    }
    LOGW("update status sensor err");
    return CODE_ERROR;
}

int BleProtocol::SetSceneSwitchSceneDC(uint16_t devAddr, uint8_t button, uint8_t mode, uint16_t sceneId, uint8_t type)
{
    LOGD("SetSceneSwitchSceneDC 0x%04x, button %d, mode %d, sceneId %d, type %d", devAddr, button, mode, sceneId, type);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t setSceneDcHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t button;
        uint8_t mode;
        uint16_t sceneId;
        uint8_t type;
        uint8_t future;
    } set_scene_dc_message_t;
    set_scene_dc_message_t set_scene_dc_message = {0};
    memset(&set_scene_dc_message, 0x00, sizeof(set_scene_dc_message));
    set_scene_dc_message.ble_message_header.devAddr = devAddr;
    set_scene_dc_message.opcodeVendor = RD_OPCODE_CONFIG;
    set_scene_dc_message.vendorId = RD_VENDOR_ID;
    set_scene_dc_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    set_scene_dc_message.header = RD_HEADER_CONFIG_SET_SCENE_SWITCH_SCENE_DC;
    set_scene_dc_message.button = button;
    set_scene_dc_message.mode = mode;
    set_scene_dc_message.sceneId = sceneId;
    set_scene_dc_message.type = type;
    int rs = SendMessage(APP_REQ, (uint8_t *)&set_scene_dc_message, sizeof(set_scene_dc_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, setSceneDcHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t button;
            uint8_t mode;
            uint16_t sceneId;
            uint8_t type;
        } set_scene_dc_rsp_message_t;
        set_scene_dc_rsp_message_t *set_scene_dc_rsp_message = (set_scene_dc_rsp_message_t *)dataRsp;
        if (set_scene_dc_rsp_message->header == RD_HEADER_CONFIG_SET_SCENE_SWITCH_SCENE_DC &&
            set_scene_dc_rsp_message->button == button &&
            set_scene_dc_rsp_message->mode == mode &&
            set_scene_dc_rsp_message->sceneId == sceneId &&
            set_scene_dc_rsp_message->type == type)
        {
            return CODE_OK;
        }
        LOGW("set scene dc resp state not match with input control");
    }
    LOGW("set scene dc remote err");
    return CODE_ERROR;
}

int BleProtocol::SetSceneSwitchSceneAC(uint16_t devAddr, uint8_t button, uint8_t mode, uint16_t sceneId, uint8_t type)
{
    LOGD("SetSceneSwitchSceneAC 0x%04x, button %d, mode %d, sceneId %d, type %d", devAddr, button, mode, sceneId, type);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t setSceneAcHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t button;
        uint8_t mode;
        uint16_t sceneId;
        uint8_t type;
        uint8_t future;
    } set_scene_ac_message_t;
    set_scene_ac_message_t set_scene_ac_message = {0};
    memset(&set_scene_ac_message, 0x00, sizeof(set_scene_ac_message));
    set_scene_ac_message.ble_message_header.devAddr = devAddr;
    set_scene_ac_message.opcodeVendor = RD_OPCODE_CONFIG;
    set_scene_ac_message.vendorId = RD_VENDOR_ID;
    set_scene_ac_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    set_scene_ac_message.header = RD_HEADER_CONFIG_SET_SCENE_SWITCH_SCENE_AC;
    set_scene_ac_message.button = button;
    set_scene_ac_message.mode = mode;
    set_scene_ac_message.sceneId = sceneId;
    set_scene_ac_message.type = type;
    int rs = SendMessage(APP_REQ, (uint8_t *)&set_scene_ac_message, sizeof(set_scene_ac_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, setSceneAcHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t button;
            uint8_t mode;
            uint16_t sceneId;
            uint8_t type;
        } set_scene_ac_rsp_message_t;
        set_scene_ac_rsp_message_t *set_scene_ac_rsp_message = (set_scene_ac_rsp_message_t *)dataRsp;
        LOGD("Header: %d, Scene: %d", set_scene_ac_rsp_message->header, sceneId);
        if (set_scene_ac_rsp_message->header == RD_HEADER_CONFIG_SET_SCENE_SWITCH_SCENE_AC &&
            set_scene_ac_rsp_message->button == button &&
            set_scene_ac_rsp_message->mode == mode &&
            set_scene_ac_rsp_message->sceneId == sceneId &&
            set_scene_ac_rsp_message->type == type)
        {
            return CODE_OK;
        }
        LOGW("set scene ac scene resp state not match with input control");
    }
    LOGW("set scene ac remote err");
    return CODE_ERROR;
}

int BleProtocol::DelSceneSwitchSceneDC(uint16_t devAddr, uint8_t button, uint8_t mode)
{
    LOGD("DelSceneSwitchSceneDC 0x%04x, button %d, mode %d", devAddr, button, mode);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t delSceneDcHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t button;
        uint8_t mode;
        uint8_t future[4];
    } del_scene_dc_message_t;
    del_scene_dc_message_t del_scene_dc_message = {0};
    memset(&del_scene_dc_message, 0x00, sizeof(del_scene_dc_message));
    del_scene_dc_message.ble_message_header.devAddr = devAddr;
    del_scene_dc_message.opcodeVendor = RD_OPCODE_CONFIG;
    del_scene_dc_message.vendorId = RD_VENDOR_ID;
    del_scene_dc_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    del_scene_dc_message.header = RD_HEADER_CONFIG_DEL_SCENE_SWITCH_SCENE_DC;
    del_scene_dc_message.button = button;
    del_scene_dc_message.mode = mode;
    int rs = SendMessage(APP_REQ, (uint8_t *)&del_scene_dc_message, sizeof(del_scene_dc_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, delSceneDcHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t button;
            uint8_t mode;
        } del_scene_dc_rsp_message_t;
        del_scene_dc_rsp_message_t *del_scene_dc_rsp_message = (del_scene_dc_rsp_message_t *)dataRsp;
        LOGD("Header: %d", del_scene_dc_rsp_message->header);
        if (del_scene_dc_rsp_message->header == RD_HEADER_CONFIG_DEL_SCENE_SWITCH_SCENE_DC &&
            del_scene_dc_rsp_message->button == button &&
            del_scene_dc_rsp_message->mode == mode)
        {
            return CODE_OK;
        }
        LOGW("del scene dc scene resp state not match with input control");
    }
    LOGW("del scene dc scene err");
    return CODE_ERROR;
}

int BleProtocol::DelSceneSwitchSceneAC(uint16_t devAddr, uint8_t button, uint8_t mode)
{
    LOGD("DelSceneSwitchSceneAC 0x%04x, button %d, mode %d", devAddr, button, mode);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t delSceneAcHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t button;
        uint8_t mode;
        uint8_t future[4];
    } del_scene_ac_message_t;
    del_scene_ac_message_t del_scene_ac_message = {0};
    memset(&del_scene_ac_message, 0x00, sizeof(del_scene_ac_message));
    del_scene_ac_message.ble_message_header.devAddr = devAddr;
    del_scene_ac_message.opcodeVendor = RD_OPCODE_CONFIG;
    del_scene_ac_message.vendorId = RD_VENDOR_ID;
    del_scene_ac_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    del_scene_ac_message.header = RD_HEADER_CONFIG_DEL_SCENE_SWITCH_SCENE_AC;
    del_scene_ac_message.button = button;
    del_scene_ac_message.mode = mode;
    int rs = SendMessage(APP_REQ, (uint8_t *)&del_scene_ac_message, sizeof(del_scene_ac_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, delSceneAcHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t button;
            uint8_t mode;
        } del_scene_ac_rsp_message_t;
        del_scene_ac_rsp_message_t *del_scene_ac_rsp_message = (del_scene_ac_rsp_message_t *)dataRsp;
        LOGD("Header: %d", del_scene_ac_rsp_message->header);
        if (del_scene_ac_rsp_message->header == RD_HEADER_CONFIG_DEL_SCENE_SWITCH_SCENE_AC &&
            del_scene_ac_rsp_message->button == button &&
            del_scene_ac_rsp_message->mode == mode)
        {
            return CODE_OK;
        }
        LOGW("del scene ac scene resp state not match with input control");
    }
    LOGW("del scene ac scene err");
    return CODE_ERROR;
}

int BleProtocol::SetScenePirLightSensor(uint16_t devAddr, uint8_t condition, uint8_t pir, uint16_t lowLux, uint16_t highLux, uint16_t scene, uint8_t type)
{
    LOGD("SetScenePirLightSensor condition: %d, pir: %d, lowLux: %d, highLux: %d", condition, pir, lowLux, highLux);
    typedef struct __attribute__((packed))
    {
        union
        {
            uint32_t data;
            struct
            {
                uint32_t store : 8;          // 8 bit not use
                uint32_t Lux_hi : 10;        // 10 bit lux hi
                uint32_t Lux_low : 10;       // 10 bit lux low
                uint32_t Light_Conditon : 3; // 7 bit low
                uint32_t Pir_Conditon : 1;   // 1 bit hight
            };
        };
    } RD_Sensor_data_tdef;

    RD_Sensor_data_tdef data_scene_pir_light;
    data_scene_pir_light.Pir_Conditon = (uint32_t)pir;
    data_scene_pir_light.Light_Conditon = (uint32_t)(condition & 0x0000007);
    data_scene_pir_light.Lux_low = (uint32_t)(lowLux);
    data_scene_pir_light.Lux_hi = (uint32_t)(highLux);

    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t sceneLightPirHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t sceneId;
        uint8_t infoScene[3];
        uint8_t type;
    } scene_light_pir_message_t;
    scene_light_pir_message_t scene_light_pir_message = {0};
    memset(&scene_light_pir_message, 0x00, sizeof(scene_light_pir_message));
    scene_light_pir_message.ble_message_header.devAddr = devAddr;
    scene_light_pir_message.opcodeVendor = RD_OPCODE_CONFIG;
    scene_light_pir_message.vendorId = RD_VENDOR_ID;
    scene_light_pir_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    scene_light_pir_message.header = RD_HEADER_CONFIG_SET_SCENE_PIR_LIGHT_SENSOR;
    scene_light_pir_message.sceneId = scene;
    scene_light_pir_message.infoScene[0] = (data_scene_pir_light.data >> 24) & 0xFF;
    scene_light_pir_message.infoScene[1] = (data_scene_pir_light.data >> 16) & 0xFF;
    scene_light_pir_message.infoScene[2] = (data_scene_pir_light.data >> 8) & 0xFF;
    int rs = SendMessage(APP_REQ, (uint8_t *)&scene_light_pir_message, sizeof(scene_light_pir_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 2000, sceneLightPirHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t scene;
        } scene_light_pir_rsp_message_t;
        scene_light_pir_rsp_message_t *scene_light_pir_rsp_message = (scene_light_pir_rsp_message_t *)&dataRsp;
        if (scene_light_pir_rsp_message->header == RD_HEADER_CONFIG_SET_SCENE_PIR_LIGHT_SENSOR && scene_light_pir_rsp_message->scene == scene)
        {
            return CODE_OK;
        }
        else
        {
            LOGW("Scene light pir rsp not match control");
        }
    }
    else
    {
        LOGW("Scene light pir error");
    }
    return CODE_ERROR;
}

int BleProtocol::DelScenePirLightSensor(uint16_t devAddr, uint16_t scene)
{
    LOGD("Del scene light pir");
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t sceneLightPirHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t sceneId;
    } scene_light_pir_message_t;
    scene_light_pir_message_t scene_light_pir_message = {0};
    memset(&scene_light_pir_message, 0x00, sizeof(scene_light_pir_message));
    scene_light_pir_message.ble_message_header.devAddr = devAddr;
    scene_light_pir_message.opcodeVendor = RD_OPCODE_CONFIG;
    scene_light_pir_message.vendorId = RD_VENDOR_ID;
    scene_light_pir_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    scene_light_pir_message.header = RD_HEADER_CONFIG_DEL_SCENE_PIR_LIGHT_SENSOR;
    scene_light_pir_message.sceneId = scene;
    int rs = SendMessage(APP_REQ, (uint8_t *)&scene_light_pir_message, sizeof(scene_light_pir_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 2000, sceneLightPirHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t scene;
        } scene_light_pir_rsp_message_t;
        scene_light_pir_rsp_message_t *scene_light_pir_rsp_message = (scene_light_pir_rsp_message_t *)&dataRsp;
        if (scene_light_pir_rsp_message->header == RD_HEADER_CONFIG_DEL_SCENE_PIR_LIGHT_SENSOR && scene_light_pir_rsp_message->scene == scene)
        {
            return CODE_OK;
        }
        else
        {
            LOGW("Del Scene light pir rsp not match control");
        }
    }
    else
    {
        LOGW("Del Scene light pir error");
    }
    return CODE_ERROR;
}

int BleProtocol::TimeActionPirLightSensor(uint16_t devAddr, uint16_t time)
{
    LOGD("TimeActionPirLightSensor 0x%04X", devAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t timeActionHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t time;
    } time_action_message_t;
    time_action_message_t time_action_message = {0};
    memset(&time_action_message, 0x00, sizeof(time_action_message));
    time_action_message.ble_message_header.devAddr = devAddr;
    time_action_message.opcodeVendor = RD_OPCODE_CONFIG;
    time_action_message.vendorId = RD_VENDOR_ID;
    time_action_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    time_action_message.header = RD_HEADER_CONFIG_SET_TIME_ACTION_PIR_LIGHT_SENSOR;
    time_action_message.time = time;
    int rs = SendMessage(APP_REQ, (uint8_t *)&time_action_message, sizeof(time_action_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, timeActionHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t time;
        } time_action_rsp_message_t;
        time_action_rsp_message_t *time_action_rsp_message = (time_action_rsp_message_t *)dataRsp;
        if (time_action_rsp_message->header == RD_HEADER_CONFIG_SET_TIME_ACTION_PIR_LIGHT_SENSOR && time_action_rsp_message->time == time)
        {
            return CODE_OK;
        }
        LOGW("time action pir light resp state not match with input control");
    }
    LOGW("time action pir light err");
    return CODE_ERROR;
}

int BleProtocol::SetModeActionPirLightSensor(uint16_t devAddr, uint8_t mode)
{
    LOGD("ModeActionPirLightSensor 0x%04X", devAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t modeActionHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t mode;
        uint8_t future[5];
    } mode_action_message_t;
    mode_action_message_t mode_action_message = {0};
    memset(&mode_action_message, 0x00, sizeof(mode_action_message));
    mode_action_message.ble_message_header.devAddr = devAddr;
    mode_action_message.opcodeVendor = RD_OPCODE_CONFIG;
    mode_action_message.vendorId = RD_VENDOR_ID;
    mode_action_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    mode_action_message.header = RD_HEADER_CONFIG_SET_MODE_ACTION_PIR_LIGHT_SENSOR;
    mode_action_message.mode = mode;
    int rs = SendMessage(APP_REQ, (uint8_t *)&mode_action_message, sizeof(mode_action_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, modeActionHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t mode;
        } mode_action_rsp_message_t;
        mode_action_rsp_message_t *mode_action_rsp_message = (mode_action_rsp_message_t *)dataRsp;
        if (mode_action_rsp_message->header == RD_HEADER_CONFIG_SET_MODE_ACTION_PIR_LIGHT_SENSOR && mode_action_rsp_message->mode == mode)
        {
            return CODE_OK;
        }
        LOGW("mode action pir light resp state not match with input control");
    }
    LOGW("mode action pir light err");
    return CODE_ERROR;
}

int BleProtocol::SetSensiPirLightSensor(uint16_t devAddr, uint8_t sensi)
{
    LOGD("Set sensiPirLightSensor: 0x%04X, sensi: %d", devAddr, sensi);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t sensiHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t sensi;
    } sensi_message_t;
    sensi_message_t sensi_message = {0};
    memset(&sensi_message, 0x00, sizeof(sensi_message));
    sensi_message.ble_message_header.devAddr = devAddr;
    sensi_message.opcodeVendor = RD_OPCODE_CONFIG;
    sensi_message.vendorId = RD_VENDOR_ID;
    sensi_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    sensi_message.header = RD_HEADER_CONFIG_SET_SENSI_PIR_LIGHT_SENSOR;
    sensi_message.sensi = sensi;
    int rs = SendMessage(APP_REQ, (uint8_t *)&sensi_message, sizeof(sensi_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, sensiHeader, 0, 7);
    if (rs == CODE_OK)
    {
        return CODE_OK;
    }
    else
        LOGW("Set sensi error");
    return CODE_ERROR;
}

int BleProtocol::SetDistanceSensor(uint16_t devAddr, uint8_t distance)
{
    LOGD("Set distance sensor: 0x%04X, distance: %d", devAddr, distance);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t distanceHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t distance;
    } distance_message_t;
    distance_message_t distance_message = {0};
    memset(&distance_message, 0x00, sizeof(distance_message));
    distance_message.ble_message_header.devAddr = devAddr;
    distance_message.opcodeVendor = RD_OPCODE_CONFIG;
    distance_message.vendorId = RD_VENDOR_ID;
    distance_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    distance_message.header = RD_HEADER_CONFIG_SET_DISTANCE_RADA_SENSOR;
    distance_message.distance = distance;
    int rs = SendMessage(APP_REQ, (uint8_t *)&distance_message, sizeof(distance_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, distanceHeader, 0, 7);
    if (rs == CODE_OK)
    {
        return CODE_OK;
    }
    else
        LOGW("Set distance error");
    return CODE_ERROR;
}

int BleProtocol::SetTimeRspSensor(uint16_t devAddr, uint16_t time)
{
    LOGD("Set time rsp sensor: 0x%04X, time: %d", devAddr, time);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t timeRspHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t time;
    } timeRsp_message_t;
    timeRsp_message_t timeRsp_message = {0};
    memset(&timeRsp_message, 0x00, sizeof(timeRsp_message));
    timeRsp_message.ble_message_header.devAddr = devAddr;
    timeRsp_message.opcodeVendor = RD_OPCODE_CONFIG;
    timeRsp_message.vendorId = RD_VENDOR_ID;
    timeRsp_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    timeRsp_message.header = RD_HEADER_CONFIG_TIME_RSP_SENSOR;
    timeRsp_message.time = time;
    int rs = SendMessage(APP_REQ, (uint8_t *)&timeRsp_message, sizeof(timeRsp_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, timeRspHeader, 0, 7);
    if (rs == CODE_OK)
    {
        return CODE_OK;
    }
    else
        LOGW("Set time rsp error");
    return CODE_ERROR;
}

int BleProtocol::SceneForScreenTouch(uint16_t devAddr, uint16_t scene, uint8_t icon, uint8_t type)
{
    LOGD("SceneForScreenTouch 0x%04X, scene %d, icon %d", devAddr, scene, icon);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t sceneScreenTouchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t sceneId;
        uint8_t icon;
        uint8_t type;
    } scene_screen_touch_message_t;
    scene_screen_touch_message_t scene_screen_touch_message = {0};
    memset(&scene_screen_touch_message, 0x00, sizeof(scene_screen_touch_message));
    scene_screen_touch_message.ble_message_header.devAddr = devAddr;
    scene_screen_touch_message.opcodeVendor = RD_OPCODE_CONFIG;
    scene_screen_touch_message.vendorId = RD_VENDOR_ID;
    scene_screen_touch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    scene_screen_touch_message.header = RD_HEADER_CONFIG_SET_SCENE_SCREEN_TOUCH;
    scene_screen_touch_message.sceneId = scene;
    scene_screen_touch_message.icon = icon;
    scene_screen_touch_message.type = type;
    int rs = SendMessage(APP_REQ, (uint8_t *)&scene_screen_touch_message, sizeof(scene_screen_touch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, sceneScreenTouchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t scene;
            uint8_t icon;
        } scene_screen_touch_rsp_message_t;
        scene_screen_touch_rsp_message_t *scene_screen_touch_rsp_message = (scene_screen_touch_rsp_message_t *)dataRsp;
        if (scene_screen_touch_rsp_message->header == RD_HEADER_CONFIG_SET_SCENE_SCREEN_TOUCH &&
            scene_screen_touch_rsp_message->scene == scene &&
            scene_screen_touch_rsp_message->icon == icon)
        {
            return CODE_OK;
        }
        LOGW("scene screen touch resp state not match with input control");
    }
    LOGW("scene screen touch err");
    return CODE_ERROR;
}

int BleProtocol::EditIconScreenTouch(uint16_t devAddr, uint16_t scene, uint8_t icon)
{
    LOGD("EditIconScreenTouch 0x%04x, scene %d, icon %d", devAddr, scene, icon);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t editIconScreenTouchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t sceneId;
        uint8_t icon;
    } edit_icon_screen_touch_message_t;
    edit_icon_screen_touch_message_t edit_icon_screen_touch_message = {0};
    memset(&edit_icon_screen_touch_message, 0x00, sizeof(edit_icon_screen_touch_message));
    edit_icon_screen_touch_message.ble_message_header.devAddr = devAddr;
    edit_icon_screen_touch_message.opcodeVendor = RD_OPCODE_CONFIG;
    edit_icon_screen_touch_message.vendorId = RD_VENDOR_ID;
    edit_icon_screen_touch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    edit_icon_screen_touch_message.header = RD_HEADER_CONFIG_EDIT_ICON_SCREEN_TOUCH;
    edit_icon_screen_touch_message.sceneId = scene;
    edit_icon_screen_touch_message.icon = icon;
    int rs = SendMessage(APP_REQ, (uint8_t *)&edit_icon_screen_touch_message, sizeof(edit_icon_screen_touch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, editIconScreenTouchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t scene;
            uint8_t icon;
        } scene_screen_touch_rsp_message_t;
        scene_screen_touch_rsp_message_t *scene_screen_touch_rsp_message = (scene_screen_touch_rsp_message_t *)dataRsp;
        if (scene_screen_touch_rsp_message->header == RD_HEADER_CONFIG_EDIT_ICON_SCREEN_TOUCH &&
            scene_screen_touch_rsp_message->scene == scene &&
            scene_screen_touch_rsp_message->icon == icon)
        {
            return CODE_OK;
        }
        LOGW("edit icon screen touch resp state not match with input control");
    }
    LOGW("edit icon screen touch err");
    return CODE_ERROR;
}

int BleProtocol::DelSceneScreenTouch(uint16_t devAddr, uint16_t scene)
{
    LOGD("DelSceneScreenTouch 0x%04x, scene %d", devAddr, scene);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t delSceneScreenTouchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t sceneId;
    } del_scene_screen_touch_message_t;
    del_scene_screen_touch_message_t del_scene_screen_touch_message = {0};
    memset(&del_scene_screen_touch_message, 0x00, sizeof(del_scene_screen_touch_message));
    del_scene_screen_touch_message.ble_message_header.devAddr = devAddr;
    del_scene_screen_touch_message.opcodeVendor = RD_OPCODE_CONFIG;
    del_scene_screen_touch_message.vendorId = RD_VENDOR_ID;
    del_scene_screen_touch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    del_scene_screen_touch_message.header = RD_HEADER_CONFIG_DEL_SCENE_SCREEN_TOUCH;
    del_scene_screen_touch_message.sceneId = scene;
    int rs = SendMessage(APP_REQ, (uint8_t *)&del_scene_screen_touch_message, sizeof(del_scene_screen_touch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, delSceneScreenTouchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t scene;
        } scene_screen_touch_rsp_message_t;
        scene_screen_touch_rsp_message_t *scene_screen_touch_rsp_message = (scene_screen_touch_rsp_message_t *)dataRsp;
        if (scene_screen_touch_rsp_message->header == RD_HEADER_CONFIG_DEL_SCENE_SCREEN_TOUCH && scene_screen_touch_rsp_message->scene == scene)
        {
            return CODE_OK;
        }
        LOGW("del scene screen touch resp state not match with input control");
    }
    LOGW("del scene screen touch err");
    return CODE_ERROR;
}

int BleProtocol::DelAllScene(uint16_t devAddr)
{
    LOGD("DelAllScene 0x%04x", devAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t delAllSceneScreenTouchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t check[6];
    } del_allscene_screen_touch_message_t;
    del_allscene_screen_touch_message_t del_allscene_screen_touch_message = {0};
    memset(&del_allscene_screen_touch_message, 0x00, sizeof(del_allscene_screen_touch_message));
    del_allscene_screen_touch_message.ble_message_header.devAddr = devAddr;
    del_allscene_screen_touch_message.opcodeVendor = RD_OPCODE_CONFIG;
    del_allscene_screen_touch_message.vendorId = RD_VENDOR_ID;
    del_allscene_screen_touch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    del_allscene_screen_touch_message.header = RD_HEADER_CONFIG_DEL_ALL_SCENE;
    for (int i = 0; i < 6; i++)
    {
        del_allscene_screen_touch_message.check[i] = i;
    }
    int rs = SendMessage(APP_REQ, (uint8_t *)&del_allscene_screen_touch_message, sizeof(del_allscene_screen_touch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, delAllSceneScreenTouchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
        } scene_screen_touch_rsp_message_t;
        scene_screen_touch_rsp_message_t *scene_screen_touch_rsp_message = (scene_screen_touch_rsp_message_t *)dataRsp;
        if (scene_screen_touch_rsp_message->header == RD_HEADER_CONFIG_DEL_ALL_SCENE)
        {
            return CODE_OK;
        }
        LOGW("del all scene screen touch resp state not match with input control");
    }
    LOGW("del all scene screen touch err");
    return CODE_ERROR;
}

int BleProtocol::SendWeatherOutdoor(uint16_t devAddr, uint8_t status, uint16_t temp)
{
    LOGD("SendWeatherOutdoor 0x%04x, status %d, temp %d", devAddr, status, temp);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t weatherOutdoorScreenTouchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t temp;
        uint8_t status;
    } weather_outdoor_screen_touch_message_t;
    weather_outdoor_screen_touch_message_t weather_outdoor_screen_touch_message = {0};
    memset(&weather_outdoor_screen_touch_message, 0x00, sizeof(weather_outdoor_screen_touch_message));
    weather_outdoor_screen_touch_message.ble_message_header.devAddr = devAddr;
    weather_outdoor_screen_touch_message.opcodeVendor = RD_OPCODE_CONFIG;
    weather_outdoor_screen_touch_message.vendorId = RD_VENDOR_ID;
    weather_outdoor_screen_touch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    weather_outdoor_screen_touch_message.header = RD_HEADER_CONFIG_SEND_WEATHER_OUTDOOR;
    weather_outdoor_screen_touch_message.temp = bswap_16(temp);
    weather_outdoor_screen_touch_message.status = status;
    int rs = SendMessage(APP_REQ, (uint8_t *)&weather_outdoor_screen_touch_message, sizeof(weather_outdoor_screen_touch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1500, weatherOutdoorScreenTouchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t temp;
            uint8_t status;
        } weather_outdoor_screen_touch_rsp_message_t;
        weather_outdoor_screen_touch_rsp_message_t *weather_outdoor_screen_touch_rsp_message = (weather_outdoor_screen_touch_rsp_message_t *)dataRsp;
        if (weather_outdoor_screen_touch_rsp_message->header == RD_HEADER_CONFIG_SEND_WEATHER_OUTDOOR &&
            weather_outdoor_screen_touch_rsp_message->temp == temp &&
            weather_outdoor_screen_touch_rsp_message->status == status)
        {
            return CODE_OK;
        }
        LOGW("weather outdoor screen touch resp state not match with input control");
    }
    LOGW("weather outdoor screen touch err");
    return CODE_ERROR;
}

int BleProtocol::SendWeatherIndoor(uint16_t devAddr, uint16_t temp, uint16_t hum, uint16_t pm25)
{
    LOGD("SendWeatherIndoor 0x%04x, temp %d, hum %d", devAddr, temp, hum);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t weatherIndoorScreenTouchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t temp;
        uint16_t hum;
        uint16_t pm25;
    } weather_indoor_screen_touch_message_t;
    weather_indoor_screen_touch_message_t weather_indoor_screen_touch_message = {0};
    memset(&weather_indoor_screen_touch_message, 0x00, sizeof(weather_indoor_screen_touch_message));
    weather_indoor_screen_touch_message.ble_message_header.devAddr = devAddr;
    weather_indoor_screen_touch_message.opcodeVendor = RD_OPCODE_CONFIG;
    weather_indoor_screen_touch_message.vendorId = RD_VENDOR_ID;
    weather_indoor_screen_touch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    weather_indoor_screen_touch_message.header = RD_HEADER_CONFIG_SEND_WEATHER_INDOOR;
    weather_indoor_screen_touch_message.temp = bswap_16(temp);
    weather_indoor_screen_touch_message.hum = bswap_16(hum);
    weather_indoor_screen_touch_message.pm25 = pm25;
    int rs = SendMessage(APP_REQ, (uint8_t *)&weather_indoor_screen_touch_message, sizeof(weather_indoor_screen_touch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1500, weatherIndoorScreenTouchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t temp;
            uint16_t hum;
            uint16_t pm25;
        } weather_outdoor_screen_touch_rsp_message_t;
        weather_outdoor_screen_touch_rsp_message_t *weather_outdoor_screen_touch_rsp_message = (weather_outdoor_screen_touch_rsp_message_t *)dataRsp;
        if (weather_outdoor_screen_touch_rsp_message->header == RD_HEADER_CONFIG_SEND_WEATHER_INDOOR &&
            weather_outdoor_screen_touch_rsp_message->temp == temp &&
            weather_outdoor_screen_touch_rsp_message->hum == hum)
        {
            return CODE_OK;
        }
        LOGW("weather indoor screen touch resp state not match with input control");
    }
    LOGW("weather indoor screen touch err");
    return CODE_ERROR;
}

int BleProtocol::SendDate(uint16_t devAddr, uint16_t years, uint8_t month, uint8_t date, uint8_t day)
{
    LOGD("SendDate 0x%04x, years %d, month %d, date %d, day %d", devAddr, years, month, date, day);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t dateScreenTouchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t years;
        uint8_t month;
        uint8_t date;
        uint8_t day;
    } date_screen_touch_message_t;
    date_screen_touch_message_t date_screen_touch_message = {0};
    memset(&date_screen_touch_message, 0x00, sizeof(date_screen_touch_message));
    date_screen_touch_message.ble_message_header.devAddr = devAddr;
    date_screen_touch_message.opcodeVendor = RD_OPCODE_CONFIG;
    date_screen_touch_message.vendorId = RD_VENDOR_ID;
    date_screen_touch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    date_screen_touch_message.header = RD_HEADER_CONFIG_SEND_DATE;
    date_screen_touch_message.years = bswap_16(years);
    date_screen_touch_message.month = month;
    date_screen_touch_message.date = date;
    date_screen_touch_message.day = day;
    int rs = SendMessage(APP_REQ, (uint8_t *)&date_screen_touch_message, sizeof(date_screen_touch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1500, dateScreenTouchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t years;
            uint8_t month;
            uint8_t date;
            uint8_t day;
        } date_screen_touch_rsp_message_t;
        date_screen_touch_rsp_message_t *date_screen_touch_rsp_message = (date_screen_touch_rsp_message_t *)dataRsp;
        if (date_screen_touch_rsp_message->header == RD_HEADER_CONFIG_SEND_DATE &&
            date_screen_touch_rsp_message->years == bswap_16(years) &&
            date_screen_touch_rsp_message->month == month &&
            date_screen_touch_rsp_message->month == month &&
            date_screen_touch_rsp_message->date == date &&
            date_screen_touch_rsp_message->day == day)
        {
            return CODE_OK;
        }
        LOGW("date screen touch resp state not match with input control");
    }
    LOGW("date screen touch err");
    return CODE_ERROR;
}
int BleProtocol::SendTime(uint16_t devAddr, uint8_t hours, uint8_t minute, uint8_t second)
{
    LOGD("SendTime 0x%04x, hours %d, minute %d, second %d", devAddr, hours, minute, second);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t timeScreenTouchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t hours;
        uint8_t minute;
        uint8_t second;
    } time_screen_touch_message_t;
    time_screen_touch_message_t time_screen_touch_message = {0};
    memset(&time_screen_touch_message, 0x00, sizeof(time_screen_touch_message));
    time_screen_touch_message.ble_message_header.devAddr = devAddr;
    time_screen_touch_message.opcodeVendor = RD_OPCODE_CONFIG;
    time_screen_touch_message.vendorId = RD_VENDOR_ID;
    time_screen_touch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    time_screen_touch_message.header = RD_HEADER_CONFIG_SEND_TIME;
    time_screen_touch_message.hours = hours;
    time_screen_touch_message.minute = minute;
    time_screen_touch_message.second = second;
    int rs = SendMessage(APP_REQ, (uint8_t *)&time_screen_touch_message, sizeof(time_screen_touch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1500, timeScreenTouchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t hours;
            uint8_t minute;
            uint8_t second;
        } date_screen_touch_rsp_message_t;
        date_screen_touch_rsp_message_t *date_screen_touch_rsp_message = (date_screen_touch_rsp_message_t *)dataRsp;
        if (date_screen_touch_rsp_message->header == RD_HEADER_CONFIG_SEND_TIME &&
            date_screen_touch_rsp_message->hours == hours &&
            date_screen_touch_rsp_message->minute == minute &&
            date_screen_touch_rsp_message->second == second)
        {
            return CODE_OK;
        }
        LOGW("time screen touch resp state not match with input control");
    }
    LOGW("time screen touch err");
    return CODE_ERROR;
}
int BleProtocol::SetGroup(uint16_t devAddr, uint16_t group)
{
    LOGD("SetGroup 0x%04x, group %d", devAddr, group);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t groupScreenTouchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t group;
    } group_screen_touch_message_t;
    group_screen_touch_message_t group_screen_touch_message = {0};
    memset(&group_screen_touch_message, 0x00, sizeof(group_screen_touch_message));
    group_screen_touch_message.ble_message_header.devAddr = devAddr;
    group_screen_touch_message.opcodeVendor = RD_OPCODE_CONFIG;
    group_screen_touch_message.vendorId = RD_VENDOR_ID;
    group_screen_touch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    group_screen_touch_message.header = RD_HEADER_CONFIG_SET_GROUP;
    group_screen_touch_message.group = bswap_16(group);
    int rs = SendMessage(APP_REQ, (uint8_t *)&group_screen_touch_message, sizeof(group_screen_touch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, groupScreenTouchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t group;
        } group_screen_touch_rsp_message_t;
        group_screen_touch_rsp_message_t *group_screen_touch_rsp_message = (group_screen_touch_rsp_message_t *)dataRsp;
        if (group_screen_touch_rsp_message->header == RD_HEADER_CONFIG_SET_GROUP && group_screen_touch_rsp_message->group == bswap_16(group))
        {
            return CODE_OK;
        }
        LOGW("group screen touch resp state not match with input control");
    }
    LOGW("group screen touch err");
    return CODE_ERROR;
}

int BleProtocol::ControlOpenClosePausePercent(uint16_t devAddr, uint8_t type, uint8_t percent)
{
    LOGD("ControlOpenClosePausePercent 0x%04x, type %d, percent %d", devAddr, type, percent);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t controlHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t type;
        uint8_t percent;
        uint8_t future[4];
    } control_message_t;
    control_message_t control_message = {0};
    memset(&control_message, 0x00, sizeof(control_message));
    control_message.ble_message_header.devAddr = devAddr;
    control_message.opcodeVendor = RD_OPCODE_CONFIG;
    control_message.vendorId = RD_VENDOR_ID;
    control_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    control_message.header = RD_HEADER_CONTROL_OPEN_CLOSE_PAUSE;
    control_message.type = type;
    control_message.percent = percent;
    int rs = SendMessage(APP_REQ, (uint8_t *)&control_message, sizeof(control_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, controlHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t type;
            uint8_t percent;
        } control_rsp_message_t;
        control_rsp_message_t *control_rsp_message = (control_rsp_message_t *)dataRsp;
        if (control_rsp_message->header == RD_HEADER_RSP_CONTROL_OPEN_CLOSE_PAUSE_OPENED && control_rsp_message->type == type)
        {
            if (type == PERCENT)
            {
                if (control_rsp_message->percent == percent)
                {
                    return CODE_OK;
                }
                LOGW("control resp opened error");
                return CODE_ERROR;
            }
            else
            {
                return CODE_OK;
            }
        }
        LOGW("control resp state not match with input control");
    }
    LOGW("control err");
    return CODE_ERROR;
}
int BleProtocol::ConfigMotor(uint16_t devAddr, uint8_t typeMotor)
{
    LOGD("ConfigMotor 0x%04x, typeMotor %d", devAddr, typeMotor);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t configHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t type;
        uint8_t future[5];
    } config_message_t;
    config_message_t config_message = {0};
    memset(&config_message, 0x00, sizeof(config_message));
    config_message.ble_message_header.devAddr = devAddr;
    config_message.opcodeVendor = RD_OPCODE_CONFIG;
    config_message.vendorId = RD_VENDOR_ID;
    config_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    config_message.header = RD_HEADER_CONFIG_MOTOR;
    config_message.type = typeMotor;
    int rs = SendMessage(APP_REQ, (uint8_t *)&config_message, sizeof(config_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, configHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t type;
        } config_rsp_message_t;
        config_rsp_message_t *config_rsp_message = (config_rsp_message_t *)dataRsp;
        if (config_rsp_message->header == RD_HEADER_CONFIG_MOTOR && config_rsp_message->type == typeMotor)
        {
            return CODE_OK;
        }
        LOGW("config resp state not match with input control");
    }
    LOGW("config err");
    return CODE_ERROR;
}
int BleProtocol::CalibCurtain(uint16_t devAddr, uint8_t status)
{
    LOGD("CalibCurtain 0x%04x, status %d", devAddr, status);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t calibHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t status;
        uint8_t future[5];
    } calib_message_t;
    calib_message_t calib_message = {0};
    memset(&calib_message, 0x00, sizeof(calib_message));
    calib_message.ble_message_header.devAddr = devAddr;
    calib_message.opcodeVendor = RD_OPCODE_CONFIG;
    calib_message.vendorId = RD_VENDOR_ID;
    calib_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    calib_message.header = RD_HEADER_CALIB;
    calib_message.status = status;
    int rs = SendMessage(APP_REQ, (uint8_t *)&calib_message, sizeof(calib_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, calibHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t status;
        } calib_rsp_message_t;
        calib_rsp_message_t *calib_rsp_message = (calib_rsp_message_t *)dataRsp;
        if (calib_rsp_message->header == RD_HEADER_CALIB && calib_rsp_message->status == status)
        {
            return CODE_OK;
        }
        LOGW("calib resp state not match with input control");
    }
    LOGW("calib err");
    return CODE_ERROR;
}

int BleProtocol::CalibAuto(uint16_t devAddr, uint16_t time)
{
    LOGD("CalibAuto 0x%04x, time %d", devAddr, time);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t calibHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t time;
        uint8_t future[4];
    } calib_message_t;
    calib_message_t calib_message = {0};
    memset(&calib_message, 0x00, sizeof(calib_message));
    calib_message.ble_message_header.devAddr = devAddr;
    calib_message.opcodeVendor = RD_OPCODE_CONFIG;
    calib_message.vendorId = RD_VENDOR_ID;
    calib_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    calib_message.header = RD_HEADER_CALIBAUTO;
    calib_message.time = time;
    int rs = SendMessage(APP_REQ, (uint8_t *)&calib_message, sizeof(calib_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, calibHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t time;
        } calib_rsp_message_t;
        calib_rsp_message_t *calib_rsp_message = (calib_rsp_message_t *)dataRsp;
        if (calib_rsp_message->header == RD_HEADER_CALIBAUTO && calib_rsp_message->time == time)
        {
            return CODE_OK;
        }
        LOGW("calib auto resp state not match with input control");
    }
    LOGW("calib auto err");
    return CODE_ERROR;
}

int BleProtocol::LockDevice(uint16_t devAddr, uint8_t locked)
{
    LOGD("LockDevice 0x%04x, locked %d", devAddr, locked);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t lockHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t lock;
        uint8_t future[5];
    } lock_message_t;
    lock_message_t lock_message = {0};
    memset(&lock_message, 0x00, sizeof(lock_message));
    lock_message.ble_message_header.devAddr = devAddr;
    lock_message.opcodeVendor = RD_OPCODE_CONFIG;
    lock_message.vendorId = RD_VENDOR_ID;
    lock_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    lock_message.header = RD_HEADER_LOCK;
    lock_message.lock = locked;
    int rs = SendMessage(APP_REQ, (uint8_t *)&lock_message, sizeof(lock_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, lockHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t lock;
        } lock_rsp_message_t;
        lock_rsp_message_t *lock_rsp_message = (lock_rsp_message_t *)dataRsp;
        if (lock_rsp_message->header == RD_HEADER_LOCK && lock_rsp_message->lock == locked)
        {
            return CODE_OK;
        }
        LOGW("lock device resp state not match with input control");
    }
    LOGW("lock device auto err");
    return CODE_ERROR;
}

int BleProtocol::SetModeWifi(uint16_t devAddr, uint8_t mode)
{
    LOGD("SetModeWifi 0x%04x, mode %d", devAddr, mode);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t modeHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t mode;
        uint8_t future[5];
    } mode_message_t;
    mode_message_t mode_message = {0};
    memset(&mode_message, 0x00, sizeof(mode_message));
    mode_message.ble_message_header.devAddr = devAddr;
    mode_message.opcodeVendor = RD_OPCODE_CONFIG;
    mode_message.vendorId = RD_VENDOR_ID;
    mode_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    mode_message.header = RD_HEADER_MODE_WIFI;
    mode_message.mode = mode;
    int rs = SendMessage(APP_REQ, (uint8_t *)&mode_message, sizeof(mode_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, modeHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t mode;
        } mode_rsp_message_t;
        mode_rsp_message_t *mode_rsp_message = (mode_rsp_message_t *)dataRsp;
        if (mode_rsp_message->header == RD_HEADER_MODE_WIFI && mode_rsp_message->mode == mode)
        {
            return CODE_OK;
        }
        LOGW("mode resp state not match with input control");
    }
    LOGW("mode device auto err");
    return CODE_ERROR;
}

int BleProtocol::UpdateStatusCurtain(uint16_t devAddr)
{
    LOGD("UpdateStatusCurtain 0x%04x", devAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t updateHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t future[6];
    } update_message_t;
    update_message_t update_message = {0};
    memset(&update_message, 0x00, sizeof(update_message));
    update_message.ble_message_header.devAddr = devAddr;
    update_message.opcodeVendor = RD_OPCODE_CONFIG;
    update_message.vendorId = RD_VENDOR_ID;
    update_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    update_message.header = RD_HEADER_REQUEST_STATUS_CURTAIN;
    int rs = SendMessage(APP_REQ, (uint8_t *)&update_message, sizeof(update_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, updateHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
        } calib_rsp_message_t;
        calib_rsp_message_t *calib_rsp_message = (calib_rsp_message_t *)dataRsp;
        if (calib_rsp_message->header == RD_HEADER_REQUEST_STATUS_CURTAIN)
        {
            return CODE_OK;
        }
        LOGW("update resp state not match with input control");
    }
    LOGW("update err");
    return CODE_ERROR;
}

int BleProtocol::ScanStopSeftPowerRemote(uint16_t devAddr, uint8_t status)
{
    LOGD("Scan SeftPowerRemote: 0x%04X, status: %d", devAddr, status);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t dataCompare[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t status;
        uint8_t future[5];
    } scan_message_t;
    scan_message_t scan_message = {0};
    memset(&scan_message, 0x00, sizeof(scan_message));
    scan_message.ble_message_header.devAddr = devAddr;
    scan_message.opcodeVendor = RD_OPCODE_CONFIG;
    scan_message.vendorId = RD_VENDOR_ID;
    scan_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    scan_message.header = RD_HEADER_SEFTPOWER_REMOTE_SCAN;
    scan_message.status = status;
    uint16_t timeout = 10000;
    if (!status)
        timeout = 1000;
    int rs = SendMessage(APP_REQ, (uint8_t *)&scan_message, sizeof(scan_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, timeout, dataCompare, 0, 7);
    if (rs == CODE_OK)
    {
        memcpy(&scanDevicePairMessage, dataRsp, sizeof(scan_device_pair_message_t));
        // LOGW("parent: %d", scanDevicePairMessage.parentAddr);
        // LOGW("mac: %02x%02x%02x%02x", scanDevicePairMessage.mac[0], scanDevicePairMessage.mac[1], scanDevicePairMessage.mac[2], scanDevicePairMessage.mac[3]);
        // LOGW("type: %d", scanDevicePairMessage.type);
    }
    return rs;
}

int BleProtocol::SaveSeftPowerRemote(scan_device_pair_message_t scanMessage, uint16_t childDev)
{
    LOGD("Save SeftPowerRemote");
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t dataCompare[] = {(uint8_t)(scanMessage.parentAddr & 0xFF), (uint8_t)((scanMessage.parentAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02, 0x0b, 0x0d};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t mac[4];
        uint16_t childDev;
    } save_message_t;
    save_message_t save_message = {0};
    memset(&save_message, 0x00, sizeof(save_message));
    save_message.ble_message_header.devAddr = scanMessage.parentAddr;
    save_message.opcodeVendor = RD_OPCODE_CONFIG;
    save_message.vendorId = RD_VENDOR_ID;
    save_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    save_message.header = RD_HEADER_SEFTPOWER_REMOTE_SAVE;
    save_message.childDev = childDev;
    for (int i = 0; i < 4; i++)
    {
        save_message.mac[i] = scanMessage.mac[i];
    }
    int rs = SendMessage(APP_REQ, (uint8_t *)&save_message, sizeof(save_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 3000, dataCompare, 0, 9);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t mac[4];
            uint8_t status;
        } save_rsp_message_t;
        save_rsp_message_t *save_rsp_message = (save_rsp_message_t *)dataRsp;
        uint32_t parentAddrRsp = save_rsp_message->mac[0] | save_rsp_message->mac[1] << 8 | save_rsp_message->mac[2] << 16 | save_rsp_message->mac[3] << 24;
        uint32_t parentAddr = scanMessage.mac[0] | scanMessage.mac[1] << 8 | scanMessage.mac[2] << 16 | scanMessage.mac[3] << 24;
        if ((parentAddrRsp != parentAddr) || (save_rsp_message->status == 0))
            rs = CODE_ERROR;
    }
    return rs;
}

int BleProtocol::SetSceneSeftPowerRemote(uint16_t devAddr, uint16_t seftPowerAddr, uint8_t button, uint8_t mode, uint16_t scene)
{
    LOGD("SetSceneSeftPowerRemote parent: 0x%04X, device: 0x%04X, button: %d, mode: %d, scene: %d", devAddr, seftPowerAddr, button, mode, scene);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t dataCompare[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t childDev;
        uint8_t button;
        uint8_t mode;
        uint16_t scene;
    } scene_message_t;
    scene_message_t scene_message = {0};
    memset(&scene_message, 0x00, sizeof(scene_message));
    scene_message.ble_message_header.devAddr = devAddr;
    scene_message.opcodeVendor = RD_OPCODE_CONFIG;
    scene_message.vendorId = RD_VENDOR_ID;
    scene_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    scene_message.header = RD_HEADER_SEFTPOWER_REMOTE_SET_SCENE;
    scene_message.childDev = seftPowerAddr;
    scene_message.button = button;
    scene_message.mode = mode;
    scene_message.scene = scene;

    int rs = SendMessage(APP_REQ, (uint8_t *)&scene_message, sizeof(scene_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, dataCompare, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t status;
        } scene_rsp_message_t;
        scene_rsp_message_t *scene_rsp_message = (scene_rsp_message_t *)dataRsp;
        if (scene_rsp_message->header != RD_HEADER_SEFTPOWER_REMOTE_SET_SCENE)
            rs = CODE_ERROR;
    }
    return rs;
}

int BleProtocol::DelSceneSeftPowerRemote(uint16_t devAddr, uint16_t seftPowerAddr, uint8_t button, uint8_t mode)
{
    LOGD("DelSceneSeftPowerRemote parent: 0x%04X, device: 0x%04X, button: %d, mode: %d", devAddr, seftPowerAddr, button, mode);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t dataCompare[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t childDev;
        uint8_t button;
        uint8_t mode;
        uint16_t feature;
    } scene_message_t;
    scene_message_t scene_message = {0};
    memset(&scene_message, 0x00, sizeof(scene_message));
    scene_message.ble_message_header.devAddr = devAddr;
    scene_message.opcodeVendor = RD_OPCODE_CONFIG;
    scene_message.vendorId = RD_VENDOR_ID;
    scene_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    scene_message.header = RD_HEADER_SEFTPOWER_REMOTE_DEL_SCENE;
    scene_message.childDev = seftPowerAddr;
    scene_message.button = button;
    scene_message.mode = mode;
    int rs = SendMessage(APP_REQ, (uint8_t *)&scene_message, sizeof(scene_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, dataCompare, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t status;
        } scene_rsp_message_t;
        scene_rsp_message_t *scene_rsp_message = (scene_rsp_message_t *)dataRsp;
        if (scene_rsp_message->header != RD_HEADER_SEFTPOWER_REMOTE_DEL_SCENE)
            rs = CODE_ERROR;
    }
    return rs;
}

int BleProtocol::ResetSeftPowerRemote(uint16_t devAddr, uint16_t seftPowerAddr)
{
    LOGD("ResetSeftPowerRemote parent: 0x%04X, device: 0x%04X", devAddr, seftPowerAddr);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t dataCompare[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t childDev;
        uint8_t feature[4];
    } reset_message_t;
    reset_message_t reset_message = {0};
    memset(&reset_message, 0x00, sizeof(reset_message));
    reset_message.ble_message_header.devAddr = devAddr;
    reset_message.opcodeVendor = RD_OPCODE_CONFIG;
    reset_message.vendorId = RD_VENDOR_ID;
    reset_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    reset_message.header = RD_HEADER_SEFTPOWER_REMOTE_RESET;
    reset_message.childDev = seftPowerAddr;
    int rs = SendMessage(APP_REQ, (uint8_t *)&reset_message, sizeof(reset_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, dataCompare, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t childAddr;
            uint8_t status;
        } reset_rsp_message_t;
        reset_rsp_message_t *reset_rsp_message = (reset_rsp_message_t *)dataRsp;
        if (reset_rsp_message->header != RD_HEADER_SEFTPOWER_REMOTE_RESET)
            rs = CODE_ERROR;
    }
    return rs;
}

int BleProtocol::ControlRgbSwitch(uint16_t devAddr, uint8_t button, uint8_t b, uint8_t g, uint8_t r, uint8_t dimOn, uint8_t dimOff)
{
    LOGD("ControlRgbSwitch 0x%04X, button %d, r %d, g %d, b %d, dimon %d, dimOff %d", devAddr, button, r, g, b, dimOn, dimOff);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t controlRgbSwitchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t button;
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t dimOn;
        uint8_t dimOff;
    } controlrgb_switch_message_t;
    controlrgb_switch_message_t controlrgb_switch_message = {0};
    memset(&controlrgb_switch_message, 0x00, sizeof(controlrgb_switch_message));
    controlrgb_switch_message.ble_message_header.devAddr = devAddr;
    controlrgb_switch_message.opcodeVendor = RD_OPCODE_CONFIG;
    controlrgb_switch_message.vendorId = RD_VENDOR_ID;
    controlrgb_switch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    controlrgb_switch_message.header = RD_HEADER_CONFIG_CONTROL_RGB_SWITCH;
    controlrgb_switch_message.button = button;
    controlrgb_switch_message.b = b;
    controlrgb_switch_message.g = g;
    controlrgb_switch_message.r = r;
    controlrgb_switch_message.dimOn = dimOn;
    controlrgb_switch_message.dimOff = dimOff;
    int rs = SendMessage(APP_REQ, (uint8_t *)&controlrgb_switch_message, sizeof(controlrgb_switch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, controlRgbSwitchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t button;
        } controlrgb_switch_rsp_message_t;
        controlrgb_switch_rsp_message_t *controlrgb_switch_rsp_message = (controlrgb_switch_rsp_message_t *)dataRsp;
        if (controlrgb_switch_rsp_message->header == RD_HEADER_CONFIG_CONTROL_RGB_SWITCH && controlrgb_switch_rsp_message->button == button)
        {
            return CODE_OK;
        }
        LOGW("control rgb switch resp state not match with input control");
    }
    LOGW("control rgb switch err");
    return CODE_ERROR;
}

int BleProtocol::ControlRelayOfSwitch(uint16_t devAddr, uint32_t type, uint8_t relay, uint8_t value)
{
    LOGD("ControlRelayOfSwitch 0x%04X, relayid %d, value %d", devAddr, relay, value);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t controlRelaySwitchHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t relay;
        uint8_t value;
    } control_relay_switch_message_t;
    control_relay_switch_message_t control_relay_switch_message = {0};
    memset(&control_relay_switch_message, 0x00, sizeof(control_relay_switch_message));
    control_relay_switch_message.ble_message_header.devAddr = devAddr;
    control_relay_switch_message.opcodeVendor = RD_OPCODE_CONFIG;
    control_relay_switch_message.vendorId = RD_VENDOR_ID;
    control_relay_switch_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    switch (type)
    {
    case BLE_SWITCH_1:
    case BLE_SWITCH_WATER_HEATER:
        control_relay_switch_message.header = RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_1;
        break;
    case BLE_SWITCH_2:
        control_relay_switch_message.header = RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_2;
        break;
    case BLE_SWITCH_3:
        control_relay_switch_message.header = RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_3;
        break;
    default:
        control_relay_switch_message.header = RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_4;
        break;
    }
    control_relay_switch_message.relay = relay;
    control_relay_switch_message.value = value;
    int rs = SendMessage(APP_REQ, (uint8_t *)&control_relay_switch_message, sizeof(control_relay_switch_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, controlRelaySwitchHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t relay;
            uint8_t value;
        } control_relay_switch_rsp_message_t;
        control_relay_switch_rsp_message_t *control_relay_switch_rsp_message = (control_relay_switch_rsp_message_t *)dataRsp;
        if (control_relay_switch_rsp_message->header == RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_4 &&
            control_relay_switch_rsp_message->relay == relay &&
            control_relay_switch_rsp_message->value == value)
        {
            return CODE_OK;
        }
        LOGW("control relay switch resp state not match with input control");
    }
    LOGW("control relay switch err");
    return CODE_ERROR;
}

int BleProtocol::SetIdCombine(uint16_t devAddr, uint16_t id)
{
    LOGD("SetIdCombine 0x%04x, id %d", devAddr, id);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t setIdCombineHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint16_t id;
    } set_id_combine_message_t;
    set_id_combine_message_t set_id_combine_message = {0};
    memset(&set_id_combine_message, 0x00, sizeof(set_id_combine_message));
    set_id_combine_message.ble_message_header.devAddr = devAddr;
    set_id_combine_message.opcodeVendor = RD_OPCODE_CONFIG;
    set_id_combine_message.vendorId = RD_VENDOR_ID;
    set_id_combine_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    set_id_combine_message.header = RD_HEADER_CONFIG_SET_ID_COMBINE;
    set_id_combine_message.id = id;
    int rs = SendMessage(APP_REQ, (uint8_t *)&set_id_combine_message, sizeof(set_id_combine_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 2000, setIdCombineHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint16_t id;
        } set_id_combine_rsp_message_t;
        set_id_combine_rsp_message_t *set_id_combine_rsp_message = (set_id_combine_rsp_message_t *)dataRsp;
        if (set_id_combine_rsp_message->header == RD_HEADER_CONFIG_SET_ID_COMBINE && set_id_combine_rsp_message->id == id)
        {
            return CODE_OK;
        }
        LOGW("set id combine resp state not match with input control");
    }
    LOGW("set id combine err");
    return CODE_ERROR;
}

int BleProtocol::CountDownSwitch(uint16_t devAddr, uint16_t timer, uint8_t status)
{
    LOGD("CountDownSwitch 0x%04x, timer %d, status %d", devAddr, timer, status);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t timerHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t status;
        uint16_t timer;
    } timer_message_t;
    timer_message_t timer_message = {0};
    memset(&timer_message, 0x00, sizeof(timer_message));
    timer_message.ble_message_header.devAddr = devAddr;
    timer_message.opcodeVendor = RD_OPCODE_CONFIG;
    timer_message.vendorId = RD_VENDOR_ID;
    timer_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    timer_message.header = RD_HEADER_CONFIG_SET_TIMER;
    timer_message.status = status;
    timer_message.timer = timer;
    int rs = SendMessage(APP_REQ, (uint8_t *)&timer_message, sizeof(timer_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, timerHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t status;
            uint8_t timer[2];
        } timer_rsp_message_t;
        timer_rsp_message_t *timer_rsp_message = (timer_rsp_message_t *)dataRsp;
        if (timer_rsp_message->header == RD_HEADER_CONFIG_SET_TIMER && timer_rsp_message->status == status)
        {
            return CODE_OK;
        }
        LOGW("CountDownSwitch resp state not match with input control");
    }
    LOGW("CountDownSwitch err");
    return CODE_ERROR;
}

int BleProtocol::UpdateStatusRelaySwitch(uint16_t devAddr, uint32_t type)
{
    LOGV("Update status Relay Switch 0x%04x", devAddr);
    uint16_t header;
    switch (type)
    {
    case BLE_SWITCH_1:
        header = RD_HEADER_REQUEST_STATUS_SWITCH_1;
        break;
    case BLE_SWITCH_2:
        header = RD_HEADER_REQUEST_STATUS_SWITCH_2;
        break;
    case BLE_SWITCH_3:
        header = RD_HEADER_REQUEST_STATUS_SWITCH_3;
        break;
    case BLE_SWITCH_4:
        header = RD_HEADER_REQUEST_STATUS_SWITCH_4;
        break;
    default:
        header = RD_HEADER_REQUEST_STATUS_SWITCH;
    }
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t timerHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
    } request_status_message_t;
    request_status_message_t request_status_message = {0};
    memset(&request_status_message, 0x00, sizeof(request_status_message));
    request_status_message.ble_message_header.devAddr = devAddr;
    request_status_message.opcodeVendor = RD_OPCODE_CONFIG;
    request_status_message.vendorId = RD_VENDOR_ID;
    request_status_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    request_status_message.header = header;
    int rs = SendMessage(APP_REQ, (uint8_t *)&request_status_message, sizeof(request_status_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, timerHeader, 0, 7);
    if (rs == CODE_OK)
    {
        return CODE_OK;
    }
    // LOGW("request status switch err");
    return CODE_ERROR;
}

int BleProtocol::ConfigStatusStartupSwitch(uint16_t devAddr, uint8_t status, uint32_t type)
{
    LOGD("ConfigStatusStartup 0x%04x, status %d", devAddr, status);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t statusHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t status;
    } status_message_t;
    status_message_t status_message = {0};
    memset(&status_message, 0x00, sizeof(status_message));
    status_message.ble_message_header.devAddr = devAddr;
    status_message.opcodeVendor = RD_OPCODE_CONFIG;
    status_message.vendorId = RD_VENDOR_ID;
    status_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    status_message.header = RD_HEADER_CONFIG_STATUS_STARTUP_SWITCH;
    status_message.status = status;
    int rs = SendMessage(APP_REQ, (uint8_t *)&status_message, sizeof(status_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, statusHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t status;
        } status_rsp_message_t;
        status_rsp_message_t *status_rsp_message = (status_rsp_message_t *)dataRsp;
        if (status_rsp_message->header == RD_HEADER_CONFIG_STATUS_STARTUP_SWITCH && status_rsp_message->status == status)
        {
            return CODE_OK;
        }
        LOGW("status startup resp state not match with input control");
    }
    LOGW("status startup switch err");
    return CODE_ERROR;
}

int BleProtocol::ConfigModeInputSwitchOnoff(uint16_t devAddr, uint8_t mode)
{
    LOGD("ConfigModeInputStatusOnoff 0x%04x, mode %d", devAddr, mode);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t modeHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t mode;
    } mode_message_t;
    mode_message_t mode_message = {0};
    memset(&mode_message, 0x00, sizeof(mode_message));
    mode_message.ble_message_header.devAddr = devAddr;
    mode_message.opcodeVendor = RD_OPCODE_CONFIG;
    mode_message.vendorId = RD_VENDOR_ID;
    mode_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    mode_message.header = RD_HEADER_CONFIG_MODE_INPUT_SWITCHONOFF;
    mode_message.mode = mode;
    int rs = SendMessage(APP_REQ, (uint8_t *)&mode_message, sizeof(mode_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, modeHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t mode;
        } mode_rsp_message_t;
        mode_rsp_message_t *mode_rsp_message = (mode_rsp_message_t *)dataRsp;
        if (mode_rsp_message->header == RD_HEADER_CONFIG_MODE_INPUT_SWITCHONOFF && mode_rsp_message->mode == mode)
        {
            return CODE_OK;
        }
        LOGW("mode input resp state not match with input control");
    }
    LOGW("mode input switch err");
    return CODE_ERROR;
}

int BleProtocol::ConfigModeInputModuleInOut(uint16_t devAddr, uint8_t index, uint8_t mode)
{
    LOGD("ConfigModeInputModuleInOut 0x%04x, index %d, mode %d", devAddr, index, mode);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t modeHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t indexIn;
        uint8_t mode;
        uint8_t magic[4];
    } mode_message_t;
    mode_message_t mode_message = {0};
    memset(&mode_message, 0x00, sizeof(mode_message));
    mode_message.ble_message_header.devAddr = devAddr;
    mode_message.opcodeVendor = RD_OPCODE_CONFIG;
    mode_message.vendorId = RD_VENDOR_ID;
    mode_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    mode_message.header = RD_HEADER_CONFIG_MODE_INPUT_MODULE_INOUT;
    mode_message.indexIn = index;
    mode_message.mode = mode;
    int rs = SendMessage(APP_REQ, (uint8_t *)&mode_message, sizeof(mode_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, modeHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t indexIn;
            uint8_t mode;
        } mode_rsp_message_t;
        mode_rsp_message_t *mode_rsp_message = (mode_rsp_message_t *)dataRsp;
        if (mode_rsp_message->header == RD_HEADER_CONFIG_MODE_INPUT_MODULE_INOUT && mode_rsp_message->mode == mode)
        {
            return CODE_OK;
        }
        LOGW("mode input module inout resp state not match with input control");
    }
    LOGW("mode input switch err");
    return CODE_ERROR;
}

int BleProtocol::ConfigCombinInOutModuleInOut(uint16_t devAddr, uint8_t indexIn, uint8_t indexOut)
{
    LOGD("ConfigCombinInOutModuleInOut 0x%04x, indexIn %d, indexOut %d", devAddr, indexIn, indexOut);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t combineHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t indexIn;
        uint8_t indexOut;
        uint8_t magic[4];
    } combine_message_t;
    combine_message_t combine_message = {0};
    memset(&combine_message, 0x00, sizeof(combine_message));
    combine_message.ble_message_header.devAddr = devAddr;
    combine_message.opcodeVendor = RD_OPCODE_CONFIG;
    combine_message.vendorId = RD_VENDOR_ID;
    combine_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    combine_message.header = RD_HEADER_CONFIG_COMBINE_MODULE_INOUT;
    combine_message.indexIn = indexIn;
    combine_message.indexOut = indexOut;
    int rs = SendMessage(APP_REQ, (uint8_t *)&combine_message, sizeof(combine_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, combineHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t indexIn;
            uint8_t indexOut;
        } combine_rsp_message_t;
        combine_rsp_message_t *combine_rsp_message = (combine_rsp_message_t *)dataRsp;
        if (combine_rsp_message->header == RD_HEADER_CONFIG_COMBINE_MODULE_INOUT && combine_rsp_message->indexIn == indexIn && combine_rsp_message->indexOut == indexOut)
        {
            return CODE_OK;
        }
        LOGW("combine module inout resp state not match with input control");
    }
    LOGW("ConfigCombinInOutModuleInOut err");
    return CODE_ERROR;
}

int BleProtocol::SetSceneModuleInOut(uint16_t devAddr, uint8_t type, uint8_t indexIn, uint8_t status, uint16_t sceneId)
{
    LOGD("SetSceneModuleInOut 0x%04x, type %d, indexIn %d, status %d, sceneId: %d", devAddr, type, indexIn, status, sceneId);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t sceneHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t type;
        uint16_t sceneId;
        uint8_t indexIn;
        uint8_t status;
        uint8_t magic;
    } scene_message_t;
    scene_message_t scene_message = {0};
    memset(&scene_message, 0x00, sizeof(scene_message));
    scene_message.ble_message_header.devAddr = devAddr;
    scene_message.opcodeVendor = RD_OPCODE_CONFIG;
    scene_message.vendorId = RD_VENDOR_ID;
    scene_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    scene_message.header = RD_HEADER_SETSCENE_MODULE_INOUT;
    scene_message.type = type;
    scene_message.sceneId = sceneId;
    scene_message.indexIn = indexIn;
    scene_message.status = status;
    int rs = SendMessage(APP_REQ, (uint8_t *)&scene_message, sizeof(scene_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, sceneHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t type;
            uint16_t sceneId;
            uint8_t indexIn;
            uint8_t status;
        } scene_rsp_message_t;
        scene_rsp_message_t *scene_rsp_message = (scene_rsp_message_t *)dataRsp;
        if (scene_rsp_message->header == RD_HEADER_SETSCENE_MODULE_INOUT && scene_rsp_message->type == type && scene_rsp_message->sceneId == sceneId)
        {
            return CODE_OK;
        }
        LOGW("scene module inout resp state not match with input control");
    }
    LOGW("SetSceneModuleInOut err");
    return CODE_ERROR;
}

int BleProtocol::ConfigDeltaADC(uint16_t devAddr, uint8_t delta)
{
    LOGD("ConfigDeltaADC 0x%04x, delta %d", devAddr, delta);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t deltaHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t delta;
        uint8_t magic[5];
    } delta_message_t;
    delta_message_t delta_message = {0};
    memset(&delta_message, 0x00, sizeof(delta_message));
    delta_message.ble_message_header.devAddr = devAddr;
    delta_message.opcodeVendor = RD_OPCODE_CONFIG;
    delta_message.vendorId = RD_VENDOR_ID;
    delta_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    delta_message.header = RD_HEADER_CONFIG_DELTA_ADC;
    delta_message.delta = delta;
    int rs = SendMessage(APP_REQ, (uint8_t *)&delta_message, sizeof(delta_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, deltaHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t delta;
        } delta_rsp_message_t;
        delta_rsp_message_t *delta_rsp_message = (delta_rsp_message_t *)dataRsp;
        if (delta_rsp_message->header == RD_HEADER_CONFIG_DELTA_ADC && delta_rsp_message->delta == delta)
        {
            return CODE_OK;
        }
        LOGW("delta module inout resp state not match with input control");
    }
    LOGW("ConfigDeltaADC err");
    return CODE_ERROR;
}

int BleProtocol::ConfigStatusStartupRelay(uint16_t devAddr, uint8_t relayId, uint8_t status)
{
    LOGD("ConfigStatusStartupRelay 0x%04x, relay %d, status %d", devAddr, relayId, status);
    uint8_t dataRsp[100];
    int lenRsp;
    uint8_t statusHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0xe3, 0x11, 0x02};
    typedef struct __attribute__((packed))
    {
        ble_message_header_t ble_message_header;
        uint8_t opcodeVendor;
        uint16_t vendorId;
        uint8_t opcodeRsp;
        uint8_t tidPos;
        uint16_t header;
        uint8_t relay;
        uint8_t status;
        uint8_t magic[4];
    } status_message_t;
    status_message_t status_message = {0};
    memset(&status_message, 0x00, sizeof(status_message));
    status_message.ble_message_header.devAddr = devAddr;
    status_message.opcodeVendor = RD_OPCODE_CONFIG;
    status_message.vendorId = RD_VENDOR_ID;
    status_message.opcodeRsp = RD_OPCODE_CONFIG_RSP;
    status_message.header = RD_HEADER_CONFIG_STATUS_STARTUP_RELAY;
    status_message.relay = relayId;
    status_message.status = status;
    int rs = SendMessage(APP_REQ, (uint8_t *)&status_message, sizeof(status_message_t), HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, statusHeader, 0, 7);
    if (rs == CODE_OK)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t devAddr;
            uint16_t gwAddr;
            uint8_t opcodeRsp;
            uint16_t vendorId;
            uint16_t header;
            uint8_t relay;
            uint8_t status;
        } status_rsp_message_t;
        status_rsp_message_t *status_rsp_message = (status_rsp_message_t *)dataRsp;
        if (status_rsp_message->header == RD_HEADER_CONFIG_STATUS_STARTUP_RELAY && status_rsp_message->relay == relayId && status_rsp_message->status == status)
        {
            return CODE_OK;
        }
        LOGW("status startup relay module inout resp state not match with input control");
    }
    LOGW("ConfigStatusStartupRelay err");
    return CODE_ERROR;
}

int BleProtocol::GetInfogw()
{
    LOGD("GetInfogw");
    uint8_t n = 0x02;
    SendMessage(SYSTEM_REQ, &n, 1, 0, 0, 0, 10000);
    uint8_t d[2] = {0x16, 0x00};
    return SendMessage(SYSTEM_REQ, d, 2, 0, 0, 0, 1000);
}

int BleProtocol::GetInfoMesh()
{
    LOGD("GetInfoMesh");
    uint8_t d = HCI_GATEWAY_CMD_GET_PRO_SELF_STS;
    uint8_t dataRsp[100];
    int lenRsp;
    return SendMessage(SYSTEM_REQ, &d, 1, HCI_GATEWAY_CMD_PRO_STS_RSP, dataRsp, &lenRsp, 2000);
}

int BleProtocol::UpdateDeviceKeyDev(uint16_t devAddr, string devKeyDev)
{
    LOGD("UpdateDeviceKeyDev: %d, devKey: %s", devAddr, devKeyDev.c_str());
    devKeyDev.erase(std::remove(devKeyDev.begin(), devKeyDev.end(), '-'), devKeyDev.end());
    if (devKeyDev.size() == 32)
    {
        typedef struct __attribute__((packed))
        {
            uint8_t header;
            uint16_t devAddr;
            uint16_t element;
            uint8_t devKey[16];
        } update_devkey_device_t;
        update_devkey_device_t update_devkey_device = {
            .header = 0x12,
            .devAddr = devAddr};
        update_devkey_device.element = 0x0002;
        for (int i = 0; i < devKeyDev.length(); i += 2)
        {
            std::string hexByte = devKeyDev.substr(i, 2);
            update_devkey_device.devKey[i / 2] = std::stoi(hexByte, nullptr, 16);
        }
        return SendMessage(SYSTEM_REQ, (uint8_t *)&update_devkey_device, sizeof(update_devkey_device_t), 0, 0, 0, 600);
    }
    else
    {
        LOGW("devKey error");
    }
    return CODE_ERROR;
}

int BleProtocol::UpdateDeviceKeyGateway(uint16_t gwAddr, string devKeyDev)
{
    LOGD("UpdateDeviceKeyGateway: %s", devKeyDev.c_str());
    devKeyDev.erase(std::remove(devKeyDev.begin(), devKeyDev.end(), '-'), devKeyDev.end());
    if (devKeyDev.size() == 32)
    {
        typedef struct __attribute__((packed))
        {
            uint8_t header;
            uint16_t devAddr;
            uint16_t element;
            uint8_t devKey[16];
        } update_devkey_device_t;
        update_devkey_device_t update_devkey_device = {
            .header = 0x12,
            .devAddr = gwAddr};
        update_devkey_device.element = 0x0001;
        for (int i = 0; i < devKeyDev.length(); i += 2)
        {
            std::string hexByte = devKeyDev.substr(i, 2);
            update_devkey_device.devKey[i / 2] = std::stoi(hexByte, nullptr, 16);
        }
        return SendMessage(SYSTEM_REQ, (uint8_t *)&update_devkey_device, sizeof(update_devkey_device_t), 0, 0, 0, 1000);
    }
    else
    {
        LOGW("UpdateDeviceKeyGateway error");
    }
    return CODE_ERROR;
}

int BleProtocol::UpdateNetKey(uint16_t gwAddr, string netKey, uint32_t indexId)
{
    LOGD("UpdateNetKey: %s, indexId: %d", netKey.c_str(), indexId);
    netKey.erase(std::remove(netKey.begin(), netKey.end(), '-'), netKey.end());
    if (netKey.size() == 32)
    {
        typedef struct __attribute__((packed))
        {
            uint8_t opcode;
            uint8_t netKey[16];
            uint8_t rev[3];
            uint32_t index;
            uint16_t addGw;
        } set_netkey_message_t;
        set_netkey_message_t set_netkey_message;
        memset(&set_netkey_message, 0x00, sizeof(set_netkey_message));
        set_netkey_message.opcode = HCI_GATEWAY_CMD_SET_PRO_PARA;
        for (int i = 0; i < netKey.length(); i += 2)
        {
            std::string hexByte = netKey.substr(i, 2);
            set_netkey_message.netKey[i / 2] = std::stoi(hexByte, nullptr, 16);
        }
        set_netkey_message.index = bswap_32(indexId);
        set_netkey_message.addGw = gwAddr;
        uint16_t adrGw = gwAddr;
        Gateway::GetInstance()->setBleAddr(adrGw);
        Database::GetInstance()->GatewayUpdateData();
        return SendMessage(SYSTEM_REQ, (uint8_t *)&set_netkey_message, sizeof(set_netkey_message_t), HCI_GATEWAY_CMD_SEND_IVI, 0, 0, 1000);
    }
    else
    {
        LOGW("UpdateNetKey error");
    }
    return CODE_ERROR;
}

int BleProtocol::UpdateDevKey(uint16_t gwAddr, string devKey)
{
    LOGD("UpdateDevKey %s", devKey.c_str());
    devKey.erase(std::remove(devKey.begin(), devKey.end(), '-'), devKey.end());
    if (devKey.size() == 32)
    {
        typedef struct __attribute__((packed))
        {
            uint8_t opcode;
            uint16_t gwAddr;
            uint8_t gwKey[16];
        } set_gwkey_message_t;
        set_gwkey_message_t set_gwkey_message;
        set_gwkey_message.opcode = 0x0D;
        set_gwkey_message.gwAddr = (gwAddr);
        for (int i = 0; i < devKey.length(); i += 2)
        {
            std::string hexByte = devKey.substr(i, 2);
            set_gwkey_message.gwKey[i / 2] = std::stoi(hexByte, nullptr, 16);
        }
        return SendMessage(SYSTEM_REQ, (uint8_t *)&set_gwkey_message, sizeof(set_gwkey_message_t), 0, 0, 0, 1000);
    }
    else
    {
        LOGW("UpdateDevKey error");
    }
    return CODE_ERROR;
}

int BleProtocol::UpdateAppKey(string appKey)
{
    LOGD("UpdateAppKey %s", appKey.c_str());
    appKey.erase(std::remove(appKey.begin(), appKey.end(), '-'), appKey.end());
    if (appKey.size() == 32)
    {
        uint8_t dataRsp[100];
        int lenRsp;
        typedef struct __attribute__((packed))
        {
            uint8_t opcode;
            uint8_t rev[3];
            uint8_t appKey[16];
        } binding_all_message_t;
        binding_all_message_t binding_all_message;
        memset(&binding_all_message, 0x00, sizeof(binding_all_message));
        binding_all_message.opcode = HCI_GATEWAY_CMD_START_KEYBIND;
        for (int i = 0; i < appKey.length(); i += 2)
        {
            std::string hexByte = appKey.substr(i, 2);
            binding_all_message.appKey[i / 2] = std::stoi(hexByte, nullptr, 16);
        }
        return SendMessage(SYSTEM_REQ, (uint8_t *)&binding_all_message, sizeof(binding_all_message_t), HCI_GATEWAY_CMD_KEY_BIND_EVT, dataRsp, &lenRsp, 30000);
    }
    else
    {
        LOGW("UpdateAppKey error");
    }
    return CODE_ERROR;
}

int BleProtocol::UpdateMaxAddr(uint16_t addr)
{
    LOGD("UpdateMaxAddr: %d", addr);
    uint8_t dataRsp[100];
    int lenRsp;
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        pro_net_info_t data;
    } provision_message_t;
    provision_message_t provision_message;
    memset(&provision_message, 0x00, sizeof(provision_message));
    provision_message.opcode = HCI_GATEWAY_CMD_SET_NODE_PARA;
    memcpy(&provision_message.data.netKey[0], &pro_net_info.netKey[0], sizeof(pro_net_info_t));
    provision_message.data.unicast_address = addr;
    return SendMessage(SYSTEM_REQ, (uint8_t *)&provision_message, sizeof(provision_message_t), HCI_GATEWAY_CMD_PROVISION_EVT, dataRsp, &lenRsp, 15000);
}
