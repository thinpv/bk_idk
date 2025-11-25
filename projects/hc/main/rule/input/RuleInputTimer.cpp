#include "RuleInputTimer.h"
#include "Util.h"
#include "TimerSchedule.h"
#include "RuleManager.h"
#include "Log.h"

RuleInputTimer::RuleInputTimer(Rule *rule, int timer, int repeat)
{
	this->rule = rule;
	this->timer = timer;
	this->repeat = repeat;
	timerRegisterIndex = TimerSchedule::GetInstance()->RegisterTimer(timer, bind(&RuleManager::CheckRule, RuleManager::GetInstance(), rule));
}

RuleInputTimer::~RuleInputTimer()
{
	LOGI("~RuleInputTimer");
	if (timerRegisterIndex > 0)
		TimerSchedule::GetInstance()->UnregisterTimer(timerRegisterIndex);
}

bool RuleInputTimer::Check()
{
	if (!Util::HaveRTC())
	{
		LOGW("Not have RTC");
		return false;
	}
	int currentWeekDay = Util::GetCurrentWeekDay();
	LOGI("currentWeekDay : %d", currentWeekDay);
	LOGI("repeat : 0x%02X", repeat);
	// LOGI("%s", rule->GetFirstRun() ? "true" : "false");
	// if (repeat == 0)
	// {
	// 	return (rule->GetFirstRun() | (timer == Util::GetCurrentTimeInDay()));
	// }
	return (Util::CheckDayInWeek(currentWeekDay, repeat) && timer == Util::GetCurrentTimeInDay());
}
