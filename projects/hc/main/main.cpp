#include "Gateway.h"
#include "DeviceManager.h"

void *__dso_handle = 0;

extern "C" int main_cpp(void *arg)
{

    Gateway::GetInstance()->init();

    string mac = "aabbccddeeff";
    DeviceManager::GetInstance()->AddDevice(mac, 1);

    return 0;
}