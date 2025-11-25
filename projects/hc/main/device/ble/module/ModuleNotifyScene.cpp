#include "ModuleNotifyScene.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "BleOpCode.h"
#include "Gateway.h"
#include "Scene.h"

ModuleNotifyScene::ModuleNotifyScene(Device *device, uint16_t addr) : Module(device, addr)
{
	idScene = 0;
}

ModuleNotifyScene::~ModuleNotifyScene()
{
}

int ModuleNotifyScene::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	idScene = 0;
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint8_t data[10];
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == RD_OPCODE_SENSOR_RSP)
	{
		if (data_message->header == RD_HEADER_SCREEN_TOUCH_PRESS)
		{
			idScene = data_message->data[1] | (data_message->data[2] << 8);
		}
		// if (idScene > 0)
		// {
		// 	Scene *scene = SceneManager::GetInstance()->GetSceneFromAddr(idScene);
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
	return CODE_ERROR;
}
