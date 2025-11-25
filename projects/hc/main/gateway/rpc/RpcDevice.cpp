#include "Gateway.h"
#include "Log.h"

void Gateway::InitRpcDevice()
{
	OnDeviceRpcCallbackRegister("control", bind(&Gateway::OnDeviceControl, this, placeholders::_1, placeholders::_2, placeholders::_3));
	OnDeviceRpcCallbackRegister("restart", bind(&Gateway::OnDeviceRestart, this, placeholders::_1, placeholders::_2, placeholders::_3));
}

int Gateway::OnDeviceControl(string &deviceId, Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnDeviceControl device id: %s", deviceId.c_str());
	Device *device = DeviceManager::GetInstance()->GetDeviceFromId(deviceId);
	if (device)
	{
		int rs = device->Do(reqValue);
		respValue["params"]["code"] = rs;
	}
	else
	{
		respValue["params"]["code"] = CODE_NOT_FOUND;
	}
	return CODE_OK;
}

int Gateway::OnDeviceRestart(string &deviceId, Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("OnDeviceRestart device id: %s", deviceId.c_str());
	respValue["params"]["code"] = CODE_NOT_FOUND;
	return CODE_OK;
}
