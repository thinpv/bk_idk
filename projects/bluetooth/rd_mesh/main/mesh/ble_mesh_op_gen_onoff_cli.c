#include <bluetooth/mesh.h>

#include "ble_mesh_defs.h"

extern struct bt_mesh_model models[];
extern struct bt_mesh_model vnd_models[];

static const char *const onoff_str[] = {"off", "on"};

/* OnOff messages' transition time and remaining time fields are encoded as an
 * 8 bit value with a 6 bit step field and a 2 bit resolution field.
 * The resolution field maps to:
 * 0: 100 ms
 * 1: 1 s
 * 2: 10 s
 * 3: 20 min
 */
static const uint32_t time_res[] = {
    100,
    MSEC_PER_SEC,
    10 * MSEC_PER_SEC,
    10 * 60 * MSEC_PER_SEC,
};

static inline int32_t model_time_decode(uint8_t val)
{
    uint8_t resolution = (val >> 6) & BIT_MASK(2);
    uint8_t steps = val & BIT_MASK(6);

    if (steps == 0x3f)
    {
        return SYS_FOREVER_MS;
    }

    return steps * time_res[resolution];
}

int gen_onoff_status(struct bt_mesh_model *model,
                     struct bt_mesh_msg_ctx *ctx,
                     struct net_buf_simple *buf)
{
    uint8_t present = net_buf_simple_pull_u8(buf);

    if (buf->len)
    {
        uint8_t target = net_buf_simple_pull_u8(buf);
        int32_t remaining_time =
            model_time_decode(net_buf_simple_pull_u8(buf));

        BT_ERR("OnOff status: %s -> %s: (%d ms)\n", onoff_str[present], onoff_str[target], remaining_time);
        return 0;
    }

    BT_ERR("OnOff status: %s\n", onoff_str[present]);

    return 0;
}

/** Send an OnOff Set message from the Generic OnOff Client to all nodes. */
int gen_onoff_send(bool val)
{
    struct bt_mesh_msg_ctx ctx = {
        .app_idx = models[3].keys[0], /* Use the bound key */
        .addr = BT_MESH_ADDR_ALL_NODES,
        .send_ttl = BT_MESH_TTL_DEFAULT,
    };
    static uint8_t tid;

    if (ctx.app_idx == BT_MESH_KEY_UNUSED)
    {
        BT_ERR("The Generic OnOff Client must be bound to a key before sending.");
        return -ENOENT;
    }

    BT_MESH_MODEL_BUF_DEFINE(buf, BLE_MESH_MODEL_OP_GEN_ONOFF_SET, 2);
    bt_mesh_model_msg_init(&buf, BLE_MESH_MODEL_OP_GEN_ONOFF_SET);
    net_buf_simple_add_u8(&buf, val);
    net_buf_simple_add_u8(&buf, tid++);

    BT_ERR("Sending OnOff Set: %s", onoff_str[val]);

    return bt_mesh_model_send(&models[3], &ctx, &buf, NULL, NULL);
}

// static int onoff_status_send(struct bt_mesh_model *model,
//                              struct bt_mesh_msg_ctx *ctx)
// {
//     uint32_t remaining;

//     BT_MESH_MODEL_BUF_DEFINE(buf, OP_ONOFF_STATUS, 3);
//     bt_mesh_model_msg_init(&buf, OP_ONOFF_STATUS);

//     remaining = k_ticks_to_ms_floor32(
//                     k_work_delayable_remaining_get(&onoff.work)) +
//                 onoff.transition_time;

//     /* Check using remaining time instead of "work pending" to make the
//      * onoff status send the right value on instant transitions. As the
//      * work item is executed in a lower priority than the mesh message
//      * handler, the work will be pending even on instant transitions.
//      */
//     if (remaining)
//     {
//         net_buf_simple_add_u8(&buf, !onoff.val);
//         net_buf_simple_add_u8(&buf, onoff.val);
//         net_buf_simple_add_u8(&buf, model_time_encode(remaining));
//     }
//     else
//     {
//         net_buf_simple_add_u8(&buf, onoff.val);
//     }

//     return bt_mesh_model_send(model, ctx, &buf, NULL, NULL);
// }

// static void onoff_timeout(struct k_work *work)
// {
//     if (onoff.transition_time)
//     {
//         /* Start transition.
//          *
//          * The LED should be on as long as the transition is in
//          * progress, regardless of the target value, according to the
//          * Bluetooth Mesh Model specification, section 3.1.1.
//          */
//         board_led_set(true);

//         k_work_reschedule(&onoff.work, K_MSEC(onoff.transition_time));
//         onoff.transition_time = 0;
//         return;
//     }

//     board_led_set(onoff.val);
// }

// /* Generic OnOff Server message handlers */

// static int gen_onoff_get(struct bt_mesh_model *model,
//                          struct bt_mesh_msg_ctx *ctx,
//                          struct net_buf_simple *buf)
// {
//     onoff_status_send(model, ctx);
//     return 0;
// }

// static int gen_onoff_set_unack(struct bt_mesh_model *model,
//                                struct bt_mesh_msg_ctx *ctx,
//                                struct net_buf_simple *buf)
// {
//     uint8_t val = net_buf_simple_pull_u8(buf);
//     uint8_t tid = net_buf_simple_pull_u8(buf);
//     int32_t trans = 0;
//     int32_t delay = 0;

//     if (buf->len)
//     {
//         trans = model_time_decode(net_buf_simple_pull_u8(buf));
//         delay = net_buf_simple_pull_u8(buf) * 5;
//     }

//     /* Only perform change if the message wasn't a duplicate and the
//      * value is different.
//      */
//     if (tid == onoff.tid && ctx->addr == onoff.src)
//     {
//         /* Duplicate */
//         return 0;
//     }

//     if (val == onoff.val)
//     {
//         /* No change */
//         return 0;
//     }

//     BT_ERR("set: %s delay: %d ms time: %d ms click_count %d\n", onoff_str[val], delay, trans);

//     onoff.tid = tid;
//     onoff.src = ctx->addr;
//     onoff.val = val;
//     onoff.transition_time = trans;

//     /* Schedule the next action to happen on the delay, and keep
//      * transition time stored, so it can be applied in the timeout.
//      */
//     k_work_reschedule(&onoff.work, K_MSEC(delay));

//     return 0;
// }

// static int gen_onoff_set(struct bt_mesh_model *model,
//                          struct bt_mesh_msg_ctx *ctx,
//                          struct net_buf_simple *buf)
// {
//     (void)gen_onoff_set_unack(model, ctx, buf);
//     onoff_status_send(model, ctx);

//     return 0;
// }