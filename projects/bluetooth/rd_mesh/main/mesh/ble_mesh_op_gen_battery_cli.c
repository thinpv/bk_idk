#include <bluetooth/mesh.h>

extern struct bt_mesh_model models[];
extern struct bt_mesh_model vnd_models[];

int gen_battery_status(struct bt_mesh_model *model,
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
    return 0;
}