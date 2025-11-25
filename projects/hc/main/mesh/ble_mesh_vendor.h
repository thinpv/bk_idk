#pragma once

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>
#include "os/os.h"

int set_gw_addr(uint16_t addr);
int get_device_type(uint16_t addr, uint8_t *mac);
