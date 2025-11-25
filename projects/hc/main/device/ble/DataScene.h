#pragma once

#include <iostream>
#include <string>
#include "json.h"
namespace DataScene
{
    void InitDataScene();
    Json::Value GetDataScene();
    Json::Value GetDataDeviceInScene(uint32_t type, int idScene);
}