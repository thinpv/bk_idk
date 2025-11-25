#pragma once

#include "DeviceBle.h"

using namespace std;

class DeviceBleRepeater : public DeviceBle
{
public:
	DeviceBleRepeater(string id, string name, string mac, uint32_t type, uint16_t addr, uint16_t version, Json::Value *dataJson);
};