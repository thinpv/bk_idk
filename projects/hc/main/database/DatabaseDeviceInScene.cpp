#include "Database.h"
#include "Log.h"
#include "Util.h"
#include "DeviceManager.h"
#include "Base64.h"
#include <time.h>

#define TABLE_NAME "[DeviceInScene]"

static int DeviceInSceneParse(sqlite3_stmt *stmt, void *ptr)
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
	// 			uint16_t sceneAddr = sqlite3_column_int(stmt, index++);
	// 			string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			uint64_t updatedAt = sqlite3_column_int64(stmt, index++);
	// 			LOGD("device: %s, sceneAddr: %d, epId: %d", deviceId.c_str(), sceneAddr, epId);

	// 			Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
	// 			if (device)
	// 			{
	// 				DeviceInScene *deviceInScene = new DeviceInScene(device, epId, sceneAddr);
	// 				if (deviceInScene)
	// 				{
	// 					deviceInScene->setData(data);

	// 					if ((deviceInScene->getIsInScene() && !deviceInScene->getIsConfigured()) ||
	// 							(!deviceInScene->getIsInScene() && deviceInScene->getIsConfigured()))
	// 					{
	// 						device->AddDeviceInSceneUnconfig(deviceInScene);
	// 					}

	// 					Scene *scene = SceneManager::GetInstance()->GetSceneFromAddr(sceneAddr);
	// 					if (scene)
	// 					{
	// 						scene->AddDeviceInScene(deviceInScene);
	// 					}
	// 					else
	// 					{
	// 						LOGW("scene addr %d not found", sceneAddr);
	// 						deviceInScene->setIsInScene(false);
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
	// 			LOGE("DeviceInSceneParse");
	// 			return CODE_ERROR;
	// 		}
	// 	}
	// }
	return CODE_OK;
}

int Database::DeviceInSceneRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceInSceneParse);
}

int Database::DeviceInSceneAdd(DeviceInScene *deviceInScene)
{
	return DeviceInSceneAdd(deviceInScene->device, deviceInScene->epId, deviceInScene->sceneAddr, deviceInScene->getDataStr());
}

int Database::DeviceInSceneAdd(Device *device, uint16_t epId, uint16_t sceneAddr, string data)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (device_id,ep_id,scene_addr,data,updated_at) VALUES ('" +
							 device->getId() + "'," +								// device_id
							 to_string(epId) + "," +								// ep_id
							 to_string(sceneAddr) + ",'" +					// scene_addr
							 macaron::Base64::Encode(data) + "'," + // data
							 to_string(time(NULL)) + ");";					// updated_at
	return Sqlite_Exec(sql);
}

int Database::DeviceInSceneDel(DeviceInScene *deviceInScene)
{
	return DeviceInSceneDel(deviceInScene->device, deviceInScene->epId, deviceInScene->sceneAddr);
}

int Database::DeviceInSceneDel(Device *device, uint16_t epId, uint16_t sceneAddr)
{
	return DeviceInSceneDel(device->getId(), epId, sceneAddr);
}

int Database::DeviceInSceneDel(string deviceId, uint16_t epId, uint16_t sceneAddr)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE device_id='" + deviceId + "' AND scene_addr=" + to_string(sceneAddr) + " AND ep_id=" + to_string(epId) + ";";
	return Sqlite_Exec(sql);
}

int Database::DeviceInSceneUpdateData(DeviceInScene *deviceInScene)
{
	string sql = "UPDATE " TABLE_NAME " SET data='" + macaron::Base64::Encode(deviceInScene->getDataStr()) + "' WHERE device_id='" + deviceInScene->device->getId() + "' AND scene_addr=" + to_string(deviceInScene->sceneAddr) + " AND ep_id=" + to_string(deviceInScene->epId) + ";";
	return Sqlite_Exec(sql);
}

int Database::DeviceInSceneDelDev(string deviceId)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE device_id='" + deviceId + "';";
	return Sqlite_Exec(sql);
}

int Database::DeviceInSceneDelDev(Device *device)
{
	return DeviceInSceneDelDev(device->getId());
}

int Database::DeviceInSceneDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
