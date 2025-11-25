#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "json.h"
#include "ErrorCode.h"

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#define SLEEP_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))
#else
#define SLEEP_MS(ms) usleep(ms * 1000)
#endif

using namespace std;

namespace Util
{
	// string getTimeStrFromTime(time_t t);
	string getCurrentTimeStr();
	/**
	 * @brief Get the Current Timer in minute object
	 *
	 * @return int
	 */
	int GetCurrentTimeInDay();

	int GetYearsCurrent();
	int GetMonthsCurrent();
	int GetDateCurrent();
	int GetDaysCurrent();
	int GetHoursCurrent();
	int GetMinutesCurrent();
	int GetSecondsCurrent();

	/**
	 * @brief Get the Current Time in milli seconds object
	 *
	 * @return int
	 */
	uint64_t millis();

	string genRandRQI(int size);
	string GenIdDeviceByElement(string id, int element);

	int GetCurrentWeekDay();
	int ConvertStrTimeToInt(string time);
	bool HaveRTC(); //Kiểm tra trạng thái giờ thời gian thực
	uint8_t CalCrc(uint8_t length, uint8_t *data);
	string setString(const char *value);
	string ConvertU32ToHexString(uint8_t *data, int len);
	int ConvertStringToHex(string str, uint8_t *data, int len);
	int CheckDayInWeek(int day, int repeater);
	vector<string> splitString(string str, char splitter);
	bool CompareNumber(string op, int a, int b, int c = 0);

	string ExecuteCMD(string command);
	string ExecuteCMD(char const *command);
	string GetCurrentTimeStr();

	string uuidToStr(uint8_t *uuid);
	string GenUuidFromMac(string mac);

	void LedInternet(bool value);
	void LedService(bool value);
	void LedZigbee(bool value);
	void LedBle(bool value);
	void LedAll(bool value);
	void LedRestoreLastValue();
	void LedServiceLock();
	void LedServiceUnlock();

	bool GetStatusLedBle();
	bool GetStatusLedService();
	bool GetStatusLedZigbee();
	bool GetStatusLedInternet();

	float GetLongitude(string data);
	float GetLatitude(string data);

	void SetStatusWeatherOutdoor(int status);
	void SetTempWeatherOutdoor(uint16_t temp);
	int GetStatusWeatherOutdoor();
	uint16_t GetTempWeatherOutdoor();

	void SetTempOfScreenTouch(uint16_t temp);
	void SetHumOfScreenTouch(uint16_t hum);
	uint16_t GetTempOfScreenTouch();
	uint16_t GetHumOfScreenTouch();

	bool compareByID(const Json::Value &obj1, const Json::Value &obj2);
	Json::Value arrangeJson(Json::Value &obj);

#ifndef ESP_PLATFORM
	string encryptAes128(string key, string plaintext);
	string calculateSHA256Checksum(string &filePath);
#endif
}
