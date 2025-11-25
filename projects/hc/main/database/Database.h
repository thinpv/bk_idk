#pragma once
#include <string>
#include <string.h>
// #include <sqlite3.h>
#include <vector>
#include <mutex>
#include "Device.h"
#include "Group.h"
#include "Scene.h"

using namespace std;

typedef struct sqlite3_stmt sqlite3_stmt;

class Database
{

private:
	// sqlite3 *db;
	mutex mtx;

	Database();
	~Database();

	int Sqlite_Exec(string &sql);
	int ReadAll(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *));

public:
	static Database *GetInstance();

	int init(void);
	bool IsHaveDb(const char *dbName);
	int createTableIfNotExists();
	void DelDatabase();

	int Sqlite_BenginTransaction();
	int Sqlite_EndTransaction();

	int GatewayRead();
	int GatewayAdd();
	int GatewayUpdateId();
	int GatewayUpdateNextGroupAddr();
	int GatewayUpdateNextSceneAddr();
	// int GatewayUpdateNetKey(string netkey);
	// int GatewayUpdateAppKey(string appkey);
	// int GatewayUpdateDeviceKey(string devicekey);
	// int GatewayUpdateUnicast(uint16_t unicast);
	// int GatewayUpdateIvIndex(uint32_t iv_index);
	// int GatewayUpdateDormitory(string dormitory);
	// int GatewayUpdateRefreshToken(string refreshToken);
	int GatewayUpdateData();
	int GatewayUpdateVersion();
	int GatewayDel();
	int GatewayDel(string id);
	int GatewayDelAll();

	int DeviceRead();
	int DeviceAdd(Device *device);
	int DeviceUpdateNameAndAddr(Device *device);
	int DeviceUpdateData(Device *device);
	int DeviceDel(Device *device);
	int DeviceDel(string mac);
	int DeviceDelAll();
	int DelDevExist(Device *device);

	int DeviceAttributeRead();
	int DeviceAttributeAdd(Device *device, string attribute, double value);
	int DeviceAttributeUpdate(Device *device, string attribute, double value);
	int DeviceAttributeDel(Device *device, string attribute);
	int DeviceAttributeDelAll();

	int GroupRead();
	int GroupAdd(Group *group);
	int GroupUpdateName(Group *group);
	int GroupDel(Group *group);
	int GroupDel(string id);
	int GroupDelAll();

	int DeviceInGroupRead();
	int DeviceInGroupAdd(DeviceInGroup *deviceInGroup);
	int DeviceInGroupAdd(Device *device, uint16_t epId, uint16_t groupAddr, string data);
	int DeviceInGroupDel(DeviceInGroup *deviceInGroup);
	int DeviceInGroupDel(Device *device, uint16_t epId, uint16_t groupAddr);
	int DeviceInGroupDel(string deviceId, uint16_t epId, uint16_t groupAddr);
	int DeviceInGroupUpdateData(DeviceInGroup *deviceInGroup);
	int DeviceInGroupDelDev(string deviceId);
	int DeviceInGroupDelDev(Device *device);
	int DeviceInGroupDelAll();

	int SceneRead();
	int SceneAdd(Scene *scene);
	int SceneUpdate(Scene *scene);
	int SceneDel(Scene *scene);
	int SceneDel(string id);
	int SceneDelAll();

	int DeviceInSceneRead();
	int DeviceInSceneAdd(DeviceInScene *deviceInScene);
	int DeviceInSceneAdd(Device *device, uint16_t epId, uint16_t sceneAddr, string data);
	int DeviceInSceneDel(DeviceInScene *deviceInScene);
	int DeviceInSceneDel(Device *device, uint16_t epId, uint16_t sceneAddr);
	int DeviceInSceneDel(string deviceId, uint16_t epId, uint16_t sceneAddr);
	int DeviceInSceneUpdateData(DeviceInScene *deviceInScene);
	int DeviceInSceneDelDev(string deviceId);
	int DeviceInSceneDelDev(Device *device);
	int DeviceInSceneDelAll();

	int RuleRead();
	int RuleAdd(Rule *rule, string data, int type);
	int RuleUpdateData(Rule *rule, string data);
	int RuleUpdateStatus(Rule *rule);
	int RuleUpdateAddr(Rule *rule);
	int RuleUpdateType(Rule *rule, int type);
	int RuleUpdateFirstRun(Rule *rule, bool isFirstRun);
	int RuleDel(Rule *rule);
	int RuleDel(string id);
	int RuleDelAll();
};
