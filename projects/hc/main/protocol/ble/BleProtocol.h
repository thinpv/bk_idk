#pragma once

#include <string>
#include <stdint.h>
#include <vector>
// #include "Uart.h"
#include <atomic>
#include <functional>
#include <mutex>

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#endif

#define BLE_GROUP_OFFSET 0xC000

#define SYSTEM_REQ 0xFFE9
#define APP_REQ 0xFFE8
#define RAL_MAGIC 0x0428

#define RD_VENDOR_ID 0x0211

#define RD_OPCODE_PROVISION 0xE0
#define RD_OPCODE_PROVISION_RSP 0xE1
#define RD_OPCODE_CONFIG 0xE2
#define RD_OPCODE_CONFIG_RSP 0xE3

#define RD_OPCODE_SENSOR_RSP 0x52

#define RD_HEADER_PROVISION_SET_GW_ADDR 0x0002
#define RD_HEADER_PROVISION_GET_DEV_TYPE 0x0003
#define RD_HEADER_SATAUS_POWER 0x0001
#define RD_HEADER_REMOTE_MODULE_DC_TYPE 0x0002
#define RD_HEADER_REMOTE_MUL_RSP_SCENE_ACTIVE 0x0502
#define RD_HEADER_REMOTE_MODULE_AC_TYPE 0x0003
#define RD_HEADER_LIGHT_SENSOR_MODULE_TYPE 0x0004
#define RD_HEADER_PIR_SENSOR_MODULE_TYPE 0x0005

#define RD_HEADER_CONFIG_SET_SCENE_SWITCH_SCENE_DC 0x0102
#define RD_HEADER_CONFIG_DEL_SCENE_SWITCH_SCENE_DC 0x0202
#define RD_HEADER_CONFIG_SET_SCENE_SWITCH_SCENE_AC 0x0103
#define RD_HEADER_CONFIG_DEL_SCENE_SWITCH_SCENE_AC 0x0203
#define RD_HEADER_RSP_LIGHT_SENSOR 0x0104
#define RD_HEADER_CONFIG_SET_SCENE_PIR_LIGHT_SENSOR 0x0145
#define RD_HEADER_CONFIG_DEL_SCENE_PIR_LIGHT_SENSOR 0x0245
#define RD_HEADER_CONFIG_SET_TIME_ACTION_PIR_LIGHT_SENSOR 0x0345
#define RD_HEADER_CONFIG_SET_MODE_ACTION_PIR_LIGHT_SENSOR 0x0445
#define RD_HEADER_CONFIG_SET_SENSI_PIR_LIGHT_SENSOR 0x0545
#define RD_HEADER_RSP_PIR_LIGHT_SENSOR_STARTUP 0x0645
#define RD_HEADER_CONFIG_SET_DISTANCE_RADA_SENSOR 0x0745
#define RD_HEADER_CONFIG_TIME_RSP_SENSOR 0x0845
#define RD_HEADER_STATUS_DOOR_SENSOR 0x0009
#define RD_HEADER_STATUS_HANGON_DOOR_SENSOR 0x0409
#define RD_HEADER_CONFIG_SET_SCENE_SCREEN_TOUCH 0x010A
#define RD_HEADER_CONFIG_DEL_SCENE_SCREEN_TOUCH 0x020A
#define RD_HEADER_CONFIG_SEND_WEATHER_INDOOR 0x030A
#define RD_HEADER_CONFIG_SEND_WEATHER_OUTDOOR 0x050A
#define RD_HEADER_CONFIG_EDIT_ICON_SCREEN_TOUCH 0x070A
#define RD_HEADER_CONFIG_SEND_DATE 0x080A
#define RD_HEADER_CONFIG_SEND_TIME 0x090A
#define RD_HEADER_CONFIG_DEL_ALL_SCENE 0x0A0A
#define RD_HEADER_CONFIG_SET_GROUP 0x0B0A
#define RD_HEADER_CONFIG_ADD_ROOM 0x0C0A
#define RD_HEADER_CONFIG_DEL_ROOM 0x0D0A

#define RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_1 0x000E
#define RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_2 0x000D
#define RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_3 0x000C
#define RD_HEADER_CONFIG_CONTROL_RELAY_SWITCH_4 0x000B
#define RD_HEADER_CONFIG_STATUS_STARTUP_SWITCH 0x100B
#define RD_HEADER_CONFIG_STATUS_ALL_RELAY_SWITCH 0x110B
#define RD_HEADER_CONFIG_MODE_INPUT_SWITCHONOFF 0x0012

#define RD_HEADER_CONFIG_CONTROL_RGB_SWITCH 0x050B
#define RD_HEADER_CONFIG_SET_ID_COMBINE 0x060B
#define RD_HEADER_CONFIG_SET_TIMER 0x070B
#define RD_HEADER_REQUEST_STATUS_SWITCH_1 0x030e
#define RD_HEADER_REQUEST_STATUS_SWITCH_2 0x030d
#define RD_HEADER_REQUEST_STATUS_SWITCH_3 0x030c
#define RD_HEADER_REQUEST_STATUS_SWITCH_4 0x030b
#define RD_HEADER_REQUEST_STATUS_SWITCH 0x090B
#define RD_HEADER_CONTROL_OPEN_CLOSE_PAUSE 0x0011
#define RD_HEADER_RSP_CONTROL_OPEN_CLOSE_PAUSE_OPENED 0x0311
#define RD_HEADER_PRESS_BUTTON_CURTAN_DOOR_ROOLING 0x0611
#define RD_HEADER_CALIBAUTO 0x0711
#define RD_HEADER_LOCK 0x0811
#define RD_HEADER_MODE_WIFI 0x0911
#define RD_HEADER_CONFIG_MOTOR 0x0511
#define RD_HEADER_CALIB 0x0411
#define RD_HEADER_REQUEST_STATUS_CURTAIN 0x0311

#define RD_HEADER_SCREEN_TOUCH_PRESS 0x000A
#define RD_HEADER_SCREEN_TOUCH_REQUEST_TIME 0xF00A
#define RD_HEADER_SCREEN_TOUCH_REQUEST_TEMP 0xF10A

#define RD_HEADER_SEFTPOWER_REMOTE_SCAN 0x0a0b
#define RD_HEADER_SEFTPOWER_REMOTE_SAVE 0x0d0b
#define RD_HEADER_SEFTPOWER_REMOTE_RESET 0x0e0b
#define RD_HEADER_SEFTPOWER_REMOTE_PRESS 0x0b0b
#define RD_HEADER_SEFTPOWER_REMOTE_SET_SCENE 0x0c0b
#define RD_HEADER_SEFTPOWER_REMOTE_DEL_SCENE 0x0f0b

#define RD_HEADER_TEMP_HUM_AIR_STATUS_1 0x0006
#define RD_HEADER_TEMP_HUM_AIR_STATUS_2 0x0106
#define RD_HEADER_TEMP_SOIL_STATUS 0x1606
#define RD_HEADER_PH_SOIL_STATUS 0x1601
#define RD_HEADER_EC_SOIL_STATUS 0x1602
#define RD_HEADER_TEMP_WATER_STATUS 0x1706
#define RD_HEADER_EC_WATER_STATUS 0x1702
#define RD_HEADER_PH_WATER_STATUS 0x1701
#define RD_HEADER_OXY_WATER_STATUS 0x1703
#define RD_HEADER_EC_SALI_TDS_WATER 0x1704

#define RD_HEADER_STATUS_TEMP_HUM_OF_PM 0x0107
#define RD_HEADER_STATUS_PM 0x0207
#define RD_HEADER_STATUS_SMOKE 0x0108

#define RD_HEADER_STATUS_IN_MODULE_INOUT 0x0115
#define RD_HEADER_STATUS_ADC_MODULE_INOUT 0x0215
#define RD_HEADER_CONFIG_MODE_INPUT_MODULE_INOUT 0x0315
#define RD_HEADER_CONFIG_COMBINE_MODULE_INOUT 0x0415
#define RD_HEADER_SETSCENE_MODULE_INOUT 0x0515
#define RD_HEADER_CONFIG_DELTA_ADC 0x0715
#define RD_HEADER_CONFIG_STATUS_STARTUP_RELAY 0x0815

#define TRANSITION_DEFAULT 5

enum
{
	CLOSE,
	OPEN,
	PAUSE,
	PERCENT,
};

enum
{
	// send cmd part
	HCI_GATEWAY_CMD_START = 0x00,
	HCI_GATEWAY_CMD_STOP = 0x01,
	HCI_GATEWAY_CMD_RESET = 0x02,
	HCI_GATEWAY_CMD_CLEAR_NODE_INFO = 0x06,
	HCI_GATEWAY_CMD_SET_ADV_FILTER = 0x08,
	HCI_GATEWAY_CMD_SET_PRO_PARA = 0x09,
	HCI_GATEWAY_CMD_SET_NODE_PARA = 0x0a,
	HCI_GATEWAY_CMD_START_KEYBIND = 0x0b,
	HCI_GATEWAY_CMD_GET_PRO_SELF_STS = 0x0c,
	HCI_GATEWAY_CMD_SET_DEV_KEY = 0x0d,
	HCI_GATEWAY_CMD_GET_SNO = 0x0e,
	HCI_GATEWAY_CMD_SET_SNO = 0x0f,
	HCI_GATEWAY_CMD_GET_UUID_MAC = 0x10,
	HCI_GATEWAY_CMD_DEL_VC_NODE_INFO = 0x11,
	HCI_GATEWAY_CMD_SEND_VC_NODE_INFO = 0x12,

	// rsp cmd part
	HCI_GATEWAY_RSP_UNICAST = 0x80,
	HCI_GATEWAY_RSP_OP_CODE = 0X81,
	HCI_GATEWAY_KEY_BIND_RSP = 0x82,
	HCI_GATEWAY_CMD_STATIC_OOB_RSP = 0x87, // HCI send back the static oob information
	HCI_GATEWAY_CMD_UPDATE_MAC = 0x88,
	HCI_GATEWAY_CMD_PROVISION_EVT = 0x89,
	HCI_GATEWAY_CMD_KEY_BIND_EVT = 0x8a,
	HCI_GATEWAY_CMD_PRO_STS_RSP = 0x8b,
	HCI_GATEWAY_CMD_SEND_ELE_CNT = 0x8c,
	HCI_GATEWAY_CMD_SEND_NODE_INFO = 0x8d,
	HCI_GATEWAY_CMD_SEND_CPS_INFO = 0x8e,
	HCI_GATEWAY_CMD_HEARTBEAT = 0x8f,
	HCI_GATEWAY_CMD_SEND_MESH_OTA_STS = 0x98,
	HCI_GATEWAY_CMD_SEND_UUID = 0x99,
	HCI_GATEWAY_CMD_SEND_IVI = 0x9a,
	HCI_GATEWAY_CMD_SEND_SRC_CMD = 0x9c,
	HCI_GATEWAY_CMD_SEND_SNO_RSP = 0xa0,
	HCI_GATEWAY_CMD_SEND = 0xb1,
	HCI_GATEWAY_DEV_RSP = 0xb2,
	HCI_GATEWAY_CMD_LINK_OPEN = 0xb3,
	HCI_GATEWAY_CMD_LINK_CLS = 0xb4,
	HCI_GATEWAY_CMD_SEND_BACK_VC = 0xb5,
	HCI_GATEWAY_CMD_LOG_STRING = 0xb6,
	HCI_GATEWAY_CMD_LOG_BUF = 0xb7,
};

#define CONNECT_DEVICE_TIMEOUT 40 // seconds
#define BLE_MAX_ELEMENT 4

using namespace std;

typedef struct __attribute__((packed))
{
	uint8_t mac[6];
	uint8_t len;
	uint8_t header_type;
	uint8_t beacon_type;
	uint8_t uuid[16];
	uint32_t uri_hash;
	uint16_t obb_info;
	int8_t rssi;
	uint16_t dc;
} scan_device_message_t;

typedef struct __attribute__((packed))
{
	uint16_t parentAddr;
	uint16_t gwAddr;
	uint8_t opcodeRsp;
	uint16_t vendorId;
	uint16_t header;
	uint8_t mac[4];
	uint8_t type;
	uint8_t rssi;
} scan_device_pair_message_t;

typedef struct __attribute__((packed))
{
	uint16_t len;
	uint8_t magic;
	uint8_t opcode;
	uint8_t data[];
} message_rsp_st;

class BleProtocol// : public Uart
{
private:
	typedef struct __attribute__((packed))
	{
		uint16_t opcode;
		uint8_t data[100];
	} message_req_st;

	typedef struct __attribute__((packed))
	{
		bool status;
		uint8_t opcode;
		int *len;
		uint8_t *data;
		uint8_t *compare_data;
		int compare_position;
		int compare_len;
	} message_rsp_list_st;

	typedef struct __attribute__((packed))
	{
		uint8_t uuid[8];
		uint8_t deviceType[4];
		uint16_t fwVersion;
		uint16_t magic;
	} uuid_t;

	typedef struct __attribute__((packed))
	{
		uint8_t netKey[16];
		uint16_t key_index;
		uint8_t flag;
		uint32_t iv_index;
		uint16_t unicast_address;
	} pro_net_info_t;

	typedef struct __attribute__((packed))
	{
		uint16_t nkIdx;
		uint16_t akIdx;
		uint8_t retryCnt;
		uint8_t rspMax;
		uint16_t devAddr;
	} ble_message_header_t;

	mutex messageRespListMtx;
	vector<message_rsp_list_st *> messageRespList;
	mutex mtxWaitSendUart;

#define BLE_CHECK_OPCODE_BUFFER_MAX_SIZE 500
	mutex vectorCheckOpcodeMtx;
	vector<message_rsp_st *> messageCheckOpcodeList;

	// TODO: Add init state
	pro_net_info_t pro_net_info;
	uint8_t netKey[16];
	uint8_t appKey[16];
	uint8_t gwKey[16];
	uint8_t deviceKey[16];
	uint16_t nextAddr;

#ifdef ESP_PLATFORM
	BleProtocol(uart_port_t num, int txPin, int rxPin, int baudrate);
#else
	BleProtocol(char *uartPort, int uartBaudrate);
#endif
	virtual ~BleProtocol();

	int OnMessage(unsigned char *data, int len);
	int SendMessage(uint16_t opReq, uint8_t *dataReq, int lenReq, uint8_t opRsp, uint8_t *dataRsp, int *lenRsp, uint32_t timeout, uint8_t *compare_data = 0, int compare_position = 0, int compare_len = 0);

public:
	atomic<bool> haveNewMac;
	atomic<bool> haveGetMacRsp;
	atomic<bool> isProvisioning;
	atomic<bool> isInitKey;
	scan_device_message_t scanDeviceMessage;
	scan_device_pair_message_t scanDevicePairMessage;

	static BleProtocol *GetInstance();

	void init();

	int GetOpcodeExceptionMessage(message_rsp_st **data);
	void CheckOpcodeException(message_rsp_st *message);

	void initKey(string netKeyStr, string appKeyStr);
	void CheckKeyBle();
	// string GetAppKey();
	int GetNetKey();
	int SetNetKey();
	int SetGwKey();

	// SeftPower Remote
	int ScanStopSeftPowerRemote(uint16_t devAddr, uint8_t status);
	int SaveSeftPowerRemote(scan_device_pair_message_t scanMessage, uint16_t childDev);
	int SetSceneSeftPowerRemote(uint16_t devAddr, uint16_t seftPowerAddr, uint8_t button, uint8_t mode, uint16_t scene);
	int DelSceneSeftPowerRemote(uint16_t devAddr, uint16_t seftPowerAddr, uint8_t button, uint8_t mode);
	int ResetSeftPowerRemote(uint16_t devAddr, uint16_t seftPowerAddr);

	int StartScan();
	int StopScan();
	int ResetBle();
	int ResetFactory();

	bool IsProvision();
	void SetProvisioning(bool isProvision);
	int AddDevice(scan_device_message_t *scan_device_message);
	int AddPairDevice(uint32_t parentAddr, uint32_t childAddr);
	int SelectMac(uint8_t *mac);
	int Provision(uint16_t deviceAddr);
	int BindingAll();
	int SetGwAddr(uint16_t devAddr, uint16_t gwAddrSet = 0x0001);
	int GetDeviceType(uint8_t *mac, uint16_t devAddr, uint32_t &deviceType, uint16_t &deviceVersion);

	int ResetDev(uint16_t devAddr);
	int ResetDelAll();

	int SendOnlineCheck(uint16_t devAddr, uint32_t typeDev, uint16_t version);
	int GetTTL(uint16_t devAddr);

	int SetOnOffLight(uint16_t devAddr, uint8_t onoff, uint16_t transition, bool ack);
	int GetOnOffLight(uint16_t devAddr);
	int SetDimmingLight(uint16_t devAddr, uint16_t dim, uint16_t transition, bool ack);
	int GetDimming(uint16_t devAddr);
	int SetCctLight(uint16_t devAddr, uint16_t cct, uint16_t transition, bool ack);
	int GetCct(uint16_t devAddr);
	int SetHSLLight(uint16_t devAddr, uint16_t H, uint16_t S, uint16_t L, uint16_t transition, bool ack);
	int GetHSL(uint16_t devAddr);
	int SetCctDimLight(uint16_t devAddr, uint16_t cct, uint16_t dim, uint16_t transition, bool ack);
	int GetCctDimLight(uint16_t devAddr);
	int SetLevelDim(uint16_t devAddr, uint8_t dimMax, uint8_t dimMin);

	// group light
	int GroupAddDevice(uint16_t devAddr, uint16_t element, uint16_t group);
	int GroupDelDevice(uint16_t devAddr, uint16_t element, uint16_t group);

	// Scene light
	int SceneAddDevice(uint16_t devAddr, uint16_t scene, uint8_t modeRgb);
	int SceneDelDevice(uint16_t devAddr, uint16_t scene);
	int SceneRecall(uint16_t devAddr, uint16_t scene, uint16_t transition, bool ack);
	int CallModeRgb(uint16_t devAddr, uint8_t modeRgb);

	// update status lights
	int UpdateLights(uint16_t devAddr);
	int UpdateStatusSensorsPm(uint16_t devAddr);

	// remote scene
	int SetSceneSwitchSceneDC(uint16_t devAddr, uint8_t button, uint8_t mode, uint16_t sceneId, uint8_t type);
	int SetSceneSwitchSceneAC(uint16_t devAddr, uint8_t button, uint8_t mode, uint16_t sceneId, uint8_t type);
	int DelSceneSwitchSceneDC(uint16_t devAddr, uint8_t button, uint8_t mode);
	int DelSceneSwitchSceneAC(uint16_t devAddr, uint8_t button, uint8_t mode);

	// PirLightSensor
	int SetScenePirLightSensor(uint16_t devAddr, uint8_t condition, uint8_t pir, uint16_t lowLux, uint16_t highLux, uint16_t scene, uint8_t type);
	int DelScenePirLightSensor(uint16_t devAddr, uint16_t scene);
	int TimeActionPirLightSensor(uint16_t devAddr, uint16_t time);
	int SetModeActionPirLightSensor(uint16_t devAddr, uint8_t mode);
	int SetSensiPirLightSensor(uint16_t devAddr, uint8_t sensi);
	int SetDistanceSensor(uint16_t devAddr, uint8_t distance);
	int SetTimeRspSensor(uint16_t devAddr, uint16_t time);

	// switch
	int ControlRgbSwitch(uint16_t devAddr, uint8_t button, uint8_t b, uint8_t g, uint8_t r, uint8_t dimOn, uint8_t dimOff);
	int ControlRelayOfSwitch(uint16_t devAddr, uint32_t type, uint8_t relay, uint8_t value);
	int SetIdCombine(uint16_t devAddr, uint16_t id);
	int CountDownSwitch(uint16_t devAddr, uint16_t timer, uint8_t status);
	int UpdateStatusRelaySwitch(uint16_t devAddr, uint32_t type = 0);
	int ConfigStatusStartupSwitch(uint16_t devAddr, uint8_t status, uint32_t type = 0);
	int ConfigModeInputSwitchOnoff(uint16_t devAddr, uint8_t mode);

	// screen touch
	int SceneForScreenTouch(uint16_t devAddr, uint16_t scene, uint8_t icon, uint8_t type);
	int EditIconScreenTouch(uint16_t devAddr, uint16_t scene, uint8_t icon);
	int DelSceneScreenTouch(uint16_t devAddr, uint16_t scene);
	int DelAllScene(uint16_t devAddr);
	int SendWeatherOutdoor(uint16_t devAddr, uint8_t status, uint16_t temp);
	int SendWeatherIndoor(uint16_t devAddr, uint16_t temp, uint16_t hum, uint16_t pm25);
	int SendDate(uint16_t devAddr, uint16_t years, uint8_t month, uint8_t date, uint8_t day);
	int SendTime(uint16_t devAddr, uint8_t hours, uint8_t minute, uint8_t second);
	int SetGroup(uint16_t devAddr, uint16_t group);

	// Rooling door, curtain
	int ControlOpenClosePausePercent(uint16_t devAddr, uint8_t type, uint8_t percent = 0);
	int ConfigMotor(uint16_t devAddr, uint8_t typeMotor);
	int CalibCurtain(uint16_t devAddr, uint8_t status);
	int UpdateStatusCurtain(uint16_t devAddr);
	int CalibAuto(uint16_t devAddr, uint16_t time);
	int LockDevice(uint16_t devAddr, uint8_t locked);
	int SetModeWifi(uint16_t devAddr, uint8_t mode);

	// ModuleInOut
	int ConfigModeInputModuleInOut(uint16_t devAddr, uint8_t index, uint8_t mode);
	int ConfigCombinInOutModuleInOut(uint16_t devAddr, uint8_t indexIn, uint8_t indexOut);
	int SetSceneModuleInOut(uint16_t devAddr, uint8_t type, uint8_t indexIn, uint8_t status, uint16_t sceneId);
	int ConfigDeltaADC(uint16_t devAddr, uint8_t delta);
	int ConfigStatusStartupRelay(uint16_t devAddr, uint8_t relayId, uint8_t status);

	// Backup
	int GetInfogw();
	int GetInfoMesh();
	int UpdateDeviceKeyDev(uint16_t devAddr, string devKeyDev);
	int UpdateDeviceKeyGateway(uint16_t gwAddr, string devKeyDev);
	int UpdateNetKey(uint16_t gwAddr, string netKey, uint32_t indexId);
	int UpdateDevKey(uint16_t gwAddr, string devKey);
	int UpdateAppKey(string appKey);
	int UpdateMaxAddr(uint16_t addr);
};
