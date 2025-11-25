#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "Object.h"
#include "RuleInput.h"
#include "RuleOutput.h"
#include "json.h"
#include <unistd.h>

using namespace std;

// typedef enum
// {
// 	RULE_TYPE_TAP_TO_RUN = -2,
// 	RULE_TYPE_TIME = -1,
// 	RULE_TYPE_OR = 0,
// 	RULE_TYPE_AND,
// 	RULE_TYPE_TIME_OR,
// 	RULE_TYPE_TIME_AND
// } string;

#define RULE_TYPE_TAP_TO_RUN "tap_to_run"
#define RULE_TYPE_TIME "time"
#define RULE_TYPE_OR "or"
#define RULE_TYPE_AND "and"
#define RULE_TYPE_TIME_OR "time_or"
#define RULE_TYPE_TIME_AND "time_and"

typedef enum
{
	RULE_MODE_ALL_DAY = 0xFE,
	RULE_MODE_WORK_DAY = 0xF1,
	RULE_MODE_WEEKEND_DAY = 0x06,
} RuleMode;

class Rule : public Object
{
private:
	string type;

	bool enable;
	unsigned char repeater;
	bool fullDay;
	int startTime;
	int endTime;
	int count;
	int timerRegisterIndex;

	Json::Value ruleData;
	mutex doMtx;

	// TODO: Check when delete rule
	// bool isBusy;

	vector<RuleInput *> ruleInputList;
	vector<RuleOutput *> ruleOutputList;

public:
	Rule(string id,
			 string name,
			 uint16_t addr,
			 uint64_t updatedAt,
			 bool enable,
			 string type,
			 unsigned char repeater,
			 Json::Value &ruleData);
	Rule(string id, string type, unsigned char repeater, string name, uint16_t addr, Json::Value &ruleData);
	Rule(string id, string type, unsigned char repeater, string name, uint16_t addr, int startTime, int endTime, Json::Value &ruleData);
	virtual ~Rule();

	bool getEnable() const { return enable; }
	void setEnable(bool value) { enable = value; }

	unsigned char getRepeater() const { return repeater; }
	void setRepeater(unsigned char newRepeater) { repeater = newRepeater; }

	bool getFullDay() const { return fullDay; }
	void setFullDay(bool value) { fullDay = value; }

	int getStartTime() const { return startTime; }
	void setStartTime(int value) { startTime = value; }

	int getEndTime() const { return endTime; }
	void setEndTime(int value) { endTime = value; }

	int getCount() const { return count; }
	void setCount(int value) { count = value; }

	int getTimerRegisterIndex() const { return timerRegisterIndex; }
	void setTimerRegisterIndex(int value) { timerRegisterIndex = value; }

	Json::Value GetRuleData();
	void SetRuleData(Json::Value ruleData);
	string getType();
	void AddRuleInput(RuleInput *ruleInput);
	void AddRuleOutput(RuleOutput *ruleOutput);
	void DelAllRuleInput();
	void DelAllRuleOutput();
	void UpdateFirstRun();
	void Check();
	void RunOutput();
	void Do();

	void UpdateData(string type, unsigned char repeater, string name, uint16_t addr, Json::Value &ruleData);
	void UpdateData(string type, unsigned char repeater, string name, uint16_t addr, int startTime, int endTime, Json::Value &ruleData);
	void UpdateData(Json::Value &ruleData);
	// bool GetStatus();
	// void SetStatus(bool enable);
	// bool GetFirstRun();
	// void SetFirstRun(bool isFirstRun);
};
