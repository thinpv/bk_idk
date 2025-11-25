#include <bluetooth/mesh.h>

#include "BleMeshDefine.h"
#include "ble_mesh_defs.h"
#include "ble_mesh_device.h"
#include "ble_mesh_vendor.h"

extern struct bt_mesh_model models[];
extern struct bt_mesh_model vnd_models[];
extern struct provisioner_ctx_struct scanning_device;

int vnd_cli_status_e0(struct bt_mesh_model *model,
                      struct bt_mesh_msg_ctx *ctx,
                      struct net_buf_simple *buf)
{
    BT_ERR("vnd_cli_status_e0");
    uint16_t header = net_buf_simple_pull_le16(buf);
    BT_ERR("vnd_cli_status_e0 addr 0x%04X header 0x%04X", ctx->addr, header);
    if (header == RD_HEADER_PROVISION_SET_GW_ADDR)
    {
        BT_WARN("Received set_gw_addr response from 0x%04X", ctx->addr);
        BT_WARN("scanning_device mac %02X:%02X:%02X:%02X:%02X:%02X",
                scanning_device.peer_uuid_mac[5],
                scanning_device.peer_uuid_mac[4],
                scanning_device.peer_uuid_mac[3],
                scanning_device.peer_uuid_mac[2],
                scanning_device.peer_uuid_mac[1],
                scanning_device.peer_uuid_mac[0]);
        get_device_type(ctx->addr, scanning_device.peer_uuid_mac);
    }
    else if (header == RD_HEADER_PROVISION_GET_DEV_TYPE)
    {
        uint32_t deviceType;
        uint8_t magic;
        uint16_t version;

        deviceType = net_buf_simple_pull_be24(buf);
        magic = net_buf_simple_pull_u8(buf);
        version = net_buf_simple_pull_le16(buf);
        BT_WARN("Device Type: %08X, Magic: %02X, Version: %d", deviceType, magic, version);
    }

    return 0;
}

int vnd_cli_status_e2(struct bt_mesh_model *model,
                      struct bt_mesh_msg_ctx *ctx,
                      struct net_buf_simple *buf)
{
    BT_ERR("vnd_cli_status_e2");

    return 0;
}