#include "Module.h"
#include "Device.h"
#include "Log.h"
#include <thread>

Module::Module(Device *device, uint16_t addr, uint32_t index)
{
	this->device = device;
	this->addr = addr;
	this->index = index;
}

Module::~Module()
{
}

bool Module::CheckAddr(uint16_t addr)
{
	return this->addr == addr;
}

void Module::CheckTrigger(Json::Value &data)
{
	LOGV("CheckTrigger %s", data.toString().c_str());
	if (!data.isNull())
	{
		bool rs;
		for (auto &ruleInputDevice : device->ruleInputList)
		{
			Json::Value *ruleValue = ruleInputDevice->getData();
			if (data.isObject() && ruleValue->isObject())
			{
				for (auto const &key : data.getMemberNames())
				{
					if (ruleValue->isMember(key))
					{
						rs = false;
						if (CheckData(*ruleInputDevice->getData(), rs))
							ruleInputDevice->Trigger(rs);
					}
				}
			}
			else
			{
				LOGW("Is not object");
			}
		}
	}
}

bool Module::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGW("CheckData not implement");
	return false;
}
