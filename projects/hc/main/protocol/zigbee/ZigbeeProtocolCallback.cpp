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

void ZigbeeProtocol::initCallback()
{
	RegisterCmdCallback(ZBHCI_CMD_NODES_DEV_ANNCE_IND, bind(&ZigbeeProtocol::OnDeviceAnnounce, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_ZCL_REPORT_MSG_RCV, bind(&ZigbeeProtocol::OnReportAttribute, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_ZCL_ATTR_READ_RSP, bind(&ZigbeeProtocol::OnReadAttributeResp, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_DATA_CONFIRM, bind(&ZigbeeProtocol::OnDataConfirm, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_NODE_LEAVE_IND, bind(&ZigbeeProtocol::OnDeviceLeave, this, placeholders::_1, placeholders::_2));

	RegisterCmdCallback(ZBHCI_CMD_ZCL_GROUP_ADD_RSP, bind(&ZigbeeProtocol::OnGroupAddDevice, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_ZCL_GROUP_REMOVE_RSP, bind(&ZigbeeProtocol::OnGroupDelDevice, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_ZCL_SCENE_STORE_RSP, bind(&ZigbeeProtocol::OnSceneAddDevice, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_ZCL_SCENE_REMOVE_RSP, bind(&ZigbeeProtocol::OnSceneDelDevice, this, placeholders::_1, placeholders::_2));
}

int ZigbeeProtocol::RegisterCmdCallback(uint16_t type, OnCmdCallbackFunc onCmdCallbackFunc)
{
	LOGD("RegisterCmd type: 0x%04X", type);
	onCmdCallbackFuncList[type] = onCmdCallbackFunc;
	return CODE_OK;
}

int ZigbeeProtocol::OnDeviceAnnounce(uint8_t *buff, uint16_t len)
{
	LOGI("OnDeviceAnnounce");
	if (len != sizeof(DeviceAnnounce_st))
	{
		LOGW("DeviceAnnounce format error");
		return CODE_ERROR;
	}
	DeviceAnnounce_st *deviceAnnounce = (DeviceAnnounce_st *)buff;
	memcpy(&this->deviceAnnounce, deviceAnnounce, sizeof(DeviceAnnounce_st));
	uint16_t devAddr = bswap_16(deviceAnnounce->nwkAddr);
	LOGI("OnDeviceAnnounce len: %d, nwkAddr: 0x%04X, capability: 0x%02X", len, devAddr, deviceAnnounce->capability);
	// string mac = Util::ConvertU32ToHexString(deviceAnnounce->ieeeAddr, sizeof(deviceAnnounce->ieeeAddr));
	// DiscoveryActiveEndpoint(devAddr);
	ReadAttribute(devAddr);
	return CODE_OK;
}

int ZigbeeProtocol::OnReportAttribute(uint8_t *buff, uint16_t len)
{
	LOGV("OnReportAttribute");
	if (len >= 8)
	{
		ZCLCmdRspHdr_st *zclCmdRspHdr = (ZCLCmdRspHdr_st *)buff;
		uint16_t srcAddr = bswap_16(zclCmdRspHdr->srcAddr);
		LOGD("srcAddr: 0x%04X, srcEp: %d, dstEp: %d, seqNum: %d", srcAddr, zclCmdRspHdr->srcEp, zclCmdRspHdr->dstEp, zclCmdRspHdr->seqNum);
		DeviceZigbee *deviceZigbee = DeviceManager::GetInstance()->getDeviceZigbeeFromAddr(srcAddr);
		if (deviceZigbee)
		{
			deviceZigbee->InputData(buff + 5, len - 5, zclCmdRspHdr->srcEp);
		}
		else
		{
			LOGW("Zigbee device 0x%04X not found", srcAddr);
			if (bswap_16(deviceAnnounce.nwkAddr) == srcAddr)
			{
				string mac = Util::ConvertU32ToHexString(deviceAnnounce.ieeeAddr, sizeof(deviceAnnounce.ieeeAddr));
				LOGI("Announce device %s update info", mac.c_str());

				int clusterLen = len - sizeof(ZCLCmdRspHdr_st);
				typedef struct __attribute__((packed))
				{
					uint16_t clusterId;
					uint8_t attrNum;
					uint8_t attrList[];
				} ClusterMessage_st;
				ClusterMessage_st *clusterMessage = (ClusterMessage_st *)(buff + sizeof(ZCLCmdRspHdr_st));

				int attrLen = clusterLen - sizeof(ClusterMessage_st);
				typedef struct __attribute__((packed))
				{
					uint16_t attrID;
					uint8_t dataType;
					uint8_t data[];
				} AttrMessage_st;
				AttrMessage_st *attrMessage = NULL;

				if (clusterLen > sizeof(ClusterMessage_st))
				{
					uint16_t clusterId = bswap_16(clusterMessage->clusterId);
					LOGD("clusterMessage->clusterId: 0x%04X, clusterMessage->attrNum: %d", clusterId, clusterMessage->attrNum);
					uint8_t *attrData = clusterMessage->attrList;
					if (clusterId == ZCL_CLUSTER_GEN_BASIC)
					{
						uint16_t attrID = 0;
						string model = "";
						uint8_t appVersion = 0, zclVersion = 0;
						for (int i = 0; i < clusterMessage->attrNum; i++)
						{
							attrMessage = (AttrMessage_st *)attrData;
							attrID = bswap_16(attrMessage->attrID);
							LOGD("Attribute ID: 0x%04X", attrID);
							if (attrID == ATTRIBUTE_BASIC_ApplicationVersion)
							{
								if (attrMessage->dataType == ZCL_DATA_TYPE_UINT8)
								{
									appVersion = attrMessage->data[0];
									LOGD("appVersion: %d", appVersion);
								}
							}
							else if (attrID == ATTRIBUTE_BASIC_ModelIdentifier)
							{
								if (attrMessage->dataType == ZCL_DATA_TYPE_CHAR_STR)
								{
									for (int i = 0; i < attrMessage->data[0]; i++)
									{
										model += attrMessage->data[i + 1];
									}
									LOGD("model: %s", model.c_str());
								}
							}
							else
							{
								LOGW("Report Attribute not handle attribute id: 0x%04X", attrID);
							}
							int dataSize = getSizeOfDataType(&attrMessage->dataType);
							attrData += sizeof(AttrMessage_st) + dataSize;
							attrLen -= sizeof(AttrMessage_st) + dataSize;
						}

						if (model != "")
						{
							uint32_t type = Device::ConvertModelToDeviceType(model.c_str());
							LOGI("Announce device %s model: %s, type: 0X%08X", mac.c_str(), model.c_str(), type);
							if (type != DEVICE_UNKNOWN_TYPE)
							{
								Device *device = DeviceManager::GetInstance()->AddDevice(mac, type, srcAddr, zclVersion | appVersion << 8);
								if (device)
								{
									Database::GetInstance()->DeviceAdd(device);
									Gateway::GetInstance()->DeviceAddNew(device);
								}
								memset(&this->deviceAnnounce, 0, sizeof(DeviceAnnounce_st));
							}
						}
						return CODE_OK;
					}
					else
					{
						LOGW("Report Attribute not handle cluster id: 0x%04X", clusterId);
					}
				}
			}
		}
		return CODE_OK;
	}
	else
	{
		LOGW("OnReportAttribute format error");
	}
	return CODE_ERROR;
}

int ZigbeeProtocol::OnReadAttributeResp(uint8_t *buff, uint16_t len)
{
	LOGD("OnReadAttributeResp");
	if (len >= 8)
	{
		ZCLCmdRspHdr_st *zclCmdRspHdr = (ZCLCmdRspHdr_st *)buff;
		uint16_t srcAddr = bswap_16(zclCmdRspHdr->srcAddr);
		LOGD("srcAddr: 0x%04X, srcEp: %d, dstEp: %d, seqNum: %d", srcAddr, zclCmdRspHdr->srcEp, zclCmdRspHdr->dstEp, zclCmdRspHdr->seqNum);
		DeviceZigbee *deviceZigbee = DeviceManager::GetInstance()->getDeviceZigbeeFromAddr(srcAddr);
		if (deviceZigbee)
		{
			// TODO: update device info
			// deviceZigbee->InputData(buff + 5, len - 5);
		}
		else
		{
			LOGW("Zigbee device 0x%04X not found", srcAddr);
			if (bswap_16(deviceAnnounce.nwkAddr) == srcAddr)
			{
				string mac = Util::ConvertU32ToHexString(deviceAnnounce.ieeeAddr, sizeof(deviceAnnounce.ieeeAddr));
				LOGI("Announce device %s update info", mac.c_str());

				int clusterLen = len - sizeof(ZCLCmdRspHdr_st);
				typedef struct __attribute__((packed))
				{
					uint16_t clusterId;
					uint8_t attrNum;
					uint8_t attrList[];
				} ClusterMessage_st;
				ClusterMessage_st *clusterMessage = (ClusterMessage_st *)(buff + sizeof(ZCLCmdRspHdr_st));

				int attrLen = clusterLen - sizeof(ClusterMessage_st);
				typedef struct __attribute__((packed))
				{
					uint16_t attrID;
					uint8_t status;
					uint8_t dataType;
					uint8_t data[];
				} AttrMessage_st;
				AttrMessage_st *attrMessage = NULL;

				if (clusterLen > sizeof(ClusterMessage_st))
				{
					uint16_t clusterId = bswap_16(clusterMessage->clusterId);
					LOGD("clusterMessage->clusterId: 0x%04X, clusterMessage->attrNum: %d", clusterId, clusterMessage->attrNum);
					uint8_t *attrData = clusterMessage->attrList;
					if (clusterId == ZCL_CLUSTER_GEN_BASIC)
					{
						uint16_t attrID = 0;
						uint8_t zclVersion = 0;
						uint8_t appVersion = 0;
						string manufacturerName = "";
						string model = "";
						uint8_t powerSource = 0;
						for (int i = 0; i < clusterMessage->attrNum; i++)
						{
							attrMessage = (AttrMessage_st *)attrData;
							attrID = bswap_16(attrMessage->attrID);
							LOGD("Attribute ID: 0x%04X", bswap_16(attrID));
							if (attrMessage->status == ZIGBEE_SUCCESS)
							{
								if (attrID == ATTRIBUTE_BASIC_ZCLVersion)
								{
									if (attrMessage->dataType == ZCL_DATA_TYPE_UINT8)
									{
										zclVersion = attrMessage->data[0];
										LOGD("zclVersion: %d", zclVersion);
									}
								}
								else if (attrID == ATTRIBUTE_BASIC_ApplicationVersion)
								{
									if (attrMessage->dataType == ZCL_DATA_TYPE_UINT8)
									{
										appVersion = attrMessage->data[0];
										LOGD("appVersion: %d", appVersion);
									}
								}
								else if (attrID == ATTRIBUTE_BASIC_ManufacturerName)
								{
									if (attrMessage->dataType == ZCL_DATA_TYPE_CHAR_STR)
									{
										for (int j = 1; j <= attrMessage->data[0]; j++)
										{
											manufacturerName += attrMessage->data[j];
										}
										LOGD("manufacturerName: %s", manufacturerName.c_str());
									}
								}
								else if (attrID == ATTRIBUTE_BASIC_ModelIdentifier)
								{
									if (attrMessage->dataType == ZCL_DATA_TYPE_CHAR_STR)
									{
										for (int i = 0; i < attrMessage->data[0]; i++)
										{
											model += attrMessage->data[i + 1];
										}
										LOGD("model: %s", model.c_str());
									}
								}
								else if (attrID == ATTRIBUTE_BASIC_PowerSource)
								{
									if (attrMessage->dataType == ZCL_DATA_TYPE_ENUM8)
									{
										powerSource = attrMessage->data[0];
										LOGD("powerSource: %d", powerSource);
									}
								}
								else
								{
									LOGW("Read Attribute Response not handle attribute id: 0x%04X", attrID);
								}
							}
							else
							{
								LOGW("Read Attribute response status err: %d, id: 0x%04X", attrMessage->status, attrID);
							}
							int dataSize = getSizeOfDataType(&attrMessage->dataType);
							attrData += sizeof(AttrMessage_st) + dataSize;
							attrLen -= sizeof(AttrMessage_st) + dataSize;
						}

						if (model != "")
						{
							uint32_t type = Device::ConvertModelToDeviceType(model.c_str());
							LOGI("Announce device %s model: %s, type: 0X%08X", mac.c_str(), model.c_str(), type);
							if (type != DEVICE_UNKNOWN_TYPE)
							{
								Device *device = DeviceManager::GetInstance()->AddDevice(mac, type, srcAddr, zclVersion | appVersion << 8);
								if (device)
								{
									Database::GetInstance()->DeviceAdd(device);
									Gateway::GetInstance()->DeviceAddNew(device);
								}
								else
								{
									LOGW("Add Device error");
								}
								memset(&this->deviceAnnounce, 0, sizeof(DeviceAnnounce_st));
							}
							else
							{
								LOGW("type not supported");
							}
						}
						return CODE_OK;
					}
					else
					{
						LOGW("Read Attribute Response not handle cluster id: 0x%04X", clusterId);
					}
				}
			}
		}
		return CODE_OK;
	}
	else
	{
		LOGW("OnReportAttribute format error");
	}
	return CODE_ERROR;
}

int ZigbeeProtocol::OnDataConfirm(uint8_t *buff, uint16_t len)
{
	LOGV("OnDataConfirm");
	if (len != sizeof(DataConfirm_st))
	{
		LOGW("DataConfirm format error");
		return CODE_ERROR;
	}
	DataConfirm_st *dataConfirm = (DataConfirm_st *)buff;
	if (dataConfirm->dstAddrMode == ZIGBEE_ADDRESS_MODE_SHORT)
	{
		uint16_t devAddr = bswap_16(dataConfirm->dstAddr);
		DeviceZigbee *deviceZigbee = DeviceManager::GetInstance()->getDeviceZigbeeFromAddr(devAddr);
		if (deviceZigbee)
		{
			Gateway::GetInstance()->DeviceAddNew(deviceZigbee);
		}
		else
			LOGW("Zigbee device 0x%04X not found", devAddr);
	}
	return CODE_OK;
}

int ZigbeeProtocol::OnDeviceLeave(uint8_t *buff, uint16_t len)
{
	LOGI("OnDeviceLeave");
	if (len != sizeof(DeviceLeave_st))
	{
		LOGW("DeviceLeave format error");
		return CODE_ERROR;
	}
	DeviceLeave_st *deviceLeave = (DeviceLeave_st *)buff;
	string mac = Util::ConvertU32ToHexString(deviceLeave->extAddr, sizeof(deviceLeave->extAddr));
	LOGI("OnDeviceLeave mac: %s", mac.c_str());
	Device *device = DeviceManager::GetInstance()->GetDeviceFromMac(mac);
	if (device)
	{
		Gateway::GetInstance()->DeviceDisconnect(device);
		GroupManager::GetInstance()->ForEach(
				[&](Group *group)
				{
					group->RemoveDevice(device, false);
				});
		SceneManager::GetInstance()->ForEach(
				[&](Scene *scene)
				{
					scene->RemoveDevice(device, false);
				});
		DeviceManager::GetInstance()->RemoveDevice(device);
	}
	else
	{
		LOGE("Leave device: %s not found", mac.c_str());
	}
	return CODE_OK;
}

int ZigbeeProtocol::OnGroupAddDevice(uint8_t *buff, uint16_t len)
{
	LOGI("OnGroupAddDevice");
	typedef struct __attribute__((packed))
	{
		ZCLCmdRspHdr_st zclCmdRspHdr;
		uint8_t status;
		uint16_t groupId;
	} GroupAddDevice_t;
	GroupAddDevice_t *groupAddDevice = (GroupAddDevice_t *)buff;
	LOGW("srcAddr: 0x%04X, srcEp: 0x%02X, dstEp: 0x%02X, groupId: 0x%04X",
			 bswap_16(groupAddDevice->zclCmdRspHdr.srcAddr),
			 groupAddDevice->zclCmdRspHdr.srcEp, groupAddDevice->zclCmdRspHdr.dstEp,
			 bswap_16(groupAddDevice->groupId));
	if (groupAddDevice)
	{
		DeviceZigbee *device = DeviceManager::GetInstance()->getDeviceZigbeeFromAddr(bswap_16(groupAddDevice->zclCmdRspHdr.srcAddr));
		if (device)
		{
			LOGW("device: %s", device->getName().c_str());
			device->OnAddToGroup(groupAddDevice->zclCmdRspHdr.srcEp,
													 bswap_16(groupAddDevice->groupId),
													 groupAddDevice->status);
		}
	}
	return CODE_OK;
}

int ZigbeeProtocol::OnGroupDelDevice(uint8_t *buff, uint16_t len)
{
	LOGI("OnGroupDelDevice");
	typedef struct __attribute__((packed))
	{
		ZCLCmdRspHdr_st zclCmdRspHdr;
		uint8_t status;
		uint16_t groupId;
	} GroupDelDevice_t;
	GroupDelDevice_t *groupDelDevice = (GroupDelDevice_t *)buff;
	LOGW("srcAddr: 0x%04X, srcEp: 0x%02X, dstEp: 0x%02X, groupId: 0x%04X",
			 bswap_16(groupDelDevice->zclCmdRspHdr.srcAddr),
			 groupDelDevice->zclCmdRspHdr.srcEp,
			 groupDelDevice->zclCmdRspHdr.dstEp,
			 bswap_16(groupDelDevice->groupId));
	if (groupDelDevice)
	{
		DeviceZigbee *device = DeviceManager::GetInstance()->getDeviceZigbeeFromAddr(bswap_16(groupDelDevice->zclCmdRspHdr.srcAddr));
		if (device)
		{
			LOGW("device: %s", device->getName().c_str());
			device->OnRemoveFromGroup(groupDelDevice->zclCmdRspHdr.srcEp,
																bswap_16(groupDelDevice->groupId),
																groupDelDevice->status);
		}
	}
	return CODE_OK;
}

int ZigbeeProtocol::OnSceneAddDevice(uint8_t *buff, uint16_t len)
{
	LOGI("OnSceneAddDevice");
	typedef struct __attribute__((packed))
	{
		ZCLCmdRspHdr_st zclCmdRspHdr;
		uint8_t status;
		uint16_t groupId;
		uint8_t sceneId;
	} SceneAddDevice_t;
	SceneAddDevice_t *sceneAddDevice = (SceneAddDevice_t *)buff;
	LOGW("srcAddr: 0x%04X, srcEp: 0x%02X, dstEp: 0x%02X, groupId: 0x%04X, sceneId: 0x%02X",
			 bswap_16(sceneAddDevice->zclCmdRspHdr.srcAddr),
			 sceneAddDevice->zclCmdRspHdr.srcEp, sceneAddDevice->zclCmdRspHdr.dstEp,
			 bswap_16(sceneAddDevice->groupId),
			 sceneAddDevice->sceneId);
	if (sceneAddDevice)
	{
		DeviceZigbee *device = DeviceManager::GetInstance()->getDeviceZigbeeFromAddr(bswap_16(sceneAddDevice->zclCmdRspHdr.srcAddr));
		if (device)
		{
			LOGW("device: %s", device->getName().c_str());
			device->OnAddToScene(sceneAddDevice->zclCmdRspHdr.srcEp,
													 //  bswap_16(sceneAddDevice->groupId),
													 sceneAddDevice->sceneId,
													 sceneAddDevice->status);
		}
	}
	return CODE_OK;
}

int ZigbeeProtocol::OnSceneDelDevice(uint8_t *buff, uint16_t len)
{
	LOGI("OnSceneDelDevice");
	typedef struct __attribute__((packed))
	{
		ZCLCmdRspHdr_st zclCmdRspHdr;
		uint8_t status;
		uint16_t groupId;
		uint8_t sceneId;
	} SceneDelDevice_t;
	SceneDelDevice_t *sceneDelDevice = (SceneDelDevice_t *)buff;
	LOGW("srcAddr: 0x%04X, srcEp: 0x%02X, dstEp: 0x%02X, groupId: 0x%04X, sceneId: 0x%02X",
			 bswap_16(sceneDelDevice->zclCmdRspHdr.srcAddr),
			 sceneDelDevice->zclCmdRspHdr.srcEp, sceneDelDevice->zclCmdRspHdr.dstEp,
			 bswap_16(sceneDelDevice->groupId),
			 sceneDelDevice->sceneId);
	if (sceneDelDevice)
	{
		DeviceZigbee *device = DeviceManager::GetInstance()->getDeviceZigbeeFromAddr(bswap_16(sceneDelDevice->zclCmdRspHdr.srcAddr));
		if (device)
		{
			LOGW("device: %s", device->getName().c_str());
			device->OnRemoveFromScene(sceneDelDevice->zclCmdRspHdr.srcEp,
																//  bswap_16(sceneDelDevice->groupId),
																sceneDelDevice->sceneId,
																sceneDelDevice->status);
		}
	}
	return CODE_OK;
}
