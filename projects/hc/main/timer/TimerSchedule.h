#pragma once

#include <vector>
#include <mutex>
#include <functional>
#include <string>

using namespace std;

typedef function<void()> TimerCallbackFunc;

class Timer
{
private:
	int index;
	int time;

public:
	TimerCallbackFunc timerCallbackFunc;
	Timer(int index, int time, TimerCallbackFunc timerCallbackFunc);

	int GetIndex();
	int GetTime();
	void run();
};

class TimerSchedule
{
private:
	int index;

	TimerSchedule();

public:
	static TimerSchedule *GetInstance();

	mutex mtx;
	vector<Timer *> timerList;

	int RegisterTimer(string timer, TimerCallbackFunc timerCallbackFunc);
	int RegisterTimer(int time, TimerCallbackFunc timerCallbackFunc);
	int UnregisterTimer(int index);
};
