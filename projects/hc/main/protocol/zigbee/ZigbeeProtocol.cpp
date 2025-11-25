#include "ZigbeeProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include "Log.h"
#include <Util.h>
#include <string.h>
#include <algorithm>
#include "Database.h"
#include "Define.h"
#include "DeviceZigbee.h"
#include "cluster/basic/ClusterBasic.h"
#ifdef ESP_PLATFORM
#include "app_task.h"
#endif

static ZigbeeProtocol *zigbeeProtocol = NULL;

ZigbeeProtocol *ZigbeeProtocol::GetInstance()
{
	if (!zigbeeProtocol)
	{
#ifdef ESP_PLATFORM
#ifdef CONFIG_TTGO_TAUDIO
		zigbeeProtocol = new ZigbeeProtocol(UART_NUM_2, GPIO_NUM_33, GPIO_NUM_34, 115200);
#else
#ifdef CONFIG_IDF_TARGET_ESP32S3
		zigbeeProtocol = new ZigbeeProtocol(UART_NUM_2, GPIO_NUM_5, GPIO_NUM_4, 115200);
#else
		zigbeeProtocol = new ZigbeeProtocol(UART_NUM_2, GPIO_NUM_23, GPIO_NUM_22, 115200);
#endif
#endif
#else
		zigbeeProtocol = new ZigbeeProtocol((char *)ZIGBEE_UART_PORT, B115200);
#endif
	}
	return zigbeeProtocol;
}

#ifdef ESP_PLATFORM
ZigbeeProtocol::ZigbeeProtocol(uart_port_t num, int txPin, int rxPin, int baudrate) : Uart(num, txPin, rxPin, baudrate)
#else
ZigbeeProtocol::ZigbeeProtocol(char *uartPort, int baudrate) : Uart(uartPort, baudrate, 100000)
#endif
{
}

ZigbeeProtocol::~ZigbeeProtocol()
{
}

void ZigbeeProtocol::HandleOpcodeThread()
{
	LOGI("Start HandleOpcodeThread");
	message_rsp_st *message_rsp = NULL;
	while (1)
	{
		if (GetOpcodeExceptionMessage(&message_rsp) == CODE_OK)
		{
			CheckOpcodeException(message_rsp);
			free(message_rsp);
		}
		SLEEP_MS(100);
	}
}

void ZigbeeProtocol::init()
{
	Uart::init();
	SLEEP_MS(100);

	initCallback();

#ifdef ESP_PLATFORM
	if (!app_new_task([](void *arg)
										{
	ZigbeeProtocol *zigbeeProtocol = (ZigbeeProtocol *)arg;
	zigbeeProtocol->HandleOpcodeThread(); }, "handleOpcodeThread", 8192, this, 10))
	{
		LOGE("Failed to create HandleOpcodeThread task");
	}
#else
	thread handleOpcodeThread(bind(&ZigbeeProtocol::HandleOpcodeThread, this));
	handleOpcodeThread.detach();
#endif
}

static uint8_t checCrC(uint16_t type, uint16_t len, uint8_t *payload)
{
	uint8_t crc8 = (type >> 0) & 0xff;
	crc8 ^= (type >> 8) & 0xff;
	crc8 ^= (len >> 0) & 0xff;
	crc8 ^= (len >> 8) & 0xff;
	for (int i = 0; i < len; i++)
	{
		crc8 ^= payload[i];
	}
	return crc8;
}

int ZigbeeProtocol::GetOpcodeExceptionMessage(message_rsp_st **data)
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

void ZigbeeProtocol::CheckOpcodeException(message_rsp_st *message_rsp)
{
	LOGV("CheckOpcodeException");
	uint16_t type = bswap_16(message_rsp->type);
	uint16_t len = bswap_16(message_rsp->len);
	if (onCmdCallbackFuncList.find(type) != onCmdCallbackFuncList.end())
	{
		OnCmdCallbackFunc onCmdCallbackFunc = onCmdCallbackFuncList[type];
		int rs = onCmdCallbackFunc(message_rsp->payload, len);
		if (rs == CODE_OK)
		{
			LOGV("onCmdCallbackFunc OK");
		}
		else
		{
			LOGW("onCmdCallbackFunc rs: %d", rs);
		}
	}
	else
	{
		LOGW("type not registed: 0x%04X", type);
	}
}

int ZigbeeProtocol::OnMessage(unsigned char *data, int len)
{
	LOGV("OnMessage len: %d", len);
	uint8_t *message = data;
	int lenRemain = len;
	message_rsp_st *message_rsp = (message_rsp_st *)message;
	Util::LedZigbee(false);
	Util::LedServiceLock();
	while (lenRemain >= sizeof(message_rsp_st) && message_rsp->header == MESSAGE_HEADER)
	{
		uint16_t type = bswap_16(message_rsp->type);
		uint16_t payloadLen = bswap_16(message_rsp->len);
		uint16_t packageLen = payloadLen + sizeof(message_rsp_st) + 1;
		if (message_rsp->payload[payloadLen] == MESSAGE_TAIL)
		{
			// LOGD("message_rsp->type: 0x%04X, message_rsp->len: %d", type, payloadLen);
			if (type == ZBHCI_CMD_ACKNOWLEDGE)
			{
				message_acknowledge_st *message_acknowledge = (message_acknowledge_st *)message_rsp->payload;
				uint16_t reqType = bswap_16(message_acknowledge->type);
				// LOGD("OnMessage resp type: 0x%04X, status: %d", reqType, message_acknowledge->status);
				messageRespListMtx.lock();
				for (auto &messageResp : messageRespList)
				{
					if ((messageResp->respType == 0 || messageResp->respType == ZBHCI_CMD_ACKNOWLEDGE) && reqType == messageResp->reqType)
					{
						messageResp->status = message_acknowledge->status;
						if (messageResp->len)
						{
							*(messageResp->len) = payloadLen;
							if (messageResp->payload)
								memcpy(messageResp->payload, message_rsp->payload, *messageResp->len);
						}
					}
				}
				messageRespListMtx.unlock();
			}
			else
			{
				messageRespListMtx.lock();
				for (auto &messageResp : messageRespList)
				{
					if (type == messageResp->respType)
					{
						messageResp->status = 0;
						if (messageResp->len)
						{
							*(messageResp->len) = payloadLen;
							if (messageResp->payload)
								memcpy(messageResp->payload, message_rsp->payload, *messageResp->len);
						}
					}
				}
				messageRespListMtx.unlock();

				vectorCheckOpcodeMtx.lock();
				if (messageCheckOpcodeList.size() < ZIGBEE_CHECK_OPCODE_BUFFER_MAX_SIZE)
				{
					message_rsp_st *messageCheckOpcode = (message_rsp_st *)malloc(packageLen);
					memcpy(messageCheckOpcode, message_rsp, packageLen);
					messageCheckOpcodeList.push_back(messageCheckOpcode);
				}
				vectorCheckOpcodeMtx.unlock();
			}
		}
		lenRemain -= packageLen;
		message += packageLen;
		message_rsp = (message_rsp_st *)message;
	}
	Util::LedZigbee(true);
	Util::LedServiceUnlock();
	return lenRemain;
}

int ZigbeeProtocol::SendMessage(uint16_t opReq, uint8_t *dataReq, int lenReq, uint16_t opRsp, uint8_t *dataRsp, int *lenRsp, uint32_t timeout)
{
	uint32_t countDelay = timeout / 10;
	uint8_t buff[128];
	message_rsp_list_st message_rsp_list = {
			.status = 0xFF,
			.reqType = opReq,
			.respType = opRsp,
			.len = lenRsp,
			.payload = dataRsp};
	if (opRsp)
	{
		messageRespListMtx.lock();
		messageRespList.push_back(&message_rsp_list);
		messageRespListMtx.unlock();
	}

	message_req_st *message_req = (message_req_st *)buff;
	message_req->header = MESSAGE_HEADER;
	message_req->type = bswap_16(opReq);
	message_req->len = bswap_16(lenReq);
	message_req->crc = checCrC(opReq, lenReq, dataReq);
	for (int i = 0; i < lenReq; i++)
	{
		message_req->payload[i] = dataReq[i];
	}
	message_req->payload[lenReq] = MESSAGE_TAIL;

	Write(buff, 7 + lenReq);

	if (opRsp)
	{
		while (message_rsp_list.status == 0xFF && countDelay--)
		{
			SLEEP_MS(10);
		}
		messageRespListMtx.lock();
		messageRespList.erase(remove(messageRespList.begin(), messageRespList.end(), &message_rsp_list), messageRespList.end());
		messageRespListMtx.unlock();
	}
	else
	{
		SLEEP_MS(timeout);
	}
	return message_rsp_list.status;
}

int ZigbeeProtocol::CommissionFormation()
{
	LOGV("CommissionFormation");
	int rs = SendMessage(ZBHCI_CMD_BDB_COMMISSION_FORMATION, 0, 0, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("CommissionFormation ok");
	}
	else
	{
		LOGE("Send CommissionFormation error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ResetFactory()
{
	LOGV("ResetFactory");
	int rs = SendMessage(ZBHCI_CMD_BDB_FACTORY_RESET, 0, 0, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("ResetFactory ok");
	}
	else
	{
		LOGE("Send ResetFactory error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::SetChannel(uint8_t channel)
{
	LOGV("SetChannel");
	int rs = SendMessage(ZBHCI_CMD_BDB_CHANNEL_SET, &channel, 1, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 5000);
	if (rs == CODE_OK)
	{
		LOGD("SetChannel ok");
	}
	else
	{
		LOGE("Send SetChannel error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::DiscoverySimpleDescription(uint16_t addr, uint8_t endpoint)
{
	LOGV("DiscoverySimpleDescription");
	typedef struct __attribute__((packed))
	{
		uint16_t dstAddr;
		uint16_t nwkAddrOfInterest;
		uint8_t endpoint;
	} discovery_simple_description_t;
	discovery_simple_description_t discovery_simple_description;
	discovery_simple_description.dstAddr = bswap_16(addr);
	discovery_simple_description.nwkAddrOfInterest = bswap_16(addr);
	discovery_simple_description.endpoint = endpoint;

	int rs = SendMessage(ZBHCI_CMD_DISCOVERY_SIMPLE_DESC_REQ, (uint8_t *)&discovery_simple_description, 5, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("DiscoverySimpleDescription ok");
	}
	else
	{
		LOGE("Send DiscoverySimpleDescription error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::DiscoveryActiveEndpoint(uint16_t addr)
{
	LOGV("DiscoveryActiveEndpoint");
	typedef struct __attribute__((packed))
	{
		uint16_t dstAddr;
		uint16_t nwkAddrOfInterest;
	} discovery_active_endpoint_t;
	discovery_active_endpoint_t discovery_active_endpoint;
	discovery_active_endpoint.dstAddr = bswap_16(addr);
	discovery_active_endpoint.nwkAddrOfInterest = bswap_16(addr);

	int rs = SendMessage(ZBHCI_CMD_DISCOVERY_ACTIVE_EP_REQ, (uint8_t *)&discovery_active_endpoint, 4, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("DiscoveryActiveEndpoint ok");
	}
	else
	{
		LOGE("Send DiscoveryActiveEndpoint error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::PermitJoin(uint8_t duration)
{
	LOGV("PermitJoin");
	typedef struct __attribute__((packed))
	{
		uint16_t dstAddr;
		uint8_t permitDuration;
		uint8_t TC_significance;
	} permit_join_req_t;
	permit_join_req_t permit_join_req = {
			.dstAddr = 0xFFFF,
			.permitDuration = duration,
			.TC_significance = 1};
	int rs = SendMessage(ZBHCI_CMD_MGMT_PERMIT_JOIN_REQ, (uint8_t *)&permit_join_req, sizeof(permit_join_req), ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("PermitJoin ok");
	}
	else
	{
		LOGE("Send PermitJoin error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ResetDev(uint16_t addr, string &mac, bool rejoin, bool removeChildren)
{
	LOGV("ResetDev");
	typedef struct __attribute__((packed))
	{
		uint16_t dstAddr;
		uint8_t ieee[8];
		uint8_t rejoin;
		uint8_t removeChildren;
	} reset_dev_req_t;
	reset_dev_req_t reset_dev_req;
	reset_dev_req.dstAddr = bswap_16(addr);
	reset_dev_req.rejoin = rejoin ? 1 : 0;
	reset_dev_req.removeChildren = removeChildren ? 1 : 0;
	if (mac.length() != 16)
	{
		LOGW("Invalid MAC address length: %zu", mac.length());
		return -1;
	}
	try
	{
		for (size_t i = 0; i < 8; ++i)
		{
			std::string byteStr = mac.substr(i * 2, 2);
			reset_dev_req.ieee[i] = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
		}
	}
	catch (const std::exception &e)
	{
		LOGW("Invalid MAC address format: %s", e.what());
		return -1;
	}

	int rs = SendMessage(ZBHCI_CMD_MGMT_LEAVE_REQ, (uint8_t *)&reset_dev_req, sizeof(reset_dev_req), ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("Reset dev ok");
	}
	else
	{
		LOGE("Send Reset dev error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ReadAttribute(uint16_t addr)
{
	LOGV("ReadAttribute");
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint16_t profileID;
		uint8_t direction;
		uint16_t clusterID;
		uint8_t attrNum;
		uint16_t attrList[32];
	} read_attribute_req_t;
	read_attribute_req_t read_attribute_req;
	read_attribute_req.dstAddrMode = ZIGBEE_ADDRESS_MODE_SHORT;
	read_attribute_req.dstAddr = bswap_16(addr);
	read_attribute_req.srcEp = 0x01;
	read_attribute_req.dstEp = 0xFF;
	read_attribute_req.profileID = bswap_16(PROFILE_ZHA);
	read_attribute_req.direction = 0;
	read_attribute_req.clusterID = bswap_16(ZCL_CLUSTER_GEN_BASIC);
	read_attribute_req.attrNum = 5;
	read_attribute_req.attrList[0] = bswap_16(ATTRIBUTE_BASIC_ZCLVersion);
	read_attribute_req.attrList[1] = bswap_16(ATTRIBUTE_BASIC_ApplicationVersion);
	read_attribute_req.attrList[2] = bswap_16(ATTRIBUTE_BASIC_ManufacturerName);
	read_attribute_req.attrList[3] = bswap_16(ATTRIBUTE_BASIC_ModelIdentifier);
	read_attribute_req.attrList[4] = bswap_16(ATTRIBUTE_BASIC_PowerSource);

	int rs = SendMessage(ZBHCI_CMD_ZCL_ATTR_READ, (uint8_t *)&read_attribute_req, 11 + 2 * read_attribute_req.attrNum, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("ReadAttribute ok");
	}
	else
	{
		LOGE("Send ReadAttribute error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::GroupAddDevice(uint16_t devAddr, uint16_t epId, uint16_t groupAddr)
{
	LOGV("GroupAddDevice devAddr: 0X%04X, groupAddr: 0X%04X, epId: %d", devAddr, groupAddr, epId);
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint16_t groupAddr;
		uint16_t groupName;
	} add_group_t;
	add_group_t add_group;

	add_group.dstAddrMode = ZIGBEE_ADDRESS_MODE_SHORT;
	add_group.dstAddr = bswap_16(devAddr);
	add_group.srcEp = 0x01;
	add_group.dstEp = epId;
	add_group.groupAddr = bswap_16(groupAddr);
	add_group.groupName = bswap_16(0x4142);

	int rs = SendMessage(ZBHCI_CMD_ZCL_GROUP_ADD, (uint8_t *)&add_group, 9, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 1000);
	if (rs == CODE_OK)
	{
		LOGD("GroupAddDevice ok");
	}
	else
	{
		LOGE("Send GroupAddDevice error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::GroupDelDevice(uint16_t devAddr, uint16_t epId, uint16_t groupAddr)
{
	LOGV("GroupDelDevice");
	// TODO: check delete device from group message struct
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint16_t groupAddr;
		uint16_t groupName;
	} add_group_t;
	add_group_t add_group;

	add_group.dstAddrMode = ZIGBEE_ADDRESS_MODE_SHORT;
	add_group.dstAddr = bswap_16(devAddr);
	add_group.srcEp = 0x01;
	add_group.dstEp = epId;
	add_group.groupAddr = bswap_16(groupAddr);
	add_group.groupName = bswap_16(0x4142);

	int rs = SendMessage(ZBHCI_CMD_ZCL_GROUP_REMOVE, (uint8_t *)&add_group, 9, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 1000);
	if (rs == CODE_OK)
	{
		LOGD("GroupDelDevice ok");
	}
	else
	{
		LOGE("Send GroupDelDevice error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::SceneAddDevice(uint16_t devAddr, uint16_t epId, uint16_t sceneAddr, uint16_t groupAddr)
{
	LOGV("SceneAddDevice");
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint16_t groupAddr;
		uint8_t sceneAddr;
	} add_scene_t;
	add_scene_t add_scene;

	add_scene.dstAddrMode = ZIGBEE_ADDRESS_MODE_SHORT;
	add_scene.dstAddr = bswap_16(devAddr);
	add_scene.srcEp = 0x01;
	add_scene.dstEp = epId;
	add_scene.groupAddr = bswap_16(groupAddr);
	add_scene.sceneAddr = sceneAddr;

	int rs = SendMessage(ZBHCI_CMD_ZCL_SCENE_STORE, (uint8_t *)&add_scene, 8, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("SceneAddDevice ok");
	}
	else
	{
		LOGE("Send SceneAddDevice error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::SceneDelDevice(uint16_t devAddr, uint16_t epId, uint16_t sceneAddr, uint16_t groupAddr)
{
	LOGV("SceneDelDevice");
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint16_t groupAddr;
		uint8_t sceneAddr;
	} del_scene_t;
	del_scene_t del_scene;

	del_scene.dstAddrMode = ZIGBEE_ADDRESS_MODE_SHORT;
	del_scene.dstAddr = bswap_16(devAddr);
	del_scene.srcEp = 0x01;
	del_scene.dstEp = epId;
	del_scene.groupAddr = bswap_16(groupAddr);
	del_scene.sceneAddr = sceneAddr;

	int rs = SendMessage(ZBHCI_CMD_ZCL_SCENE_REMOVE, (uint8_t *)&del_scene, 8, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("SceneDelDevice ok");
	}
	else
	{
		LOGE("Send SceneDelDevice error, rs: %d", rs);
	}
	return rs;
	return CODE_OK;
}

int ZigbeeProtocol::SceneRecall(uint16_t devAddr, uint16_t epId, uint16_t sceneAddr, uint16_t groupAddr)
{
	LOGV("SceneRecall");
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint16_t groupAddr;
		uint8_t sceneAddr;
	} del_scene_t;
	del_scene_t del_scene;

	del_scene.dstAddrMode = ZIGBEE_ADDRESS_MODE_SHORT;
	del_scene.dstAddr = bswap_16(devAddr);
	del_scene.srcEp = 0x01;
	del_scene.dstEp = epId;
	del_scene.groupAddr = bswap_16(groupAddr);
	del_scene.sceneAddr = sceneAddr;

	int rs = SendMessage(ZBHCI_CMD_ZCL_SCENE_RECALL, (uint8_t *)&del_scene, 8, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("SceneRecall ok");
	}
	else
	{
		LOGE("Send SceneRecall error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ZCLOnoffDevice(uint16_t devAddr, uint8_t epId, uint8_t func)
{
	LOGV("ZCLOnoffDevice");
	uint16_t codeFunc;
	if (func == 0)
		codeFunc = ZBHCI_CMD_ZCL_ONOFF_OFF;
	else if (func == 1)
		codeFunc = ZBHCI_CMD_ZCL_ONOFF_ON;
	else if (func == 2)
		codeFunc = ZBHCI_CMD_ZCL_ONOFF_TOGGLE;
	else
	{
		LOGW("ZCLOnoffDevice func not match: %d", func);
		return CODE_ERROR;
	}
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
	} zcl_onoff_t;
	zcl_onoff_t zcl_onoff;
	zcl_onoff.dstAddrMode = ZIGBEE_ADDRESS_MODE_SHORT;
	zcl_onoff.dstAddr = bswap_16(devAddr);
	zcl_onoff.srcEp = 0x01;
	zcl_onoff.dstEp = epId;

	int rs = SendMessage(codeFunc, (uint8_t *)&zcl_onoff, 5, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("ZCLOnoffDevice ok");
	}
	else
	{
		LOGE("Send ZCLOnoffDevice error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ZCLLevelControlDevice(uint16_t devAddr, uint8_t epId, uint8_t func, uint16_t level, uint8_t mode, uint8_t rate, uint8_t stepSize)
{
	LOGV("ZCLLevelControlDevice");
	typedef struct __attribute__((packed))
	{
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint8_t data[8];
	} zcl_level_t;
	zcl_level_t zcl_level;
	zcl_level.dstAddrMode = ZIGBEE_ADDRESS_MODE_SHORT;
	zcl_level.dstAddr = bswap_16(devAddr);
	zcl_level.srcEp = 0x01;
	zcl_level.dstEp = epId;

	uint16_t codeFunc;
	int len = 0;
	if (func == 0)
	{
		codeFunc = ZBHCI_CMD_ZCL_LEVEL_MOVE2LEVEL;
		zcl_level.data[0] = level;
		zcl_level.data[1] = 0;
		zcl_level.data[2] = 1;
		len = 8;
	}
	else if (func == 1)
	{
		codeFunc = ZBHCI_CMD_ZCL_LEVEL_MOVE;
		zcl_level.data[0] = mode;
		zcl_level.data[1] = rate;
		len = 7;
	}
	else if (func == 2)
	{
		codeFunc = ZBHCI_CMD_ZCL_LEVEL_STEP;
		zcl_level.data[0] = mode;
		zcl_level.data[1] = stepSize;
		zcl_level.data[2] = 0;
		zcl_level.data[3] = 1;
		len = 9;
	}
	else if (func == 3)
	{
		codeFunc = ZBHCI_CMD_ZCL_LEVEL_STOP;
	}
	else if (func == 4)
	{
		codeFunc = ZBHCI_CMD_ZCL_LEVEL_MOVE2LEVEL_WITHONOFF;
		zcl_level.data[0] = level;
		zcl_level.data[1] = 0;
		zcl_level.data[2] = 1;
		len = 8;
	}
	else if (func == 5)
	{
		codeFunc = ZBHCI_CMD_ZCL_LEVEL_MOVE_WITHONOFF;
		zcl_level.data[0] = mode;
		zcl_level.data[1] = rate;
		len = 7;
	}
	else if (func == 6)
	{
		codeFunc = ZBHCI_CMD_ZCL_LEVEL_STEP_WITHONOFF;
		zcl_level.data[0] = mode;
		zcl_level.data[1] = stepSize;
		zcl_level.data[2] = 0;
		zcl_level.data[3] = 1;
		len = 9;
	}
	else if (func == 7)
		codeFunc = ZBHCI_CMD_ZCL_LEVEL_STOP_WITHONOFF;
	else
	{
		LOGW("ZCLLevelControlDevice func not match: %d", func);
		return CODE_ERROR;
	}

	int rs = SendMessage(codeFunc, (uint8_t *)&zcl_level, len, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("ZCLLevelControlDevice ok");
	}
	else
	{
		LOGE("Send ZCLLevelControlDevice error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ZCLOnoffGroup(uint16_t groupAddr, uint8_t func)
{
	LOGV("ZCLOnoffGroup groupAddr: 0X%04X, func: %d", groupAddr, func);
	uint16_t codeFunc;
	if (func == 0)
		codeFunc = ZBHCI_CMD_ZCL_ONOFF_OFF;
	else if (func == 1)
		codeFunc = ZBHCI_CMD_ZCL_ONOFF_ON;
	else if (func == 2)
		codeFunc = ZBHCI_CMD_ZCL_ONOFF_TOGGLE;
	else
	{
		LOGW("ZCLOnoffGroup func not match: %d", func);
		return CODE_ERROR;
	}
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
	} zcl_onoff_t;
	zcl_onoff_t zcl_onoff;
	zcl_onoff.dstAddrMode = ZIGBEE_ADDRESS_MODE_GROUP;
	zcl_onoff.dstAddr = bswap_16(groupAddr);
	zcl_onoff.srcEp = 0x01;

	int rs = SendMessage(codeFunc, (uint8_t *)&zcl_onoff, 4, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
	{
		LOGD("ZCLOnoffGroup ok");
	}
	else
	{
		LOGE("Send ZCLOnoffGroup error, rs: %d", rs);
	}
	return rs;
}