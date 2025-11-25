#include "Database.h"
#include "Log.h"
#include "Util.h"
#include "DeviceManager.h"
#include "Base64.h"
#include <time.h>

#define TABLE_NAME "[DeviceInGroup]"

static int DeviceInGroupParse(sqlite3_stmt *stmt, void *ptr)
{
	// int s, index;
	// if (stmt)
	// {
	// 	while (1)
	// 	{
	// 		s = sqlite3_step(stmt);
	// 		if (s == SQLITE_ROW)
	// 		{
	// 			index = 0;
	// 			string deviceId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			uint16_t epId = sqlite3_column_int(stmt, index++);
	// 			uint16_t groupAddr = sqlite3_column_int(stmt, index++);
	// 			string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			uint64_t updatedAt = sqlite3_column_int64(stmt, index++);
	// 			LOGD("device: %s, groupAddr: %d, epId: %d", deviceId.c_str(), groupAddr, epId);

	// 			Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
	// 			if (device)
	// 			{
	// 				DeviceInGroup *deviceInGroup = new DeviceInGroup(device, epId, groupAddr);
	// 				if (deviceInGroup)
	// 				{
	// 					deviceInGroup->setData(data);

	// 					if ((deviceInGroup->getIsInGroup() && !deviceInGroup->getIsConfigured()) ||
	// 							(!deviceInGroup->getIsInGroup() && deviceInGroup->getIsConfigured()))
	// 					{
	// 						device->AddDeviceInGroupUnconfig(deviceInGroup);
	// 					}

	// 					Group *group = GroupManager::GetInstance()->GetGroupFromAddr(groupAddr);
	// 					if (group)
	// 					{
	// 						group->AddDeviceInGroup(deviceInGroup);
	// 					}
	// 					else
	// 					{
	// 						LOGW("group addr %d not found", groupAddr);
	// 						deviceInGroup->setIsInGroup(false);
	// 					}
	// 				}
	// 			}
	// 			else
	// 			{
	// 				LOGW("device %s not found", deviceId.c_str());
	// 			}
	// 		}
	// 		else if (s == SQLITE_DONE)
	// 		{
	// 			return CODE_OK;
	// 		}
	// 		else
	// 		{
	// 			LOGE("DeviceInGroupParse");
	// 			return CODE_ERROR;
	// 		}
	// 	}
	// }
	return CODE_OK;
}

int Database::DeviceInGroupRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceInGroupParse);
}

int Database::DeviceInGroupAdd(DeviceInGroup *deviceInGroup)
{
	return DeviceInGroupAdd(deviceInGroup->device, deviceInGroup->epId, deviceInGroup->groupAddr, deviceInGroup->getDataStr());
}

int Database::DeviceInGroupAdd(Device *device, uint16_t epId, uint16_t groupAddr, string data)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (device_id,ep_id,group_addr,data,updated_at) VALUES ('" +
							 device->getId() + "'," +								// device_id
							 to_string(epId) + "," +								// ep_id
							 to_string(groupAddr) + ",'" +					// group_addr
							 macaron::Base64::Encode(data) + "'," + // data
							 to_string(time(NULL)) + ");";					// updated_at
	return Sqlite_Exec(sql);
}

int Database::DeviceInGroupDel(DeviceInGroup *deviceInGroup)
{
	return DeviceInGroupDel(deviceInGroup->device, deviceInGroup->epId, deviceInGroup->groupAddr);
}

int Database::DeviceInGroupDel(Device *device, uint16_t epId, uint16_t groupAddr)
{
	return DeviceInGroupDel(device->getId(), epId, groupAddr);
}

int Database::DeviceInGroupDel(string deviceId, uint16_t epId, uint16_t groupAddr)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE device_id='" + deviceId + "' AND group_addr=" + to_string(groupAddr) + " AND ep_id=" + to_string(epId) + ";";
	return Sqlite_Exec(sql);
}

int Database::DeviceInGroupUpdateData(DeviceInGroup *deviceInGroup)
{
	string sql = "UPDATE " TABLE_NAME " SET data='" + macaron::Base64::Encode(deviceInGroup->getDataStr()) + "' WHERE device_id='" + deviceInGroup->device->getId() + "' AND group_addr=" + to_string(deviceInGroup->groupAddr) + " AND ep_id=" + to_string(deviceInGroup->epId) + ";";
	return Sqlite_Exec(sql);
}

int Database::DeviceInGroupDelDev(string deviceId)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE device_id='" + deviceId + "';";
	return Sqlite_Exec(sql);
}

int Database::DeviceInGroupDelDev(Device *device)
{
	return DeviceInGroupDelDev(device->getId());
}

int Database::DeviceInGroupDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
