#include "Group.h"
#include <thread>
#include <algorithm>
#include "Log.h"
#include "Database.h"
#include "Base64.h"

#ifdef CONFIG_ENABLE_BLE
#include "BleProtocol.h"
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#endif

void DeviceInGroup::setIsConfigured(bool value)
{
	isConfigured = value;
	dataValue["isConfigured"] = value;
}

void DeviceInGroup::setIsInGroup(bool value)
{
	isInGroup = value;
	dataValue["isInGroup"] = value;
}

void DeviceInGroup::setRetryCount(int value)
{
	retryCount = value;
	dataValue["retryCount"] = value;
}

void DeviceInGroup::setData(const string &value)
{
	string dataStr;
	string decode = macaron::Base64::Decode(value, dataStr);
	if (decode == "")
	{
		dataValue.parse(dataStr);
	}
	else
	{
		dataValue.parse(value);
	}

	if (!dataValue.isObject())
	{
		dataValue = Json::objectValue;
	}
	parseDataValue(dataValue);
}

void DeviceInGroup::parseDataValue(Json::Value &dataValue)
{
	if (dataValue.isObject())
	{
		if (dataValue.isMember("isConfigured") && dataValue["isConfigured"].isBool())
		{
			isConfigured = dataValue["isConfigured"].asBool();
		}

		if (dataValue.isMember("isInGroup") && dataValue["isInGroup"].isBool())
		{
			isInGroup = dataValue["isInGroup"].asBool();
		}

		if (dataValue.isMember("retryCount") && dataValue["retryCount"].isInt())
		{
			retryCount = dataValue["retryCount"].asInt();
		}
	}
	LOGD("DeviceInGroup: %s, epId: %d, groupAddr: %d isConfigured: %d, isInGroup: %d, retryCount: %d",
			 device->getName().c_str(),
			 epId,
			 groupAddr,
			 isConfigured,
			 isInGroup,
			 retryCount);
}

Group::Group(string id, string name, uint16_t addr, uint64_t updatedAt)
		: Object(id, name, addr, updatedAt)
{
	onoff = 0;
}

Group::Group(string id, string name, uint16_t addr)
		: Object(id, name, addr, time(NULL))
{
	onoff = 0;
	waitingToCheck = false;
}

Group::~Group()
{
	deviceListMtx.lock();
	LOGI("Delete all devices in group %s", name.c_str());
	for (auto it = deviceList.begin(); it != deviceList.end();)
	{
		DeleteDeviceInGroup(*it, true);
		it = deviceList.erase(it);
	}
	deviceList.clear();
	deviceListMtx.unlock();
}

size_t Group::countDevice()
{
	deviceListMtx.lock();
	size_t size = deviceList.size();
	deviceListMtx.unlock();
	return size;
}

DeviceInGroup *Group::GetDeviceInGroup(Device *device, uint16_t epId)
{
	deviceListMtx.lock();
	for (auto &deviceInGroup : deviceList)
	{
		if (deviceInGroup->device == device && deviceInGroup->epId == epId)
		{
			deviceListMtx.unlock();
			return deviceInGroup;
		}
	}
	deviceListMtx.unlock();
	return NULL;
}

int Group::GetPositionDevice(Device *device, uint16_t epId)
{
	string deviceId = device->getId();
	deviceListMtx.lock();
	for (uint32_t i = 0; i < deviceList.size(); i++)
	{
		if ((deviceId == deviceList[i]->device->getId()) && (deviceList[i]->epId == epId))
		{
			deviceListMtx.unlock();
			return i;
		}
	}
	deviceListMtx.unlock();
	return -1;
}

DeviceInGroup *Group::AddDeviceInGroup(DeviceInGroup *deviceInGroup)
{
	if (deviceInGroup &&
			GetDeviceInGroup(deviceInGroup->device, deviceInGroup->epId) == NULL)
	{
		deviceListMtx.lock();
		deviceList.push_back(deviceInGroup);
		deviceListMtx.unlock();
	}
	return deviceInGroup;
}

DeviceInGroup *Group::AddDeviceAndConfig(Device *device, uint16_t epId)
{
	if (device)
	{
		DeviceInGroup *deviceInGroup = GetDeviceInGroup(device, epId);
		if (deviceInGroup == NULL)
		{
			deviceInGroup = new DeviceInGroup(device, epId, addr, true, false);
			if (deviceInGroup)
			{
				deviceListMtx.lock();
				deviceList.push_back(deviceInGroup);
				deviceListMtx.unlock();

				Database::GetInstance()->DeviceInGroupAdd(deviceInGroup);
				device->AddDeviceInGroupUnconfig(deviceInGroup);
				device->AddToGroup(epId, addr); // update DB in callback
			}
		}
		else
		{
			if (deviceInGroup->getIsConfigured() == false)
			{
				device->AddToGroup(epId, addr); // update DB in callback
			}
		}
		return deviceInGroup;
	}
	return NULL;
}

int Group::DelDevice(Device *device, uint16_t epId)
{
	if (device)
	{
		int deviceIndex = GetPositionDevice(device, epId);
		if (deviceIndex >= 0)
		{
			deviceListMtx.lock();
			deviceList.erase(deviceList.begin() + deviceIndex);
			deviceListMtx.unlock();
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Group::DelDeviceAndConfig(Device *device, uint16_t epId)
{
	if (device)
	{
		DeviceInGroup *deviceInGroup = GetDeviceInGroup(device, epId);
		if (deviceInGroup)
		{
			DeleteDeviceInGroup(deviceInGroup, true);

			deviceListMtx.lock();
			deviceList.erase(remove(deviceList.begin(), deviceList.end(), deviceInGroup), deviceList.end());
			deviceListMtx.unlock();
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Group::DeleteDeviceInGroup(DeviceInGroup *deviceInGroup, bool needConfig)
{
	if (needConfig)
	{
		if (deviceInGroup->getIsConfigured())
		{
			deviceInGroup->setIsInGroup(false);
			Database::GetInstance()->DeviceInGroupUpdateData(deviceInGroup);
			deviceInGroup->device->AddDeviceInGroupUnconfig(deviceInGroup);
			deviceInGroup->device->RemoveFromGroup(deviceInGroup->epId, addr); // update DB in callback
		}
	}
	else
	{
		Database::GetInstance()->DeviceInGroupDel(deviceInGroup);
		delete deviceInGroup;
	}
	return CODE_OK;
}

int Group::RemoveDevice(Device *device, bool needConfig)
{
	deviceListMtx.lock();
	for (auto it = deviceList.begin(); it != deviceList.end();)
	{
		DeviceInGroup *deviceInGroup = *it;
		if (deviceInGroup->device == device)
		{
			DeleteDeviceInGroup(deviceInGroup, needConfig);
			it = deviceList.erase(it);
		}
		else
		{
			++it;
		}
	}
	deviceListMtx.unlock();
	return CODE_OK;
}

// đưa tất cả thiết bị trong nhóm vào trạng thái chờ kiểm tra, nếu không còn thuộc nhóm thì phải xóa khỏi danh sách
void Group::StartCheckDeviceInGroup()
{
	deviceListMtx.lock();
	for (auto &deviceInGroup : deviceList)
	{
		deviceInGroup->setWaitingToCheck(true);
	}
	deviceListMtx.unlock();
}

// xóa tất cả các thiết bị không còn thuộc nhóm
void Group::StopCheckDeviceInGroup()
{
	deviceListMtx.lock();
	for (auto it = deviceList.begin(); it != deviceList.end();)
	{
		DeviceInGroup *deviceInGroup = *it;
		if (deviceInGroup->getWaitingToCheck())
		{
			LOGI("Device %s with epId %d is not in group %s, remove it", deviceInGroup->device->getId().c_str(), deviceInGroup->epId, getId().c_str());
			DeleteDeviceInGroup(deviceInGroup, true);
			it = deviceList.erase(it);
		}
		else
		{
			++it;
		}
	}
	deviceListMtx.unlock();
}

int Group::Do(Json::Value &dataValue, bool ack)
{
	if (dataValue.isObject())
	{
		if (dataValue.isMember(KEY_ATTRIBUTE_ONOFF) && dataValue[KEY_ATTRIBUTE_ONOFF].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_ONOFF].asInt();
#ifdef CONFIG_ENABLE_BLE
			if (value == 2)
			{
				++onoff;
				onoff = onoff % 2;
				BleProtocol::GetInstance()->SetOnOffLight(addr + BLE_GROUP_OFFSET, onoff, TRANSITION_DEFAULT, ack);
			}
			else
			{
				BleProtocol::GetInstance()->SetOnOffLight(addr + BLE_GROUP_OFFSET, value, TRANSITION_DEFAULT, ack);
			}
#endif
#ifdef CONFIG_ENABLE_ZIGBEE
			ZigbeeProtocol::GetInstance()->ZCLOnoffGroup(addr, value);
#endif
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_DIM) && dataValue[KEY_ATTRIBUTE_DIM].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_DIM].asInt();
			uint16_t dim = (value * 65535) / 100;
#ifdef CONFIG_ENABLE_BLE
			BleProtocol::GetInstance()->SetDimmingLight(addr + BLE_GROUP_OFFSET, dim, TRANSITION_DEFAULT, ack);
#endif
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_CCT) && dataValue[KEY_ATTRIBUTE_CCT].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_CCT].asInt();
			uint16_t cct = (value * 192) + 800;
#ifdef CONFIG_ENABLE_BLE
			BleProtocol::GetInstance()->SetCctLight(addr + BLE_GROUP_OFFSET, cct, TRANSITION_DEFAULT, ack);
#endif
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_HUE) && dataValue[KEY_ATTRIBUTE_HUE].isInt() &&
				dataValue.isMember(KEY_ATTRIBUTE_SATURATION) && dataValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
				dataValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && dataValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
		{
			int h = dataValue[KEY_ATTRIBUTE_HUE].asInt();
			int s = dataValue[KEY_ATTRIBUTE_SATURATION].asInt();
			int l = dataValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
#ifdef CONFIG_ENABLE_BLE
			BleProtocol::GetInstance()->SetHSLLight(addr + BLE_GROUP_OFFSET, h, s, l, TRANSITION_DEFAULT, ack);
#endif
		}
		if (dataValue.isMember(KEY_ATTRIBUTE_MODE_RGB) && dataValue[KEY_ATTRIBUTE_MODE_RGB].isInt())
		{
			int value = dataValue[KEY_ATTRIBUTE_MODE_RGB].asInt();
#ifdef CONFIG_ENABLE_BLE
			BleProtocol::GetInstance()->CallModeRgb(addr + BLE_GROUP_OFFSET, value);
#endif
		}
	}
	return CODE_OK;
}
