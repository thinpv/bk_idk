#pragma once

#include <string>
#include <stdint.h>
#include "json.h"

using namespace std;

class Object
{
protected:
	string id;
	string name;
	uint16_t addr;
	Json::Value dataValue;
	uint64_t updatedAt;
	bool waitingToCheck;

public:
	Object(string id, string name, uint16_t addr, uint64_t updatedAt)
			: id(id), name(name), addr(addr), dataValue(Json::objectValue), updatedAt(updatedAt), waitingToCheck(false) {}

	~Object() {}

	// Getters and Setters
	string getId() const { return id; }
	void setId(const string &newId) { id = newId; }

	string getName() const { return name; }
	void setName(const string &newName) { name = newName; }

	uint16_t getAddr() const { return addr; }
	void setAddr(uint16_t newAddr) { addr = newAddr; }

	uint64_t getUpdatedAt() const { return updatedAt; }
	void setUpdatedAt(uint64_t newUpdatedAt) { updatedAt = newUpdatedAt; }

	bool isWaitingToCheck() const { return waitingToCheck; }
	void setWaitingToCheck(bool value) { waitingToCheck = value; }

	Json::Value getData() const { return dataValue; }
	string getDataStr() const { return dataValue.toString(); }
	void setData(const Json::Value &value) { dataValue = value; }
	void setData(const string &value);

	virtual void parseDataValue(Json::Value &dataValue) {}
};
