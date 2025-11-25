#pragma once

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>
#include "os/os.h"

#if defined(__cplusplus)
extern "C" {
#endif

int set_gw_addr(uint16_t addr);
int get_device_type(uint16_t addr, uint8_t *mac);


#if defined(__cplusplus)
}
#endif