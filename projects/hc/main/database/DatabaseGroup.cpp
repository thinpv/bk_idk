#include "Database.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"

#define TABLE_NAME "[Group]"

static int GroupParse(sqlite3_stmt *stmt, void *ptr)
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
	// 			string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			uint16_t addr = sqlite3_column_int(stmt, index++);
	// 			string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
	// 			uint64_t updatedAt = sqlite3_column_int64(stmt, index++);
	// 			LOGD("%s, %d, %s", id.c_str(), addr, name.c_str());

	// 			Group *group = new Group(id, name, addr, updatedAt);
	// 			if (group)
	// 			{
	// 				GroupManager::GetInstance()->AddGroup(group);
	// 			}
	// 		}
	// 		else if (s == SQLITE_DONE)
	// 		{
	// 			return CODE_OK;
	// 		}
	// 		else
	// 		{
	// 			LOGE("GroupParse");
	// 			return CODE_ERROR;
	// 		}
	// 	}
	// }
	return CODE_OK;
}

int Database::GroupRead()
{
	return ReadAll(TABLE_NAME, NULL, GroupParse);
}

int Database::GroupAdd(Group *group)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (id, name, addr, data, updated_at) VALUES ('" +
							 group->getId() + "','" +															 // id
							 group->getName() + "'," +														 // name
							 to_string(group->getAddr()) + ",'" +									 // addr
							 macaron::Base64::Encode(group->getDataStr()) + "'," + // data
							 to_string(group->getUpdatedAt()) + ");";							 // updated_at
	return Sqlite_Exec(sql);
}

int Database::GroupUpdateName(Group *group)
{
	string sql = "UPDATE " TABLE_NAME " SET name='" + group->getName() + "',updated_at=" + to_string(group->getUpdatedAt()) + " WHERE id= '" + group->getId() + "';";
	return Sqlite_Exec(sql);
}

int Database::GroupDel(Group *group)
{
	return GroupDel(group->getId());
}

int Database::GroupDel(string id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id = '" + id + "';";
	return Sqlite_Exec(sql);
}

int Database::GroupDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
