#include "Database.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"

#define TABLE_NAME "[Scene]"

static int SceneParse(sqlite3_stmt *stmt, void *ptr)
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

	// 			Scene *scene = new Scene(id, name, addr, updatedAt);
	// 			if (scene)
	// 			{
	// 				SceneManager::GetInstance()->AddScene(scene);
	// 			}
	// 		}
	// 		else if (s == SQLITE_DONE)
	// 		{
	// 			return CODE_OK;
	// 		}
	// 		else
	// 		{
	// 			LOGE("SceneParse");
	// 			return CODE_ERROR;
	// 		}
	// 	}
	// }
	return CODE_OK;
}

int Database::SceneRead()
{
	return ReadAll(TABLE_NAME, NULL, SceneParse);
}

int Database::SceneAdd(Scene *scene)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (id, name, addr, data, updated_at) VALUES ('" +
							 scene->getId() + "','" +															 // id
							 scene->getName() + "'," +														 // name
							 to_string(scene->getAddr()) + ",'" +									 // addr
							 macaron::Base64::Encode(scene->getDataStr()) + "'," + // data
							 to_string(scene->getUpdatedAt()) + ");";							 // updated_at
	return Sqlite_Exec(sql);
}

int Database::SceneUpdate(Scene *scene)
{
	string sql = "UPDATE " TABLE_NAME " SET name='" + scene->getName() + "',updated_at=" + to_string(scene->getUpdatedAt()) + " WHERE id= '" + scene->getId() + "';";
	return Sqlite_Exec(sql);
}

int Database::SceneDel(Scene *scene)
{
	return SceneDel(scene->getId());
}

int Database::SceneDel(string id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id = '" + id + "';";
	return Sqlite_Exec(sql);
}

int Database::SceneDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
