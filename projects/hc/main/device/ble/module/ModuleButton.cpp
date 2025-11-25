#include "ModuleButton.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "Scene.h"

ModuleButton::ModuleButton(Device *device, uint16_t addr, uint32_t index) : Module(device, addr, index)
{
	bt = 0;
	key = KEY_ATTRIBUTE_BUTTON + (index ? to_string(index + 1) : "");
}

ModuleButton::~ModuleButton()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleButton::InitAttribute(string attribute, double value)
{
	if (attribute == key)
		bt = value;
}

void ModuleButton::SaveAttribute()
{
	Database::GetInstance()->DeviceAttributeAdd(device, key, bt);
}
#endif

int ModuleButton::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(key) && dataValue[key].isInt())
	{
		bt = dataValue[key].asInt();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModuleButton::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint8_t btId;
		uint8_t mode;
		uint16_t scene;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == RD_OPCODE_SENSOR_RSP)
	{
		if (data_message->header == RD_HEADER_REMOTE_MODULE_DC_TYPE ||
				data_message->header == RD_HEADER_REMOTE_MODULE_AC_TYPE ||
				data_message->header == RD_HEADER_REMOTE_MUL_RSP_SCENE_ACTIVE)
		{
			if (data_message->btId == index + 1)
			{
				if (bt != data_message->mode)
				{
					bt = data_message->mode;
#ifdef CONFIG_SAVE_ATTRIBUTE
					SaveAttribute();
#endif
				}
				BuildTelemetryValue(jsonValue);
				CheckTrigger(jsonValue);
				// if (data_message->scene > 0)
				// {
				// 	Scene *scene = SceneManager::GetInstance()->GetSceneFromAddr(data_message->scene);
				// 	if (scene)
				// 	{
				// 		for (int i = 0; i < scene->deviceList.size(); i++)
				// 		{
				// 			DeviceBle *dev = (DeviceBle *)scene->deviceList[i]->device;
				// 			if (dev)
				// 			{
				// 				if (scene->deviceList[i]->data.isArray())
				// 				{
				// 					for (Json::ArrayIndex j = 0; j < scene->deviceList[i]->data.size(); j++)
				// 					{
				// 						if (scene->deviceList[i]->data[j].isObject())
				// 						{
				// 							dev->InputData(scene->deviceList[i]->data[j]);
				// 						}
				// 					}
				// 				}
				// 				else if (scene->deviceList[i]->data.isObject())
				// 				{
				// 					dev->InputData(scene->deviceList[i]->data);
				// 				}
				// 			}
				// 			else
				// 			{
				// 				LOGW("DeviceBle error");
				// 			}
				// 		}
				// 	}
				// 	else
				// 		LOGW("Scene not found");
				// }
				return CODE_OK;
			}
		}
	}
	return CODE_ERROR;
}

bool ModuleButton::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(key) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[key].isInt())
		{
			int bt = dataValue[key].asInt();
			rs = Util::CompareNumber(op, this->bt, bt);
			return true;
		}
		else if (dataValue[key].isArray())
		{
			Json::Value listValue = dataValue[key];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int bt1 = listValue[0].asInt();
				int bt2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->bt, bt1, bt2);
				return true;
			}
		}
	}
	return false;
}

void ModuleButton::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[key] = bt;
	device->UpdatePropertyJsonUpdate(jsonValue);
}
