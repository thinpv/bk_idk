#pragma once

#include <string>
#include "json.h"

using namespace std;

class Storage
{
private:
	Storage();

public:
	static Storage *GetInstance();

	int init();
	int ReadFile(const char *filePath, char **data, int *len);
	int WriteFile(const char *filePath, const char *data);
	int DelFile(const char *filePath);

	int ReadAllRule();
	int ReadRule(int id, Json::Value &payloadJson);
	int ReadRule(int id, char **data, int *len);
	int WriteRule(int id, const char *data);
	int DelRule(int id);
	void DelAllFile();
};