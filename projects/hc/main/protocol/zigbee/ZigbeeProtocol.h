#pragma once

#include <stdint.h>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <string>
#include <Uart.h>

#define ZIGBEE_ADDRESS_MODE_NONE_DEST 0
#define ZIGBEE_ADDRESS_MODE_GROUP 1
#define ZIGBEE_ADDRESS_MODE_SHORT 2
#define ZIGBEE_ADDRESS_MODE_IEEE 3

//  Acknowledge format
#define ZBHCI_CMD_ACKNOWLEDGE 0x8000

// BDB commands
#define ZBHCI_CMD_BDB_COMMISSION_FORMATION 0x0001
#define ZBHCI_CMD_BDB_COMMISSION_STEER 0x0002
#define ZBHCI_CMD_BDB_COMMISSION_TOUCHLINK 0x0003
#define ZBHCI_CMD_BDB_COMMISSION_FINDBIND 0x0004
#define ZBHCI_CMD_BDB_FACTORY_RESET 0x0005
#define ZBHCI_CMD_BDB_PRE_INSTALL_CODE 0x0006
#define ZBHCI_CMD_BDB_CHANNEL_SET 0x0007

// Network management command
#define ZBHCI_CMD_DISCOVERY_NWK_ADDR_REQ 0x0010
#define ZBHCI_CMD_DISCOVERY_IEEE_ADDR_REQ 0x0011
#define ZBHCI_CMD_DISCOVERY_NODE_DESC_REQ 0x0012
#define ZBHCI_CMD_DISCOVERY_SIMPLE_DESC_REQ 0x0013
#define ZBHCI_CMD_DISCOVERY_MATCH_DESC_REQ 0x0014
#define ZBHCI_CMD_DISCOVERY_ACTIVE_EP_REQ 0x0015
#define ZBHCI_CMD_BIND_REQ 0x0020
#define ZBHCI_CMD_UNBIND_REQ 0x0021
#define ZBHCI_CMD_MGMT_LQI_REQ 0x0030
#define ZBHCI_CMD_MGMT_BIND_REQ 0x0031
#define ZBHCI_CMD_MGMT_LEAVE_REQ 0x0032
#define ZBHCI_CMD_MGMT_DIRECT_JOIN_REQ 0x0033
#define ZBHCI_CMD_MGMT_PERMIT_JOIN_REQ 0x0034
#define ZBHCI_CMD_MGMT_NWK_UPDATE_REQ 0x0035
#define ZBHCI_CMD_NODES_JOINED_GET_REQ 0x0040
#define ZBHCI_CMD_NODES_TOGGLE_TEST_REQ 0x0041
#define ZBHCI_CMD_GET_LOCAL_NWK_INFO_REQ 0x0045

#define ZBHCI_CMD_DISCOVERY_NWK_ADDR_RSP 0x8010
#define ZBHCI_CMD_DISCOVERY_IEEE_ADDR_RSP 0x8011
#define ZBHCI_CMD_DISCOVERY_NODE_DESC_RSP 0x8012
#define ZBHCI_CMD_DISCOVERY_SIMPLE_DESC_RSP 0x8013
#define ZBHCI_CMD_DISCOVERY_MATCH_DESC_RSP 0x8014
#define ZBHCI_CMD_DISCOVERY_ACTIVE_EP_RSP 0x8015
#define ZBHCI_CMD_BIND_RSP 0x8020
#define ZBHCI_CMD_UNBIND_RSP 0x8021
#define ZBHCI_CMD_MGMT_LQI_RSP 0x8030
#define ZBHCI_CMD_MGMT_BIND_RSP 0x8031
#define ZBHCI_CMD_MGMT_LEAVE_RSP 0x8032
#define ZBHCI_CMD_MGMT_DIRECT_JOIN_RSP 0x8033
#define ZBHCI_CMD_MGMT_PERMIT_JOIN_RSP 0x8034
#define ZBHCI_CMD_MGMT_NWK_UPDATE_RSP 0x8035
#define ZBHCI_CMD_NODES_JOINED_GET_RSP 0x8040
#define ZBHCI_CMD_NODES_TOGGLE_TEST_RSP 0x8041
#define ZBHCI_CMD_NODES_DEV_ANNCE_IND 0x8043
#define ZBHCI_CMD_GET_LOCAL_NWK_INFO_RSP 0x8045
#define ZBHCI_CMD_DATA_CONFIRM 0x8200
#define ZBHCI_CMD_NODE_LEAVE_IND 0x8202

// ZCL Cluster commands

// General cluster command
#define ZBHCI_CMD_ZCL_ATTR_READ 0x0100
#define ZBHCI_CMD_ZCL_ATTR_WRITE 0x0101
#define ZBHCI_CMD_ZCL_CONFIG_REPORT 0x0102
#define ZBHCI_CMD_ZCL_READ_REPORT_CFG 0x0103

#define ZBHCI_CMD_ZCL_ATTR_READ_RSP 0x8100
#define ZBHCI_CMD_ZCL_ATTR_WRITE_RSP 0x8101
#define ZBHCI_CMD_ZCL_CONFIG_REPORT_RSP 0x8102
#define ZBHCI_CMD_ZCL_READ_REPORT_CFG_RSP 0x8103
#define ZBHCI_CMD_ZCL_REPORT_MSG_RCV 0x8104
#define ZBHCI_CMD_ZCL_DEFAULT_RSP 0x8105

// Basic cluster command
#define ZBHCI_CMD_ZCL_BASIC_RESET 0x0110

// Group cluster command
#define ZBHCI_CMD_ZCL_GROUP_ADD 0x0120
#define ZBHCI_CMD_ZCL_GROUP_VIEW 0x0121
#define ZBHCI_CMD_ZCL_GROUP_GET_MEMBERSHIP 0x0122
#define ZBHCI_CMD_ZCL_GROUP_REMOVE 0x0123
#define ZBHCI_CMD_ZCL_GROUP_REMOVE_ALL 0x0124
#define ZBHCI_CMD_ZCL_GROUP_ADD_IF_IDENTIFYING 0x0125

#define ZBHCI_CMD_ZCL_GROUP_ADD_RSP 0x8120
#define ZBHCI_CMD_ZCL_GROUP_VIEW_RSP 0x8121
#define ZBHCI_CMD_ZCL_GROUP_GET_MEMBERSHIP_RSP 0x8122
#define ZBHCI_CMD_ZCL_GROUP_REMOVE_RSP 0x8123

// Identify cluster command
#define ZBHCI_CMD_ZCL_IDENTIFY 0x0130
#define ZBHCI_CMD_ZCL_IDENTIFY_QUERY 0x0131

#define ZBHCI_CMD_ZCL_IDENTIFY_QUERY_RSP 0x8131

// On/Off cluster command
#define ZBHCI_CMD_ZCL_ONOFF_ON 0x0140
#define ZBHCI_CMD_ZCL_ONOFF_OFF 0x0141
#define ZBHCI_CMD_ZCL_ONOFF_TOGGLE 0x0142

// Level cluster command
#define ZBHCI_CMD_ZCL_LEVEL_MOVE2LEVEL 0x0150
#define ZBHCI_CMD_ZCL_LEVEL_MOVE 0x0151
#define ZBHCI_CMD_ZCL_LEVEL_STEP 0x0152
#define ZBHCI_CMD_ZCL_LEVEL_STOP 0x0153
#define ZBHCI_CMD_ZCL_LEVEL_MOVE2LEVEL_WITHONOFF 0x0154
#define ZBHCI_CMD_ZCL_LEVEL_MOVE_WITHONOFF 0x0155
#define ZBHCI_CMD_ZCL_LEVEL_STEP_WITHONOFF 0x0156
#define ZBHCI_CMD_ZCL_LEVEL_STOP_WITHONOFF 0x0157

// Scene cluster command
#define ZBHCI_CMD_ZCL_SCENE_ADD 0x0160
#define ZBHCI_CMD_ZCL_SCENE_VIEW 0x0161
#define ZBHCI_CMD_ZCL_SCENE_REMOVE 0x0162
#define ZBHCI_CMD_ZCL_SCENE_REMOVE_ALL 0x0163
#define ZBHCI_CMD_ZCL_SCENE_STORE 0x0164
#define ZBHCI_CMD_ZCL_SCENE_RECALL 0x0165
#define ZBHCI_CMD_ZCL_SCENE_GET_MENBERSHIP 0x0166

#define ZBHCI_CMD_ZCL_SCENE_ADD_RSP 0x8160
#define ZBHCI_CMD_ZCL_SCENE_VIEW_RSP 0x8161
#define ZBHCI_CMD_ZCL_SCENE_REMOVE_RSP 0x8162
#define ZBHCI_CMD_ZCL_SCENE_REMOVE_ALL_RSP 0x8163
#define ZBHCI_CMD_ZCL_SCENE_STORE_RSP 0x8164
#define ZBHCI_CMD_ZCL_SCENE_GET_MENBERSHIP_RSP 0x8166

// OTA cluster command
#define ZBHCI_CMD_ZCL_OTA_IMAGE_NOTIFY 0x0190

// HCI serial port upgrade command
#define ZBHCI_CMD_OTA_START_REQUEST 0x0210
#define ZBHCI_CMD_OTA_BLOCK_RESPONSE 0x0211

#define ZBHCI_CMD_OTA_START_RESPONSE 0x8210
#define ZBHCI_CMD_OTA_BLOCK_REQUEST 0x8211
#define ZBHCI_CMD_OTA_END_STATUS 0x8212

#define CONNECT_DEVICE_TIMEOUT 40 // seconds

#define MESSAGE_HEADER 0x55
#define MESSAGE_TAIL 0xAA

using namespace std;

typedef function<int(uint8_t *buff, uint16_t len)> OnCmdCallbackFunc;

class ZigbeeProtocol : public Uart
{
private:
	typedef struct __attribute__((packed))
	{
		uint16_t type;
		uint8_t status;
		uint8_t rev;
	} message_acknowledge_st;

	typedef struct __attribute__((packed))
	{
		uint8_t header;
		uint16_t type;
		uint16_t len;
		uint8_t crc;
		uint8_t payload[];
	} message_req_st;

	typedef struct __attribute__((packed))
	{
		uint8_t header;
		uint16_t type;
		uint16_t len;
		uint8_t crc;
		uint8_t payload[];
	} message_rsp_st;

	typedef struct __attribute__((packed))
	{
		uint8_t status;
		uint16_t reqType;
		uint16_t respType;
		int *len;
		uint8_t *payload;
	} message_rsp_list_st;

	typedef struct __attribute__((packed))
	{
		uint16_t nwkAddr;
		uint8_t ieeeAddr[8];
		uint8_t capability;
	} DeviceAnnounce_st;

	typedef struct __attribute__((packed))
	{
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEP;
		uint8_t dstEP;
		uint16_t clusterID;
		uint8_t status;
		uint8_t apsCnt;
	} DataConfirm_st;

	typedef struct __attribute__((packed))
	{
		uint16_t nwkAddr;
		uint8_t extAddr[8];
	} DeviceLeave_st;

	typedef struct __attribute__((packed))
	{
		uint8_t dstAddrMode;
		uint8_t dstAddr[2];
		uint8_t srcEp;
		uint8_t dstEp;
	} ZCLCmdHdr_st;

	typedef struct __attribute__((packed))
	{
		uint16_t srcAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint8_t seqNum;
		// uint8_t data[];
	} ZCLCmdRspHdr_st;

	mutex messageRespListMtx;
	vector<message_rsp_list_st *> messageRespList;

#define ZIGBEE_CHECK_OPCODE_BUFFER_MAX_SIZE 20
	mutex vectorCheckOpcodeMtx;
	vector<message_rsp_st *> messageCheckOpcodeList;

	map<uint16_t, OnCmdCallbackFunc> onCmdCallbackFuncList;

	DeviceAnnounce_st deviceAnnounce;

#ifdef ESP_PLATFORM
	ZigbeeProtocol(uart_port_t num, int txPin, int rxPin, int baudrate);
#else
	ZigbeeProtocol(char *uartPort, int uartBaudrate);
#endif
	virtual ~ZigbeeProtocol();
	
	void initCallback();

	int GetOpcodeExceptionMessage(message_rsp_st **data);
	void CheckOpcodeException(message_rsp_st *message);

	int RegisterCmdCallback(uint16_t type, OnCmdCallbackFunc onCmdCallbackFunc);
	int OnMessage(unsigned char *data, int len);
	int SendMessage(uint16_t opReq, uint8_t *dataReq, int lenReq, uint16_t opRsp, uint8_t *dataRsp, int *lenRsp, uint32_t timeout);

	int OnDeviceAnnounce(uint8_t *buff, uint16_t len);
	int OnReportAttribute(uint8_t *buff, uint16_t len);
	int OnReadAttributeResp(uint8_t *buff, uint16_t len);
	int OnDataConfirm(uint8_t *buff, uint16_t len);
	int OnDeviceLeave(uint8_t *buff, uint16_t len);

	int OnGroupAddDevice(uint8_t *buff, uint16_t len);
	int OnGroupDelDevice(uint8_t *buff, uint16_t len);

	int OnSceneAddDevice(uint8_t *buff, uint16_t len);
	int OnSceneDelDevice(uint8_t *buff, uint16_t len);

public:
	static ZigbeeProtocol *GetInstance();

	void init();

	void HandleOpcodeThread();

	// BDB commands
	int CommissionFormation();
	int ResetFactory();
	int SetChannel(uint8_t channel);

	// Network management command
	/* Get cluster list of endpoint */
	int DiscoverySimpleDescription(uint16_t addr, uint8_t endpoint);
	/* Get list endpoint of the device */
	int DiscoveryActiveEndpoint(uint16_t addr);
	int PermitJoin(uint8_t duration);
	int ResetDev(uint16_t addr, string &mac, bool rejoin, bool removeChildren);

	// General cluster command
	int ReadAttribute(uint16_t addr);

	// Group cluster command
	int GroupAddDevice(uint16_t devAddr, uint16_t epId, uint16_t groupAddr);
	int GroupDelDevice(uint16_t devAddr, uint16_t epId, uint16_t groupAddr);

	// Scene cluster command
	int SceneAddDevice(uint16_t devAddr, uint16_t epId, uint16_t sceneAddr, uint16_t groupAddr);
	int SceneDelDevice(uint16_t devAddr, uint16_t epId, uint16_t sceneAddr, uint16_t groupAddr);
	int SceneRecall(uint16_t devAddr, uint16_t epId, uint16_t sceneAddr, uint16_t groupAddr);

	// On/Off cluster command
	int ZCLOnoffDevice(uint16_t devAddr, uint8_t epId, uint8_t func);
	int ZCLLevelControlDevice(uint16_t devAddr, uint8_t epId, uint8_t func, uint16_t level, uint8_t mode, uint8_t rate, uint8_t stepSize);
	int ZCLOnoffGroup(uint16_t groupAddr, uint8_t func);
};
