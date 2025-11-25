#include <bluetooth/mesh.h>
#include "ble_mesh_defs.h"
#include "DeviceManager.h"
#include "DeviceBle.h"

extern struct bt_mesh_model models[];
extern struct bt_mesh_model vnd_models[];

extern "C" int sensor_status(struct bt_mesh_model *model,
                             struct bt_mesh_msg_ctx *ctx,
                             struct net_buf_simple *buf)
{
    char str[128];
    str[0] = 0;
    int start = 0;
    for (int i = 0; i < buf->len; i++)
    {
        start = strlen(str);
        sprintf(str + start, " %02x", buf->data[i]);
    }
    BT_ERR("addr 0x%04x, data:%s", ctx->addr, str);

    DeviceBle *deviceBle = DeviceManager::GetInstance()->GetDeviceBleFromAddr(ctx->addr);
    if (!deviceBle)
    {
        BT_ERR("%s: Get deviceBle failed", __func__);
        return -1;
    }

    struct __attribute__((packed))
    {
        uint8_t vendor_opcode;
        uint8_t data[31];
    } data_message = {
        .vendor_opcode = (uint8_t)BLE_MESH_MODEL_OP_SENSOR_STATUS};
    memcpy(data_message.data, buf->data, buf->len);
    deviceBle->InputData((uint8_t *)&data_message, buf->len + 1, ctx->addr);
    return 0;
}