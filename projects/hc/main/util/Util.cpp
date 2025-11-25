#include "Util.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
// #include <ifaddrs.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/ioctl.h>
// #include <linux/if.h>
// #include <sys/socket.h>
#include <algorithm>
#include <functional>
#include <cctype>
#include <ctime>
#include <locale>
#include "Base64.h"
#include "Log.h"
#include <fstream>
#include <sstream>
#include <iomanip>

#include <iostream>
#include <string>
#include <cstring>

// #ifndef ESP_PLATFORM
// #include <openssl/sha.h>
// #include <openssl/conf.h>
// #include <openssl/evp.h>
// #include <openssl/err.h>
// #else
// #include "Sntp.h"
// #endif

using namespace std;

string Util::genRandRQI(int size)
{
	string rqi = "";
	srand(time(0));
	for (int i = 0; i < size; i++)
	{
		rqi += 'a' + rand() % 26;
	}
	return rqi;
}

string Util::GenIdDeviceByElement(string id, int element)
{
	int c = id[0] - 48 + element;
	string strTemp = to_string(c);
	string tempId = id.substr(strTemp.length(), id.length() - strTemp.length());
	return (strTemp + tempId);
}

string getTimeStrFromTime(time_t t)
{
	struct tm start;
	start = *localtime(&t);
	char timeBuffer[100];
	sprintf(timeBuffer, "%04d%02d%02dT%02d%02d%02d", start.tm_year + 1900, start.tm_mon + 1, start.tm_mday, start.tm_hour, start.tm_min, start.tm_sec);
	return string(timeBuffer);
}

string Util::GetCurrentTimeStr()
{
	return getTimeStrFromTime(time(NULL));
}

int Util::GetCurrentTimeInDay()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_hour * 3600 + lt.tm_min * 60 + lt.tm_sec;
}

int Util::GetYearsCurrent()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_year + 1900;
}

int Util::GetMonthsCurrent()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_mon + 1;
}

int Util::GetDateCurrent()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_mday;
}

int Util::GetDaysCurrent()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_wday + 1;
}

int Util::GetHoursCurrent()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_hour;
}

int Util::GetMinutesCurrent()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_min;
}

int Util::GetSecondsCurrent()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_sec;
}

uint64_t Util::millis()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * (uint64_t)1000 + (tv.tv_usec) / 1000;
}

int Util::GetCurrentWeekDay()
{
	time_t t = time(NULL);
	struct tm lt = *localtime(&t);
	return lt.tm_wday;
}

int Util::ConvertStrTimeToInt(string time)
{
	int hour;
	int minute;
	int second;
	if (sscanf(time.c_str(), "%d:%d:%d", &hour, &minute, &second) == 3)
		return hour * 3600 + minute * 60 + second;
	else if (sscanf(time.c_str(), "%d:%d", &hour, &minute) == 2)
		return hour * 3600 + minute * 60;
	return CODE_ERROR;
}

bool Util::HaveRTC()
{
#ifdef ESP_PLATFORM
	if (!Sntp::haveNtpTime())
	{
		return false;
	}
#endif
	return true;
}

uint8_t Util::CalCrc(uint8_t length, uint8_t *data)
{
	uint8_t crc = 0;
	int i = 0;
	for (i = 0; i < length; i++)
	{
		crc = crc ^ data[i];
	}
	crc = crc & 0xFF;
	return crc;
}

string Util::setString(const char *value)
{
	return value ? value : "";
}

string Util::ConvertU32ToHexString(uint8_t *data, int len)
{
	char buff[100];
	if (len > 50)
		len = 50;
	for (int i = 0; i < len; i++)
	{
		sprintf(buff + i * 2, "%02x", data[i]);
	}
	return string(buff);
}

int Util::ConvertStringToHex(string str, uint8_t *data, int len)
{
	LOGE("Bo sung code");
	return CODE_OK;
}

int Util::CheckDayInWeek(int day, int repeater)
{
	int byte;
	switch (day)
	{
	case 0: // sun
		byte = 6;
		break;
	case 1: // mon
	case 2: // tue
	case 3: // wed
	case 4: // thu
	case 5: // fri
	case 6: // sat
		byte = day - 1;
		break;
	default:
		return false;
	}
	if (repeater & (1 << byte))
		return true;
	return false;
}

vector<string> Util::splitString(string str, char splitter)
{
	vector<string> result;
	string current = "";
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == splitter)
		{
			if (current != "")
			{
				result.push_back(current);
				current = "";
			}
			continue;
		}
		current += str[i];
	}
	if (current.size() != 0)
		result.push_back(current);
	return result;
}

bool Util::CompareNumber(string op, int a, int b, int c)
{
	if (op == "==")
		return a == b;
	else if (op == "!=")
		return a != b;
	else if (op == ">")
		return a > b;
	else if (op == ">=")
		return a >= b;
	else if (op == "<")
		return a < b;
	else if (op == "<=")
		return a <= b;
	else if (op == "<>")
		return ((a >= b) && (a <= c));
	else if (op == "><")
		return ((a <= b) || (a >= c));
	return false;
}

string Util::ExecuteCMD(string command)
{
	return ExecuteCMD(command.c_str());
}

string Util::ExecuteCMD(char const *command)
{
	// LOGV("ExecuteCMD: %s", command);
	string msg_rsp = "";
#ifndef ESP_PLATFORM
	FILE *file;
	char msg_line[100] = {0};
	file = popen(command, "r");
	if (file == NULL)
	{
		LOGE("ExecuteCMD");
		exit(1);
	}
	while (1)
	{
		fgets(msg_line, 100, file);
		msg_rsp += msg_line;
		if (feof(file))
		{
			break;
		}
	}
	pclose(file);
#endif
	return msg_rsp;
}

string Util::uuidToStr(uint8_t *uuid)
{
	char buf[100];
	sprintf(buf, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					uuid[0], uuid[1], uuid[2], uuid[3],
					uuid[4], uuid[5], uuid[6], uuid[7],
					uuid[8], uuid[9], uuid[10], uuid[11],
					uuid[12], uuid[13], uuid[14], uuid[15]);
	buf[36] = '\0';
	return string(buf);
}

string Util::GenUuidFromMac(string mac)
{
	if (mac.length() == 16)
	{
		string mac1 = mac.substr(0, 4);
		string mac2 = mac.substr(4, 4);
		string mac3 = mac.substr(8, 4);
		string mac4 = mac.substr(12, 4);
		return mac1 + mac2 + "-" + mac3 + "-" + mac4 + "-" + mac1 + "-" + mac2 + mac3 + mac4;
	}
	return "";
}

static bool ledInternet = false;
static bool ledService = false;
static bool ledZigbee = false;
static bool ledBle = false;

void Util::LedInternet(bool value)
{
	ledInternet = value;
#ifdef __OPENWRT__
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:internet/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:internet/brightness");
	}
#endif
}

void Util::LedService(bool value)
{
	ledService = value;
#ifdef __OPENWRT__
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:service/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:service/brightness");
	}
#endif
}

void Util::LedZigbee(bool value)
{
	ledZigbee = value;
#ifdef __OPENWRT__
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness");
	}
#endif
}

void Util::LedBle(bool value)
{
	ledBle = value;
#ifdef __OPENWRT__
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness");
	}
#endif
}

void Util::LedAll(bool value)
{
#ifdef __OPENWRT__
	if (value)
	{
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:internet/brightness");
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:service/brightness");
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness");
		ExecuteCMD("/bin/echo \"1\" > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness");
	}
	else
	{
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:internet/brightness");
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:service/brightness");
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness");
		ExecuteCMD("/bin/echo \"0\" > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness");
	}
#endif
}

void Util::LedRestoreLastValue()
{
	LedInternet(ledInternet);
	LedService(ledService);
	LedZigbee(ledZigbee);
	LedBle(ledBle);
}

static int ledServiceCount = 0;
void Util::LedServiceLock()
{
	ledServiceCount++;
	if (ledServiceCount)
		LedService(false);
}

void Util::LedServiceUnlock()
{
	ledServiceCount--;
	if (!ledServiceCount)
		LedService(true);
}

bool Util::GetStatusLedBle()
{
	return ledBle;
}

bool Util::GetStatusLedService()
{
	return ledService;
}

bool Util::GetStatusLedZigbee()
{
	return ledZigbee;
}

bool Util::GetStatusLedInternet()
{
	return ledInternet;
}

float Util::GetLongitude(string data)
{
	float longitude = 0.0;
	Json::Value dataJson;
	dataJson.parse(data);
	if (dataJson.isObject() && dataJson.isMember("longitude") && dataJson["longitude"].isDouble())
	{
		longitude = dataJson["longitude"].asDouble();
	}
	return longitude;
}

float Util::GetLatitude(string data)
{
	float latitude = 0.0;
	Json::Value dataJson;
	dataJson.parse(data);
	if (dataJson.isObject() && dataJson.isMember("latitude") && dataJson["latitude"].isDouble())
	{
		latitude = dataJson["latitude"].asDouble();
	}
	return latitude;
}

static int statusWeather;
static uint16_t tempWeather;
void Util::SetStatusWeatherOutdoor(int status)
{
	statusWeather = status;
}

void Util::SetTempWeatherOutdoor(uint16_t temp)
{
	tempWeather = temp;
}

int Util::GetStatusWeatherOutdoor()
{
	return statusWeather;
}

uint16_t Util::GetTempWeatherOutdoor()
{
	return tempWeather;
}

static uint16_t tempForScreenTouch = 0;
static uint16_t humForScreenTouch = 0;

void Util::SetTempOfScreenTouch(uint16_t temp)
{
	tempForScreenTouch = temp;
}

void Util::SetHumOfScreenTouch(uint16_t hum)
{
	humForScreenTouch = hum;
}

uint16_t Util::GetTempOfScreenTouch()
{
	return tempForScreenTouch;
}

uint16_t Util::GetHumOfScreenTouch()
{
	return humForScreenTouch;
}

bool Util::compareByID(const Json::Value &obj1, const Json::Value &obj2)
{
	return obj1["ID"].asInt() > obj2["ID"].asInt();
}

Json::Value Util::arrangeJson(Json::Value &property)
{
	if (property.isArray())
	{
		vector<Json::Value> listProperties;
		for (int numProperty = 0; numProperty < property.size(); numProperty++)
		{
			if (property[numProperty].isObject())
			{
				listProperties.push_back(property[numProperty]);
			}
		}
		std::sort(listProperties.begin(), listProperties.end(), Util::compareByID);
		Json::Value result;
		for (int i = 0; i < listProperties.size(); i++)
		{
			result.append(listProperties[i]);
		}
		return result;
	}
	return Json::Value::null;
}

#ifndef ESP_PLATFORM
static void handleErrors(void)
{
	// ERR_print_errors_fp(stderr);
	abort();
}

string Util::encryptAes128(string key, string plaintext)
{
	// unsigned char *key_c = new unsigned char[key.length() + 1];
	// memcpy((char *)key_c, key.c_str(), key.length());
	// key_c[key.length()] = '\0';

	// unsigned char *plaintext_c = new unsigned char[plaintext.length() + 1];
	// memcpy((char *)plaintext_c, plaintext.c_str(), plaintext.length());
	// plaintext_c[plaintext.length()] = '\0';

	// int plaintext_len = plaintext.length();

	// EVP_CIPHER_CTX *ctx;
	// unsigned char ciphertext[128] = {0};
	// int len;
	// int ciphertext_len;

	// if (!(ctx = EVP_CIPHER_CTX_new()))
	// 	handleErrors();

	// if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key_c, NULL))
	// {
	// 	handleErrors();
	// }

	// if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext_c, plaintext_len))
	// {
	// 	handleErrors();
	// }
	// ciphertext_len = len;

	// if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
	// 	handleErrors();
	// ciphertext_len += len;

	// EVP_CIPHER_CTX_free(ctx);

	// std::stringstream ss;
	// ss << std::hex << std::setfill('0');
	// for (int i = 0; i < ciphertext_len; ++i)
	// {
	// 	ss << std::setw(2) << static_cast<int>(ciphertext[i]);
	// }

	// delete[] key_c;
	// delete[] plaintext_c;

	// return ss.str();
	return "sha";
}

string Util::calculateSHA256Checksum(string &filePath)
{
	// std::ifstream file(filePath, std::ios::binary);
	// if (!file)
	// {
	// 	throw std::runtime_error("Failed to open file.");
	// }

	// SHA256_CTX sha256Context;
	// SHA256_Init(&sha256Context);

	// char buffer[1024];
	// while (!file.eof())
	// {
	// 	file.read(buffer, sizeof(buffer));
	// 	SHA256_Update(&sha256Context, buffer, file.gcount());
	// }

	// unsigned char hash[SHA256_DIGEST_LENGTH];
	// SHA256_Final(hash, &sha256Context);

	// std::stringstream checksum;
	// checksum << std::hex << std::setfill('0');
	// for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	// {
	// 	checksum << std::setw(2) << static_cast<int>(hash[i]);
	// }
	// return checksum.str();
	return "sha256";
}
#endif
