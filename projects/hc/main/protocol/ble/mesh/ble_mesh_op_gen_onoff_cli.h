#pragma once

#include "bluetooth/mesh.h"

/* Generic OnOff Client */
int gen_onoff_status(struct bt_mesh_model *model,
                     struct bt_mesh_msg_ctx *ctx,
                     struct net_buf_simple *buf);

int gen_onoff_send(bool val);