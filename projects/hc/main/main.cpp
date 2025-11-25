#include "Log.h"
#include "Gateway.h"
#include "DeviceManager.h"

void *__dso_handle = 0;

extern "C" int main_cpp(void *arg)
{
    LOGI("Start main_cpp");
    // DeviceManager::GetInstance()->init();
    // Gateway::GetInstance()->init();

    // sleep(2); // wait for other module init
    // Device::InitDeviceModelList();

    LOGI("End main_cpp");

    return 0;
}