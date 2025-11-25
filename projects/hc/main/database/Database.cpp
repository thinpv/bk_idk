#include "Database.h"
#include "Log.h"
#include "Util.h"
#ifdef ESP_PLATFORM
#include "esp_littlefs.h"
#endif
#include <sys/stat.h>

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

Database *Database::GetInstance()
{
	static Database *database = NULL;
	if (!database)
	{
		database = new Database();
	}
	return database;
}

Database::Database()
{
}

Database::~Database()
{
	// if (db)
	// 	sqlite3_close_v2(db);
}

int Database::init(void)
{
	LOGI("Init db");
	int isCreateDb = false;
#ifdef ESP_PLATFORM
	LOGI("Initializing LITTLEFS");

	esp_vfs_littlefs_conf_t conf = {
			.base_path = "/storage",
			.partition_label = "storage",
			.format_if_mount_failed = true,
			.dont_mount = false,
	};

	// Use settings defined above to initialize and mount LITTLEFS filesystem.
	// Note: esp_vfs_littlefs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_littlefs_register(&conf);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			LOGE("Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			LOGE("Failed to find LITTLEFS partition");
		}
		else
		{
			LOGE("Failed to initialize LITTLEFS (%s)", esp_err_to_name(ret));
		}
		return isCreateDb;
	}

	size_t total = 0, used = 0;
	ret = esp_littlefs_info(conf.partition_label, &total, &used);
	if (ret != ESP_OK)
	{
		LOGE("Failed to get LITTLEFS partition information (%s)", esp_err_to_name(ret));
	}
	else
	{
		LOGI("Partition size: total: %d, used: %d", total, used);
	}

	sqlite3_initialize();

// // All done, unmount partition and disable LITTLEFS
// esp_vfs_littlefs_unregister(conf.partition_label);
// LOGI("LITTLEFS unmounted");
#endif

	// if (!IsHaveDb(DB_NAME))
	// {
	// 	int rc = sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
	// 	if (rc != SQLITE_OK)
	// 	{
	// 		LOGW("Cannot open DB: %s\n", sqlite3_errmsg(db));
	// 	}

	// 	rc = createTableIfNotExists();
	// 	if (rc != SQLITE_OK)
	// 	{
	// 		LOGW("Cannot open DB: %s\n", sqlite3_errmsg(db));
	// 	}
	// 	isCreateDb = true;
	// }
	// else
	// {
	// 	sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
	// }

	return isCreateDb;
}

bool Database::IsHaveDb(const char *dbName)
{
	struct stat st;
	return !stat(dbName, &st);
}

int Database::createTableIfNotExists()
{
	string sql =
			"BEGIN TRANSACTION;"
			"CREATE TABLE IF NOT EXISTS Gateway(id TEXT CHECK(length(id) <= 128),mac TEXT NOT NULL CHECK(length(mac) <= 128),name TEXT CHECK(length(name) <= 256),version TEXT,next_group_addr INTEGER,next_scene_addr INTEGER,data TEXT,updated_at INTEGER,PRIMARY KEY(mac)) WITHOUT ROWID;"
			"CREATE TABLE IF NOT EXISTS Device(id TEXT NOT NULL CHECK(length(id) <= 128),mac TEXT CHECK(length(mac) <= 128),name TEXT CHECK(length(name) <= 256),addr INTEGER,type INTEGER,sw_ver INTEGER,hw_ver INTEGER,active_at INTEGER,data TEXT,updated_at INTEGER,PRIMARY KEY(id)) WITHOUT ROWID;"
			"CREATE TABLE IF NOT EXISTS DeviceAttribute(device_id TEXT NOT NULL CHECK(length(device_id) <= 128),attribute TEXT NOT NULL CHECK(length(attribute) <= 128),value DOUBLE,PRIMARY KEY(device_id,attribute)) WITHOUT ROWID;"
			"CREATE TABLE IF NOT EXISTS[Group](id TEXT NOT NULL CHECK(length(id) <= 128),name TEXT CHECK(length(name) <= 256),addr INTEGER,data TEXT,updated_at INTEGER,PRIMARY KEY(id)) WITHOUT ROWID;"
			"CREATE TABLE IF NOT EXISTS DeviceInGroup(device_id TEXT NOT NULL CHECK(length(device_id) <= 128),ep_id INTEGER,group_addr INTEGER NOT NULL,data TEXT,updated_at INTEGER,PRIMARY KEY(group_addr,ep_id,device_id)) WITHOUT ROWID;"
			"CREATE TABLE IF NOT EXISTS Scene(id TEXT NOT NULL CHECK(length(id) <= 128),name TEXT CHECK(length(name) <= 256),addr INTEGER,data TEXT,updated_at INTEGER,PRIMARY KEY(id)) WITHOUT ROWID;"
			"CREATE TABLE IF NOT EXISTS DeviceInScene(device_id TEXT NOT NULL CHECK(length(device_id) <= 128),ep_id INTEGER,scene_addr INTEGER NOT NULL,data TEXT,updated_at INTEGER,PRIMARY KEY(scene_addr,ep_id,device_id)) WITHOUT ROWID;"
			"CREATE TABLE IF NOT EXISTS Rule(id TEXT NOT NULL CHECK(length(id) <= 128),name TEXT CHECK(length(name) <= 256),type INTEGER,enable BOOLEAN,count INTEGER,data TEXT NOT NULL,updated_at INTEGER,PRIMARY KEY(id)) WITHOUT ROWID;"
			"COMMIT;";
	return Sqlite_Exec(sql);
}

static int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		LOGD("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	LOGD("\n");
	return CODE_OK;
}

int Database::Sqlite_BenginTransaction()
{
	// int rc = SQLITE_ERROR;
	// char *err_msg = 0;
	// mtx.lock();
	// rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
	// if (rc != SQLITE_OK)
	// {
	// 	LOGE("Error executing sql statement :%s", err_msg);
	// 	sqlite3_free(err_msg);
	// }
	// mtx.unlock();
	// return rc;
	return -1;
}

int Database::Sqlite_EndTransaction()
{
	// int rc = SQLITE_ERROR;
	// char *err_msg = 0;
	// mtx.lock();
	// rc = sqlite3_exec(db, "COMMIT", NULL, NULL, &err_msg);
	// if (rc != SQLITE_OK)
	// {
	// 	LOGE("Error executing sql statement :%s", err_msg);
	// 	sqlite3_free(err_msg);
	// }
	// mtx.unlock();
	// return rc;
	return -1;
}

int Database::Sqlite_Exec(string &sql)
{
	LOGD("Sqlite_Exec sql: %s", sql.c_str());
	// int rc = SQLITE_ERROR;
	// char *err_msg = 0;
	// mtx.lock();
	// rc = sqlite3_exec(db, sql.c_str(), sqlite_callback, NULL, &err_msg);
	// if (rc != SQLITE_OK)
	// {
	// 	if (err_msg)
	// 	{
	// 		LOGE("Error executing sql statement: %s", err_msg);
	// 	}
	// 	else
	// 	{
	// 		LOGE("Error executing sql: %s", sql.c_str());
	// 	}
	// 	sqlite3_free(err_msg);
	// }
	// mtx.unlock();
	// return rc;
	return -1;
}

int Database::ReadAll(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *))
{
	// int rc = SQLITE_ERROR;
	// sqlite3_stmt *stmt;
	// string sql = "SELECT * FROM " + table + ";";

	// if (!Parse)
	// {
	// 	LOGW("Parse func NULL");
	// 	return CODE_ERROR;
	// }

	// LOGD("ReadAll table %s", table.c_str());
	// mtx.lock();
	// rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
	// if (rc == SQLITE_OK)
	// {
	// 	LOGD("sqlite3_prepare_v2 successfully");
	// 	Parse(stmt, listPtr);
	// 	sqlite3_finalize(stmt);
	// }
	// else
	// {
	// 	LOGW("SQL error: %d - %s", rc, sqlite3_errmsg(db));
	// }
	// mtx.unlock();
	// SLEEP_MS(10);
	// return rc;
	return -1;
}

void Database::DelDatabase()
{
// 	// TODO: rewrite delete database
// 	// delete database;
// 	sqlite3_close(db);
// #ifdef ESP_PLATFORM
// 	if (unlink(DB_NAME) != 0)
// 	{
// 		LOGE("Failed to delete file\n");
// 	}

// 	// Unmount SPIFFS
// 	// esp_vfs_spiffs_unregister(NULL);
// #else
// 	string rmDb = "rm " DB_NAME;
// 	system(rmDb.c_str());
// #endif
}
