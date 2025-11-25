#include "Database.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"

#define TABLE_NAME "[Device]"

static int DeviceParse(sqlite3_stmt *stmt, void *ptr)
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
	// 			string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			string mac = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			uint16_t addr = sqlite3_column_int(stmt, index++);
	// 			uint32_t type = sqlite3_column_int(stmt, index++);
	// 			uint16_t firmwareVersion = sqlite3_column_int(stmt, index++);
	// 			uint16_t hardwareVersion = sqlite3_column_int(stmt, index++);
	// 			uint32_t activeTime = sqlite3_column_int(stmt, index++);
	// 			string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			uint32_t updateAt = sqlite3_column_int(stmt, index++);
	// 			LOGD("%s, %s, %s, 0X%04X, 0X%04X, %d, %d, %d, %d, %s", mac.c_str(), id.c_str(), name.c_str(), addr, type, firmwareVersion, hardwareVersion, activeTime, updateAt, data.c_str());

	// 			string devData;
	// 			string decode = macaron::Base64::Decode(data, devData);
	// 			Json::Value dataJson;
	// 			dataJson.parse(devData);
	// 			Device *device = DeviceManager::GetInstance()->AddDevice(id, mac, type, addr, firmwareVersion, &dataJson);
	// 		}
	// 		else if (s == SQLITE_DONE)
	// 		{
	// 			return CODE_OK;
	// 		}
	// 		else
	// 		{
	// 			LOGE("DeviceParse");
	// 			return CODE_ERROR;
	// 		}
	// 	}
	// }
	return CODE_OK;
}

int Database::DeviceRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceParse);
}

int Database::DeviceAdd(Device *device)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (id, mac, name, addr, type, sw_ver, hw_ver, active_at, data, updated_at) VALUES ('" +
							 device->getId() + "','" +																			// id
							 device->getMac() + "','" +																			// mac
							 device->getName() + "'," +																			// name
							 to_string(device->getAddr()) + "," +														// addr
							 to_string(device->getType()) + "," +														// type
							 to_string(device->getSwVer()) + "," +													// sw_ver
							 to_string(device->getHwVer()) + "," +													// hw_ver
							 to_string(time(NULL)) + ",'" +																	// active_at
							 macaron::Base64::Encode(device->getData().toString()) + "'," + // data
							 to_string(time(NULL)) + ");";																	// update_at
	return Sqlite_Exec(sql);
}

int Database::DeviceUpdateNameAndAddr(Device *device)
{
	string sql = "UPDATE " TABLE_NAME " SET name='" + device->getName() + "', addr=" + to_string(device->getAddr()) + " WHERE id='" + device->getId() + "';";
	return Sqlite_Exec(sql);
}

int Database::DeviceUpdateData(Device *device)
{
	string sql = "UPDATE " TABLE_NAME " SET data='" + macaron::Base64::Encode(device->getData().toString()) + "' WHERE id='" + device->getId() + "';";
	return Sqlite_Exec(sql);
}

int Database::DeviceDel(Device *device)
{
	return DeviceDel(device->getId());
}

int Database::DeviceDel(string id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id='" + id + "';";
	return Sqlite_Exec(sql);
}

int Database::DeviceDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}

int Database::DelDevExist(Device *device)
{
	string sql = "DELETE FROM DeviceInGroup WHERE id = '" + device->getId() + "';";
	Sqlite_Exec(sql);
	sql = "DELETE FROM DeviceInScene WHERE id = '" + device->getId() + "';";
	Sqlite_Exec(sql);
	sql = "DELETE FROM DeviceAttribute WHERE id = '" + device->getId() + "';";
	Sqlite_Exec(sql);
	return CODE_OK;
}
