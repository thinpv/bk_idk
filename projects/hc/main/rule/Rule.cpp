#include "Rule.h"
#include <functional>
#include "TimerSchedule.h"
#include "Util.h"
#include "Log.h"
#include "Database.h"
#ifdef ESP_PLATFORM
#include "Sntp.h"
#include "app_task.h"
#endif

Rule::Rule(string id,
					 string name,
					 uint16_t addr,
					 uint64_t updatedAt,
					 bool enable,
					 string type,
					 unsigned char repeater,
					 Json::Value &ruleData) : Object(id, name, addr, updatedAt),
																		type(type),
																		enable(enable),
																		repeater(repeater),
																		ruleData(ruleData)
{
	this->startTime = -1;
	this->endTime = -1;
	// isEnable = true;
	timerRegisterIndex = 0;
}

Rule::Rule(string id, string type, unsigned char repeater, string name, uint16_t addr, Json::Value &ruleData)
		: Object(id, name, addr, time(NULL))
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = -1;
	this->endTime = -1;
	this->ruleData = ruleData;
	// isEnable = true;
	enable = true;
	timerRegisterIndex = 0;
}

Rule::Rule(string id, string type, unsigned char repeater, string name, uint16_t addr, int startTime, int endTime, Json::Value &ruleData)
		: Object(id, name, addr, time(NULL))
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = startTime;
	this->endTime = endTime;
	this->ruleData = ruleData;
	// isEnable = true;
	enable = true;
	timerRegisterIndex = TimerSchedule::GetInstance()->RegisterTimer(startTime, bind(&Rule::Check, this));
}

Rule::~Rule()
{
	LOGI("~Rule");
	for (auto &ruleInput : ruleInputList)
	{
		delete ruleInput;
	}
	for (auto &ruleOutput : ruleOutputList)
	{
		delete ruleOutput;
	}
	if (timerRegisterIndex > 0)
		TimerSchedule::GetInstance()->UnregisterTimer(timerRegisterIndex);
}

Json::Value Rule::GetRuleData()
{
	return ruleData;
}

void Rule::SetRuleData(Json::Value ruleData)
{
	this->ruleData = ruleData;
}

string Rule::getType()
{
	return type;
}

void Rule::Check()
{
	if (enable)
	{
		bool checkRuleInputResult = false;
		int currentTimer = Util::GetCurrentTimeInDay();
		int currentWeekDay = Util::GetCurrentWeekDay();
		if (Util::CheckDayInWeek(currentWeekDay, repeater))
		{
			LOGD("Check repeater day OK");
			LOGD("current: %d", currentTimer);
			LOGD("start: %d", startTime);
			LOGD("end: %d", endTime);
			if ((startTime < 0) ||																																											// fullDay
					(Util::HaveRTC() && ((startTime <= currentTimer && currentTimer <= endTime) ||													// bắt đầu và kết thúc trong cùng 1 ngày
															 (endTime < startTime && (startTime <= currentTimer || currentTimer <= endTime))))) // bắt đầu và kết thúc trong 2 ngày khác nhau
			{
				LOGD("Check time OK");
				if (type == RULE_TYPE_OR || type == RULE_TYPE_TIME_OR || type == RULE_TYPE_TIME)
				{
					checkRuleInputResult = false;
					for (auto &ruleInput : ruleInputList)
					{
						if (ruleInput->Check())
						{
							checkRuleInputResult = true;
							break;
						}
					}
				}
				else if (type == RULE_TYPE_AND || type == RULE_TYPE_TIME_AND)
				{
					checkRuleInputResult = true;
					for (auto &ruleInput : ruleInputList)
					{
						if (ruleInput->Check() == false)
						{
							checkRuleInputResult = false;
							break;
						}
					}
				}
			}
		}
		if (checkRuleInputResult)
		{
			LOGI("Do output rule id: %s", id.c_str());
			Do();
			// if (isFirstRun)
			// {
			// 	UpdateFirstRun();
			// }
		}
	}
}

void Rule::RunOutput()
{
	// doMtx.lock();
	if (doMtx.try_lock())
	{
		count++;
		for (auto &ruleOutput : ruleOutputList)
		{
			ruleOutput->RunOutput();
		}
		doMtx.unlock();
	}
}

void Rule::Do()
{

#ifdef ESP_PLATFORM
	if (!app_new_task([](void *arg)
							 {
	Rule *rule = (Rule *)arg;
	rule->RunOutput(); },			 // Task function
							 "ruleDo", // Task name
							 5120,		 // Stack size (words, not bytes)
							 this,		 // Param
							 5				 // Priority
	))
	{
		LOGE("Failed to create ruleDo task");
	}
#else
	// thread t(&Rule::RunOutput, this);
	// t.detach();
#endif
}

void Rule::AddRuleInput(RuleInput *ruleInput)
{
	ruleInputList.push_back(ruleInput);
}

void Rule::AddRuleOutput(RuleOutput *ruleOutput)
{
	ruleOutputList.push_back(ruleOutput);
}

void Rule::DelAllRuleInput()
{
	for (auto ruleInput : ruleInputList)
		delete ruleInput;
	ruleInputList.clear();
}

void Rule::DelAllRuleOutput()
{
	for (auto ruleOutput : ruleOutputList)
		delete ruleOutput;
	ruleOutputList.clear();
}

void Rule::UpdateData(string type, unsigned char repeater, string name, uint16_t addr, Json::Value &ruleData)
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = -1;
	this->endTime = -1;
	this->ruleData = ruleData;
	if (timerRegisterIndex)
	{
		TimerSchedule::GetInstance()->UnregisterTimer(timerRegisterIndex);
	}
	timerRegisterIndex = 0;
}

void Rule::UpdateData(string type, unsigned char repeater, string name, uint16_t addr, int startTime, int endTime, Json::Value &ruleData)
{
	this->type = type;
	this->repeater = repeater;
	this->startTime = startTime;
	this->endTime = endTime;
	this->ruleData = ruleData;
	if (timerRegisterIndex)
	{
		TimerSchedule::GetInstance()->UnregisterTimer(timerRegisterIndex);
	}
	timerRegisterIndex = TimerSchedule::GetInstance()->RegisterTimer(startTime, bind(&Rule::Check, this));
}

void Rule::UpdateData(Json::Value &ruleData)
{
	this->ruleData = ruleData;
}

// bool Rule::GetStatus()
// {
// 	return this->isEnable;
// }

// void Rule::SetStatus(bool enable)
// {
// 	this->isEnable = enable;
// }

// bool Rule::GetFirstRun()
// {
// 	return this->isFirstRun;
// }

// void Rule::SetFirstRun(bool isFirstRun)
// {
// 	this->isFirstRun = isFirstRun;
// }
