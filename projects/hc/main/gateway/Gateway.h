#pragma once

#include <string>
#include <map>
#include <functional>
#include <thread>
#include <mutex>
#include "json.h"
#include "Define.h"
#include "ErrorCode.h"
#include "CloudProtocol.h"
#include "GatewayInfo.h"
#include "Device.h"
#include "Group.h"
#include "Device.h"
#include "Scene.h"
#include "RuleInputTimer.h"
#include "RuleOutputScene.h"
#include "RuleOutputDevice.h"
#include "RuleOutputGroup.h"
#include "RuleOutputDelay.h"
#include "Util.h"
#include "DeviceManager.h"
#include "GroupManager.h"
#include "RuleManager.h"
#include "SceneManager.h"

#ifdef __ANDROID__
#include "Noti.h"
#endif

#ifdef CONFIG_ENABLE_BLE
#include "DeviceBle.h"
#include "BleDefine.h"
#include "BleProtocol.h"
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
#include "DeviceZigbee.h"
#endif

using namespace std;

class Gateway : public CloudProtocol, public GatewayInfo
{
private:
	atomic<bool> isAutoOta;

	mutex mtxDelGroup;
	mutex mtxDelScene;
	mutex mtxDelRule;

	// check module Ble alive
	time_t lastTimePingGwBle;

	Gateway(string mac, string address, int port, const char *clientId, string username, string password, int keepalive, bool ssl);
	~Gateway();

	void OnConnect(int rc);
	void OnDisconnect(int rc);
	void OnSubscribeDone();
	void CheckDataSync();

	void CheckSharedDeviceAttrite(Json::Value &payloadJson, bool updateParams) override;
	bool FirmwareCheckNeedUpdate(string fwVersion) override;
	int FirmwareUpdate(string fwUrl, string fwChecksumAlgorithm, string fwChecksum) override;
	int FirmwareUpdate(int fwSize, string fwChecksumAlgorithm, string fwChecksum) override;
	int FirmwareUpdateChunk(int chunkId, uint8_t *data, int dataLen) override;
	int FirmwareUpdateFinish() override;

	// Gateway
	void InitRpc();
	int OnGatewayDeviceDeleted(Json::Value &reqValue, Json::Value &respValue);
	int OnGatewayControl(Json::Value &reqValue, Json::Value &respValue);
	int OnGatewayControlDevice(Json::Value &reqValue, Json::Value &respValue);
	int OnGatewayControlGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnGatewayControlScene(Json::Value &reqValue, Json::Value &respValue);
	int OnGatewayControlRule(Json::Value &reqValue, Json::Value &respValue);

	// RPC remote shell
	int OnGetTermInfo(Json::Value &reqValue, Json::Value &respValue);
	int OnSendCommand(Json::Value &reqValue, Json::Value &respValue);
	int OnGetCommandStatus(Json::Value &reqValue, Json::Value &respValue);
	int OnTerminateCommand(Json::Value &reqValue, Json::Value &respValue);

	// Device
	void InitRpcDevice();
	int OnDeviceControl(string &deviceId, Json::Value &reqValue, Json::Value &respValue);
	int OnDeviceRestart(string &deviceId, Json::Value &reqValue, Json::Value &respValue);

	// int OnControlDevice(Json::Value &reqValue, Json::Value &respValue);
	// int OnControlAllDevice(Json::Value &reqValue, Json::Value &respValue);
	// int OnGetDeviceStatus(Json::Value &reqValue, Json::Value &respValue);
	// int OnGetAllDeviceStatus(Json::Value &reqValue, Json::Value &respValue);
	// int OnGetDeviceList(Json::Value &reqValue, Json::Value &respValue);
	// int OnGetCamList(Json::Value &reqValue, Json::Value &respValue);
	// int OnGetAllCam(Json::Value &reqValue, Json::Value &respValue);
	// int OnNewDevice(Json::Value &reqValue, Json::Value &respValue);
	// int OnDeleteDevice(Json::Value &reqValue, Json::Value &respValue);
	// int OnUpdateDeviceName(Json::Value &reqValue, Json::Value &respValue);

	// int OnCreateSwitchLink(Json::Value &reqValue, Json::Value &respValue);
	// int OnAddBtToSwitchLink(Json::Value &reqValue, Json::Value &respValue);
	// int OnDelBtFromSwitchLink(Json::Value &reqValue, Json::Value &respValue);
	// int OnDelSwitchLink(Json::Value &reqValue, Json::Value &respValue);

	// int OnDevicesSync(Json::Value &reqValue, Json::Value &respValue);

	// Group
	void InitRpcGroup();
	int OnControlGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnGetGroupList(Json::Value &reqValue, Json::Value &respValue);
	int OnGetDevListInGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnAddDeviceToGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteDeviceFromGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteGroup(Json::Value &reqValue, Json::Value &respValue);
	int OnUpdateGroupName(Json::Value &reqValue, Json::Value &respValue);
	int OnGroupsSync(Json::Value &reqValue, Json::Value &respValue);

	// Rule
	void InitRpcRule();
	int OnGetRuleList(Json::Value &reqValue, Json::Value &respValue);
	int OnGetRuleInfo(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateRule(Json::Value &reqValue, Json::Value &respValue);
	int OnEditRule(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteRule(Json::Value &reqValue, Json::Value &respValue);
	int OnActiveRule(Json::Value &reqValue, Json::Value &respValue);
	int OnActionRule(Json::Value &reqValue, Json::Value &respValue);
	int OnActionRuleCloud(Json::Value &reqValue, Json::Value &respValue);
	int OnRulesSync(Json::Value &reqValue, Json::Value &respValue);

	// Scene
	void InitRpcScene();
	int OnActiveScene(Json::Value &reqValue, Json::Value &respValue);
	int OnControlScene(Json::Value &reqValue, Json::Value &respValue);
	int OnGetSceneList(Json::Value &reqValue, Json::Value &respValue);
	int OnGetDevListInScene(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateScene(Json::Value &reqValue, Json::Value &respValue);
	int OnEditScene(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteScene(Json::Value &reqValue, Json::Value &respValue);
	int OnCallScene(Json::Value &reqValue, Json::Value &respValue);
	int OnAddDevToScene(Json::Value &reqValue, Json::Value &respValue);
	int OnDelDevToScene(Json::Value &reqValue, Json::Value &respValue);

	int ConfigSceneForRemote(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene);
	int ConfigSceneForPirSensor(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene);
	int ConfigSceneForScreenTouch(Device *device, Json::Value &data, Json::Value &scene, bool isAddScene);
	int OnCreateSceneController(Json::Value &reqValue, Json::Value &respValue);
	int OnDelSceneController(Json::Value &reqValue, Json::Value &respValue);
	int OnUpdateSceneName(Json::Value &reqValue, Json::Value &respValue);
	int OnScenesSync(Json::Value &reqValue, Json::Value &respValue);

	// Hc
	void InitRpcHc();
	int OnUdpHcConnectCloud(Json::Value &reqValue, Json::Value &respValue);
	int OnRegisterHc(Json::Value &reqValue, Json::Value &respValue);
	int OnControlHc(Json::Value &reqValue, Json::Value &respValue);
	int OnGetHcInfo(Json::Value &reqValue, Json::Value &respValue);
	int OnStartScan(Json::Value &reqValue, Json::Value &respValue);
	int OnStopScan(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcBleStartScanPairDev(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcBleStopScanPairDev(Json::Value &reqValue, Json::Value &respValue);
	int OnResetHC(Json::Value &reqValue, Json::Value &respValue);
	int OnVersionHC(Json::Value &reqValue, Json::Value &respValue);
	int OnSSHRemote(Json::Value &reqValue, Json::Value &respValue);
	int OnCreateTunnel(Json::Value &reqValue, Json::Value &respValue);
	int OnDeleteAllTunnel(Json::Value &reqValue, Json::Value &respValue);
	int OnOtaHc(Json::Value &reqValue, Json::Value &respValue);
	int OnSetPasswordMqtt(Json::Value &reqValue, Json::Value &respValue);
	int OnAutoOta(Json::Value &reqValue, Json::Value &respValue);
	int OnBackupData(Json::Value &reqValue, Json::Value &respValue);
	int OnRestoreData(Json::Value &reqValue, Json::Value &respValue);

	// int OnDelMatterFabric(Json::Value &reqValue, Json::Value &respValue);

	// // Matter
	// int OnBridgeGetMatterCommission(Json::Value &reqValue, Json::Value &respValue);
	// int OnBridgeGetMatterFabric(Json::Value &reqValue, Json::Value &respValue);

public:
	static Gateway *GetInstance();
	void init();

	/**
	 * @brief Factory reset (call when hold reset button in 5s)
	 *
	 */
	void ResetFactory();

	int RestartBleGw();
	int CheckOnlineThread();

	bool getAutoOta();
	void setAutoOta(bool isAutoOta);

	void CheckGroupSync();
	void OnTimerTest();
	void PushRelayState(uint8_t relay);

	int Do(Json::Value &dataValue);

	int pushDeviceUpdateCloud(Json::Value &dataValue);
	int pushNewDeviceCloud(Json::Value &dataValue);
	// int pushStartAddHc(Json::Value &dataValue);
	// int pushStopAddHc(Json::Value &dataValue);
	int pushNotify(Json::Value &dataValue);

	time_t getLastTimePingGwBle();
	void setLastTimePingGwBle(time_t timeUpdate);

	uint32_t getNextAndIncreaseGroupAddr();
	uint32_t getNextAndIncreaseSceneAddr();
};
