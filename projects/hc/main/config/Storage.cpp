#include "Storage.h"
#include <stdio.h>
#include <string.h>
// #include <sys/unistd.h>
// #include <sys/stat.h>
// #include <dirent.h>
// #include "esp_err.h"
// #include "esp_spiffs.h"
// #include "define_esp32.h"
#include "Log.h"
// #include "RuleManager.h"
#include "ErrorCode.h"
// #include "common.h"

const static char *TAG = "Storage";

Storage *Storage::GetInstance()
{
	static Storage *storage = NULL;
	if (!storage)
	{
		storage = new Storage();
	}
	return storage;
}

Storage::Storage()
{
}

int Storage::init()
{
	// esp_vfs_spiffs_conf_t conf = {
	// 		.base_path = "/storage",
	// 		.partition_label = NULL,
	// 		/*Maximum files that could be open at the same time.*/
	// 		.max_files = 2,
	// 		.format_if_mount_failed = true};
	// /*Register and mount SPIFFS to VFS with given path prefix.*/
	// esp_err_t ret = esp_vfs_spiffs_register(&conf);
	// if (ret != ESP_OK)
	// {
	// 	if (ret == ESP_FAIL)
	// 	{
	// 		LOGE("Failed to mount or format filesystem");
	// 	}
	// 	else if (ret == ESP_ERR_NOT_FOUND)
	// 	{
	// 		LOGE("Failed to find SPIFFS partition");
	// 	}
	// 	else
	// 	{
	// 		LOGE("Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
	// 	}
	// 	return ESP_FAIL;
	// }

	// size_t total = 0, used = 0;
	// ret = esp_spiffs_info(NULL, &total, &used);
	// if (ret != ESP_OK)
	// {
	// 	LOGE("Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
	// }
	// else
	// {
	// 	LOGI("Partition size: total: %d, used: %d", total, used);
	// }
	// return ret;
	return 1;
}

int Storage::ReadFile(const char *filePath, char **data, int *len)
{
	LOGD("ReadFile %s", filePath);
	// if (*data != NULL)
	// 	return CODE_ERROR;
	// FILE *f = fopen(filePath, "r");
	// if (f == NULL)
	// {
	// 	LOGE("Failed to open file for reading");
	// 	return ESP_FAIL;
	// }
	// fseek(f, 0L, SEEK_END);
	// *len = ftell(f);
	// fseek(f, 0L, SEEK_SET);
	// *data = (char *)malloc((*len) + 1);
	// if (!(*data))
	// {
	// 	return ESP_FAIL;
	// }
	// *len = fread(*data, sizeof(char), *len, f);
	// (*data)[*len] = '\0';
	// fclose(f);
	// return ESP_OK;
	return 1;
}

int Storage::WriteFile(const char *filePath, const char *data)
{
	LOGD("WriteFile %s", filePath);
	// // Check if destination file exists before renaming
	// struct stat st;
	// if (stat(filePath, &st) == 0)
	// {
	// 	// Delete it if it exists
	// 	unlink(filePath);
	// }

	// FILE *f = fopen(filePath, "w");
	// if (f == NULL)
	// {
	// 	LOGE("Failed to open file for writing");
	// 	return ESP_FAIL;
	// }
	// fprintf(f, data);
	// fclose(f);
	// return ESP_OK;
	return 1;
}

int Storage::DelFile(const char *filePath)
{
	LOGD("DelFile %s", filePath);
	// Check if destination file exists before renaming
	// struct stat st;
	// if (stat(filePath, &st) == 0)
	// {
	// 	// Delete it if it exists
	// 	unlink(filePath);
	// 	return CODE_OK;
	// }
	return CODE_OK;
}

int Storage::ReadAllRule()
{
	LOGD("ReadAllRule");
	// // Open the rule directory
	// DIR *dir = opendir("/storage/rule");
	// if (dir == NULL)
	// {
	// 	LOGE("Failed to open directory");
	// 	return CODE_ERROR;
	// }

	// struct dirent *entry;
	// char filePath[300];
	// while ((entry = readdir(dir)) != NULL)
	// {
	// 	char *data = NULL;
	// 	int len = 0;
	// 	sprintf(filePath, "/storage/rule/%s", entry->d_name);
	// 	if (ReadFile(filePath, &data, &len) == CODE_OK)
	// 	{
	// 		Json::Value payloadJson;
	// 		if (payloadJson.parse(data) && payloadJson.isObject())
	// 		{
	// 			// RuleManager::GetInstance()->AddRule(payloadJson);
	// 		}
	// 		free(data);
	// 	}
	// }

	// // Close the directory
	// closedir(dir);
	return CODE_OK;
}

int Storage::ReadRule(int id, Json::Value &payloadJson)
{
	LOGD("ReadRule");
	char *data = NULL;
	int len = 0;
	char filePath[64];
	sprintf(filePath, "/storage/rule/rule_%d", id);
	if (ReadFile(filePath, &data, &len) == CODE_OK)
	{
		payloadJson.parse(data);
		free(data);
	}
	return CODE_OK;
}

int Storage::ReadRule(int id, char **data, int *len)
{
	LOGD("ReadRule");
	char filePath[64];
	sprintf(filePath, "/storage/rule/rule_%d", id);
	return ReadFile(filePath, data, len);
}

int Storage::WriteRule(int id, const char *data)
{
	LOGD("WriteRule");
	char filePath[64];
	sprintf(filePath, "/storage/rule/rule_%d", id);
	return Storage::GetInstance()->WriteFile(filePath, data);
}

int Storage::DelRule(int id)
{
	LOGD("DelRule");
	char filePath[64];
	sprintf(filePath, "/storage/rule/rule_%d", id);
	return Storage::GetInstance()->DelFile(filePath);
}

void Storage::DelAllFile()
{
	LOGI("SPIFFS mounted successfully, formatting...");
	// esp_spiffs_format(NULL); // Format SPIFFS
	// LOGI("SPIFFS formatted successfully");
	// esp_vfs_spiffs_unregister(NULL); // Unmount SPIFFS
}