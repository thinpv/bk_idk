// #include "Database.h"
// #include "Log.h"
// #include "Util.h"
// #include "Base64.h"

// #define TABLE_NAME "[Gateway]"

// // TODO: insert autoUpdate

// static int GatewayParse(sqlite3_stmt *stmt, void *ptr)
// {
// 	// int s, index;
// 	// if (stmt)
// 	// {
// 	// 	while (1)
// 	// 	{
// 	// 		s = sqlite3_step(stmt);
// 	// 		if (s == SQLITE_ROW)
// 	// 		{
// 	// 			index = 0;
// 	// 			string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
// 	// 			string mac = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
// 	// 			string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
// 	// 			string version = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
// 	// 			uint32_t next_group_addr = sqlite3_column_int(stmt, index++);
// 	// 			uint32_t next_scene_addr = sqlite3_column_int(stmt, index++);
// 	// 			string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
// 	// 			uint64_t updatedAt = sqlite3_column_int64(stmt, index++);

// 	// 			// Gateway::GetInstance()->setId(id);
// 	// 			// Gateway::GetInstance()->setMac(mac);
// 	// 			Gateway::GetInstance()->setName(name);
// 	// 			Gateway::GetInstance()->setVersion(version);
// 	// 			Gateway::GetInstance()->setNextGroupAddr(next_group_addr);
// 	// 			Gateway::GetInstance()->setNextSceneAddr(next_scene_addr);
// 	// 			Gateway::GetInstance()->setData(data);

// 	// 			LOGI("Gateway mac: %s, id: %s, name: %s, version: %s, ble_netkey: %s,ble_appkey: %s, ble_devicekey: %s, ble_addr: %d, ble_iv_index: %d, latitude: %s, longitude: %s, data: %s",
// 	// 					 Gateway::GetInstance()->getMac().c_str(),
// 	// 					 Gateway::GetInstance()->getId().c_str(),
// 	// 					 Gateway::GetInstance()->getName().c_str(),
// 	// 					 Gateway::GetInstance()->getVersion().c_str(),
// 	// 					 Gateway::GetInstance()->getBleNetKey().c_str(),
// 	// 					 Gateway::GetInstance()->getBleAppKey().c_str(),
// 	// 					 Gateway::GetInstance()->getBleDeviceKey().c_str(),
// 	// 					 Gateway::GetInstance()->getBleAddr(),
// 	// 					 Gateway::GetInstance()->getBleIvIndex(),
// 	// 					 Gateway::GetInstance()->getLatitude().c_str(),
// 	// 					 Gateway::GetInstance()->getLongitude().c_str(),
// 	// 					 Gateway::GetInstance()->getData().toString().c_str());

// 	// 			return CODE_OK;
// 	// 		}
// 	// 		else if (s == SQLITE_DONE)
// 	// 		{
// 	// 			return CODE_OK;
// 	// 		}
// 	// 		else
// 	// 		{
// 	// 			LOGE("GatewayParse");
// 	// 			return CODE_ERROR;
// 	// 		}
// 	// 	}
// 	// }
// 	return CODE_OK;
// }

// int Database::GatewayRead()
// {
// 	return ReadAll(TABLE_NAME, NULL, GatewayParse);
// }

// int Database::GatewayAdd()
// {
// 	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (id, mac, name, version, next_group_addr, next_scene_addr, data, updated_at) VALUES ('" +
// 							 Gateway::GetInstance()->getId() + "','" +
// 							 Gateway::GetInstance()->getMac() + "','" +
// 							 Gateway::GetInstance()->getName() + "','" +
// 							 Gateway::GetInstance()->getVersion() + "'," +
// 							 to_string(Gateway::GetInstance()->getNextGroupAddr()) + "," +
// 							 to_string(Gateway::GetInstance()->getNextSceneAddr()) + ",'" +
// 							 macaron::Base64::Encode(Gateway::GetInstance()->getDataStr()) + "'," +
// 							 to_string(Gateway::GetInstance()->getUpdatedAt()) + ");";
// 	return Sqlite_Exec(sql);
// }

// int Database::GatewayUpdateId()
// {
// 	string sql = "UPDATE " TABLE_NAME " SET id = '" + Gateway::GetInstance()->getId() + "' WHERE mac = '" + Gateway::GetInstance()->getMac() + "';";
// 	return Sqlite_Exec(sql);
// }

// int Database::GatewayUpdateNextGroupAddr()
// {
// 	string sql = "UPDATE " TABLE_NAME " SET next_group_addr =" + to_string(Gateway::GetInstance()->getNextGroupAddr()) + " WHERE mac='" + Gateway::GetInstance()->getMac() + "';";
// 	return Sqlite_Exec(sql);
// }

// int Database::GatewayUpdateNextSceneAddr()
// {
// 	string sql = "UPDATE " TABLE_NAME " SET next_scene_addr =" + to_string(Gateway::GetInstance()->getNextSceneAddr()) + " WHERE mac='" + Gateway::GetInstance()->getMac() + "';";
// 	return Sqlite_Exec(sql);
// }

// int Database::GatewayUpdateData()
// {
// 	string sql = "UPDATE " TABLE_NAME " SET data='" + macaron::Base64::Encode(Gateway::GetInstance()->getDataStr()) + "' WHERE mac='" + Gateway::GetInstance()->getMac() + "';";
// 	return Sqlite_Exec(sql);
// }

// int Database::GatewayUpdateVersion()
// {
// 	string sql = "UPDATE " TABLE_NAME " SET version='" + Gateway::GetInstance()->getVersion() + "' WHERE mac='" + Gateway::GetInstance()->getMac() + "';";
// 	return Sqlite_Exec(sql);
// }

// int Database::GatewayDel()
// {
// 	return GatewayDel(Gateway::GetInstance()->getMac());
// }

// int Database::GatewayDel(string mac)
// {
// 	string sql = "DELETE FROM " TABLE_NAME " WHERE mac='" + mac + "';";
// 	return Sqlite_Exec(sql);
// }

// int Database::GatewayDelAll()
// {
// 	string sql = "DELETE FROM " TABLE_NAME ";";
// 	return Sqlite_Exec(sql);
// }
