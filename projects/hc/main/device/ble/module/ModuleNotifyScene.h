#pragma once

#include "Module.h"

using namespace std;

class ModuleNotifyScene : public Module
{
protected:
	uint16_t idScene;

public:
	ModuleNotifyScene(Device *device, uint16_t addr);
	~ModuleNotifyScene();

	/**
	 * @brief Parse raw data to module parameter value
	 *
	 * @param data data from device driver (uart)
	 * @param len length of data
	 * @param jsonValue json value to put parameter after parsing
	 * @return true if data include this module opcode
	 * @return false
	 */
	int InputData(uint8_t *data, int len, Json::Value &jsonValue);
};
