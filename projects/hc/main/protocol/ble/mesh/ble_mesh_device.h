#pragma once

// #include <bluetooth/bluetooth.h>
// #include <bluetooth/mesh.h>
// #include "os/os.h"

#include <stddef.h>
#include <stdint.h>

typedef struct provisioner_ctx_struct
{
    uint8_t status;
    uint16_t peer_addr;
    uint32_t recv_count;

    uint8_t peer_uuid_mac[6];
} provisioner_ctx_struct;
