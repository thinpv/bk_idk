#include <settings/settings.h>

#include <sys/byteorder.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>

#include "ble_mesh_ui.h"
#include "mesh/mesh.h"
#include "mesh/subnet.h"
#include "bluetooth/bluetooth.h"
#include "bluetooth/mesh.h"
#include "host/ethermind_impl.h"
#include "ble_mesh_provision.h"
#include "common/log.h"
#include "ble_mesh_ui.h"
#include "os/os.h"
#include "components/bluetooth/bk_ble_types.h"
#include "mesh/rpl.h"

#include "ble_mesh_vendor.h"
#include "BleMeshDefine.h"

#include "ble_mesh_defs.h"
#include "ble_mesh_device.h"

#define UINT8_TO_STREAM(p, u8)  \
    do                          \
    {                           \
        *(p)++ = (uint8_t)(u8); \
    } while (0)
#define UINT16_TO_STREAM(p, u16)        \
    do                                  \
    {                                   \
        *(p)++ = (uint8_t)(u16);        \
        *(p)++ = (uint8_t)((u16) >> 8); \
    } while (0)
#define UINT32_TO_STREAM(p, u32)         \
    do                                   \
    {                                    \
        *(p)++ = (uint8_t)(u32);         \
        *(p)++ = (uint8_t)((u32) >> 8);  \
        *(p)++ = (uint8_t)((u32) >> 16); \
        *(p)++ = (uint8_t)((u32) >> 24); \
    } while (0)
#define ARRAY_TO_STREAM(p, s, size) \
    do                              \
    {                               \
        memcpy(p, s, size);         \
        p += size;                  \
    } while (0)
#define ARRAY_TO_STREAM_REV(p, s, size)   \
    do                                    \
    {                                     \
        for (size_t i = 0; i < size; i++) \
        {                                 \
            p[i] = s[size - i - 1];       \
        }                                 \
        p += size;                        \
    } while (0)

#define ceiling_fraction(numerator, divider) (((numerator) + ((divider) - 1)) / (divider))

extern ble_err_t bk_bluetooth_get_address(uint8_t *mac);
static struct k_work button_work;

// static uint16_t current_dev_addr = BT_MESH_ADDR_UNASSIGNED;

struct provision_ctx_struct
{
    uint8_t status;
    uint16_t local_addr;
    uint16_t net_idx;
    uint16_t netkey_idx;
} s_provision_ctx = {
    .local_addr = BT_MESH_ADDR_UNASSIGNED,
};

struct provisioner_ctx_struct scanning_device = {
    .status = PROVISION_STATUS_IDLE,
    .peer_addr = 0,
    .recv_count = 0,
};

struct provisioner_ctx_struct s_provisioner_ctx[20] = {
    {PROVISION_STATUS_IDLE, 2, 0},
    {PROVISION_STATUS_IDLE, 3, 0},
    {PROVISION_STATUS_IDLE, 4, 0},
    {PROVISION_STATUS_IDLE, 5, 0},
    {PROVISION_STATUS_IDLE, 6, 0},
    {PROVISION_STATUS_IDLE, 7, 0},
    {PROVISION_STATUS_IDLE, 8, 0},
    {PROVISION_STATUS_IDLE, 9, 0},
    {PROVISION_STATUS_IDLE, 10, 0},
    {PROVISION_STATUS_IDLE, 11, 0},
    {PROVISION_STATUS_IDLE, 12, 0},
    {PROVISION_STATUS_IDLE, 13, 0},
    {PROVISION_STATUS_IDLE, 14, 0},
    {PROVISION_STATUS_IDLE, 15, 0},
    {PROVISION_STATUS_IDLE, 16, 0},
    {PROVISION_STATUS_IDLE, 17, 0},
    {PROVISION_STATUS_IDLE, 18, 0},
    {PROVISION_STATUS_IDLE, 19, 0},
    {PROVISION_STATUS_IDLE, 20, 0},
    {PROVISION_STATUS_IDLE, 21, 0},
};

// static uint16_t s_app_idx = 0;
static uint16_t s_appkey_idx = 0;
static uint16_t s_net_idx = 0;
static uint16_t s_netkey_idx = 0;

static uint8_t dev_uuid[16] = {BEKEN_VND_PROVISIONEE_UUID_HEAD, 0};
const static uint8_t provisioner_uuid[16] = {BEKEN_VND_PROVISIONER_UUID_HEAD, 0};
const static uint8_t provisionee_uuid[16] = {BEKEN_VND_PROVISIONEE_UUID_HEAD, 0};

static uint8_t provision_role = PROVISION_ROLE_UNKNOW;

static uint8_t net_key[16] = {1, 0};
static uint8_t dev_key[16] = {0};
static uint8_t app_key[16] = {0};

static void board_led_set_int(void)
{
    static uint8_t on = 1;

    extern void ble_gpio_debug(uint32_t pin, uint8_t up);
    ble_gpio_debug(25, on ? 1 : 0);
    on = (on ? 0 : 1);
}

static void board_led_set(bool val)
{
    BT_ERR("%d", val);
}

static void board_output_number(bt_mesh_output_action_t action, uint32_t number)
{
    BT_ERR("%d %d", action, number);
}

static void attention_on(struct bt_mesh_model *mod)
{
    BT_ERR("");
    board_led_set(true);
}

static void attention_off(struct bt_mesh_model *mod)
{
    BT_ERR("");
    board_led_set(false);
}

static const struct bt_mesh_health_srv_cb health_cb = {
    .attn_on = attention_on,
    .attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
    .cb = &health_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

int add_new_device(char *mac, uint32_t type, uint16_t addr, uint16_t version);

int gen_onoff_status(struct bt_mesh_model *model,
                            struct bt_mesh_msg_ctx *ctx,
                            struct net_buf_simple *buf);

int gen_battery_status(struct bt_mesh_model *model,
                              struct bt_mesh_msg_ctx *ctx,
                              struct net_buf_simple *buf);

int sensor_status(struct bt_mesh_model *model,
                         struct bt_mesh_msg_ctx *ctx,
                         struct net_buf_simple *buf);

int time_status(struct bt_mesh_model *model,
                       struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf);

int scene_status(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf);

int scheduler_status(struct bt_mesh_model *model,
                            struct bt_mesh_msg_ctx *ctx,
                            struct net_buf_simple *buf);

int light_lightness_status(struct bt_mesh_model *model,
                                  struct bt_mesh_msg_ctx *ctx,
                                  struct net_buf_simple *buf);

int light_ctl_status(struct bt_mesh_model *model,
                            struct bt_mesh_msg_ctx *ctx,
                            struct net_buf_simple *buf);

int light_hsl_status(struct bt_mesh_model *model,
                            struct bt_mesh_msg_ctx *ctx,
                            struct net_buf_simple *buf);

int vnd_cli_status_e0(struct bt_mesh_model *model,
                             struct bt_mesh_msg_ctx *ctx,
                             struct net_buf_simple *buf);

int vnd_cli_status_e2(struct bt_mesh_model *model,
                             struct bt_mesh_msg_ctx *ctx,
                             struct net_buf_simple *buf);

static const struct bt_mesh_model_op gen_onoff_cli_op[] = {
    {BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, BT_MESH_LEN_MIN(1), gen_onoff_status},
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op gen_battery_cli_op[] = {
    {BLE_MESH_MODEL_OP_GEN_BATTERY_STATUS, BT_MESH_LEN_MIN(1), gen_battery_status},
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op sensor_cli_op[] = {
    {BLE_MESH_MODEL_OP_SENSOR_STATUS, BT_MESH_LEN_MIN(1), sensor_status},
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op time_cli_op[] = {
    {BLE_MESH_MODEL_OP_TIME_STATUS, BT_MESH_LEN_MIN(1), time_status},
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op scene_cli_op[] = {
    {BLE_MESH_MODEL_OP_SCENE_STATUS, BT_MESH_LEN_MIN(1), scene_status},
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op scheduler_cli_op[] = {
    {BLE_MESH_MODEL_OP_SCHEDULER_STATUS, BT_MESH_LEN_MIN(1), scheduler_status},
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op light_lightness_cli_op[] = {
    {BLE_MESH_MODEL_OP_LIGHT_LIGHTNESS_STATUS, BT_MESH_LEN_MIN(1), light_lightness_status},
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op light_ctl_cli_op[] = {
    {BLE_MESH_MODEL_OP_LIGHT_CTL_STATUS, BT_MESH_LEN_MIN(1), light_ctl_status},
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op light_hsl_cli_op[] = {
    {BLE_MESH_MODEL_OP_LIGHT_HSL_STATUS, BT_MESH_LEN_MIN(1), light_hsl_status},
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op vnd_cli_op[] = {
    {RD_VND_MODEL_OP_STATUS_E0, BT_MESH_LEN_MIN(2), vnd_cli_status_e0},
    {RD_VND_MODEL_OP_STATUS_E2, BT_MESH_LEN_MIN(2), vnd_cli_status_e2},
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_cfg_cli cfg_cli =
    {};

/* This application only needs one element to contain its models */
struct bt_mesh_model models[] = {
    BT_MESH_MODEL_CFG_SRV,
    BT_MESH_MODEL_CFG_CLI(&cfg_cli),
    BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
    // BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_srv_op, NULL, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, gen_onoff_cli_op, NULL, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_BATTERY_CLI, gen_battery_cli_op, NULL, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_SENSOR_CLI, sensor_cli_op, NULL, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_TIME_CLI, time_cli_op, NULL, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_SCENE_CLI, scene_cli_op, NULL, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_SCHEDULER_CLI, scheduler_cli_op, NULL, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_LIGHT_LIGHTNESS_CLI, light_lightness_cli_op, NULL, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_LIGHT_CTL_CLI, light_ctl_cli_op, NULL, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_LIGHT_HSL_CLI, light_hsl_cli_op, NULL, NULL),
};

struct bt_mesh_model vnd_models[] = {
    BT_MESH_MODEL_VND(RD_VENDOR_ID, RD_VND_MODEL_CLIENT, vnd_cli_op, NULL, NULL),
};

static struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, models, vnd_models),
};

static const struct bt_mesh_comp comp = {
    .cid = BT_COMP_ID_LF,
    .elem = elements,
    .elem_count = ARRAY_SIZE(elements),
};

static void reset_device_ctx(uint8_t i)
{
    uint16_t addr = s_provisioner_ctx[i].peer_addr;

    memset(s_provisioner_ctx + i, 0, sizeof(s_provisioner_ctx[i]));

    s_provisioner_ctx[i].peer_addr = addr;
}

static void deprovision_device_inter(uint16_t addr)
{
    int err = 0;
    bool reset = false;
    (void)(reset);

    err = bt_mesh_cfg_node_reset(s_netkey_idx, addr, &reset);

    if (err)
    {
        BT_ERR("Unable to send Remote Node Reset (err %d)", err);
    }

    struct bt_mesh_cdb_node *node = bt_mesh_cdb_node_get(addr);

    if (node == NULL)
    {
        BT_ERR("No node with address 0x%04x", addr);
        //                return 0;
    }
    else
    {
        bt_mesh_cdb_node_del(node, true);
    }

    // this is a unsafe perform, you will under replay attack !! Dont't do it unless debug !!!
    bt_mesh_rpl_clear();
}

static void deprovision_device(uint8_t *mac)
{
    for (uint8_t i = 0; i < sizeof(s_provisioner_ctx) / sizeof(s_provisioner_ctx[0]); ++i)
    {
        if (s_provisioner_ctx[i].status == PROVISION_STATUS_PROVISION_COMPL &&
            0 == memcmp(mac, s_provisioner_ctx[i].peer_uuid_mac, sizeof(s_provisioner_ctx[i].peer_uuid_mac)))
        {
            deprovision_device_inter(s_provisioner_ctx[i].peer_addr);
            reset_device_ctx(i);

            BT_WARN("Remote node reset complete");

            return;
        }
    }
}

static uint8_t is_provisioning(void)
{
    for (uint8_t i = 0; i < sizeof(s_provisioner_ctx) / sizeof(s_provisioner_ctx[0]); ++i)
    {
        if (s_provisioner_ctx[i].status != PROVISION_STATUS_IDLE && s_provisioner_ctx[i].status != PROVISION_STATUS_PROVISION_COMPL)
        {
            return 1;
        }
    }

    return 0;
}

static int user_output_number(bt_mesh_output_action_t action, uint32_t number)
{
    BT_ERR("OOB Number: %u 0x%0x\n", number, action);

    board_output_number(action, number);

    return 0;
}

static void user_prov_reset(void)
{
    BT_WARN("The local node has been reset and needs reprovisioning");
    //    bt_mesh_prov_enable(BT_MESH_PROV_ADV);// | BT_MESH_PROV_GATT);
    // onoff.count_value = 0;
    memset(&s_provision_ctx, 0, sizeof(s_provision_ctx));
}

static void user_unprovisioned_beacon(uint8_t uuid[16],
                                      bt_mesh_prov_oob_info_t oob_info,
                                      uint32_t *uri_hash,
                                      const bt_addr_le_t *addr)
{
    if (provision_role == PROVISION_ROLE_PROVISIONER)
    {
        int ret = 0;

        if (is_provisioning())
        {
            return;
        }

        if (addr == NULL)
        {
            return;
        }

        // if (0 == memcmp(uuid, provisionee_uuid, 4))
        // if (uuid[0] == 0x17 && uuid[14] == 0x28 && uuid[15] == 0x04) // RAL
        // if (uuid[14] == 0xa2 && uuid[15] == 0xbe) // RAL
        {
            uint32_t i = 0;

            for (i = 0; i < sizeof(s_provisioner_ctx) / sizeof(s_provisioner_ctx[0]); ++i)
            {
                if (s_provisioner_ctx[i].status != PROVISION_STATUS_IDLE &&
                    0 == memcmp(s_provisioner_ctx[i].peer_uuid_mac, uuid + 4, sizeof(s_provisioner_ctx[i].peer_uuid_mac)))
                {
                    // already provision
                    BT_WARN("already provision, %02X:%02X:%02X:%02X:%02X:%02X status %d",
                            s_provisioner_ctx[i].peer_uuid_mac[5],
                            s_provisioner_ctx[i].peer_uuid_mac[4],
                            s_provisioner_ctx[i].peer_uuid_mac[3],
                            s_provisioner_ctx[i].peer_uuid_mac[2],
                            s_provisioner_ctx[i].peer_uuid_mac[1],
                            s_provisioner_ctx[i].peer_uuid_mac[0],
                            s_provisioner_ctx[i].status);
                    return;
                }
            }

            for (i = 0; i < sizeof(s_provisioner_ctx) / sizeof(s_provisioner_ctx[0]); ++i)
            {
                if (s_provisioner_ctx[i].status == PROVISION_STATUS_IDLE)
                {
                    break;
                }
            }

            if (i >= sizeof(s_provisioner_ctx) / sizeof(s_provisioner_ctx[0]))
            {
                // prov is full, ignore
                BT_WARN("provision is full, ignore");
                return;
            }

            s_provisioner_ctx[i].status = PROVISION_STATUS_PROVISIONING;
            memcpy(s_provisioner_ctx[i].peer_uuid_mac, uuid + 4, 6);

            scanning_device.peer_addr = s_provisioner_ctx[i].peer_addr;
            memcpy(scanning_device.peer_uuid_mac, addr->a.val, 6);

            BT_WARN("scanning_device mac %02X:%02X:%02X:%02X:%02X:%02X",
                    scanning_device.peer_uuid_mac[5],
                    scanning_device.peer_uuid_mac[4],
                    scanning_device.peer_uuid_mac[3],
                    scanning_device.peer_uuid_mac[2],
                    scanning_device.peer_uuid_mac[1],
                    scanning_device.peer_uuid_mac[0]);

            ret = bt_mesh_provision_adv(uuid, s_netkey_idx, s_provisioner_ctx[i].peer_addr, 0);

            if (ret)
            {
                BT_ERR("bt_mesh_provision_adv ret err %d", ret);
                return;
            }

            // todo: log "Public keys are identical", indicate same dhkey
        }
    }
}

static void user_prov_complete(uint16_t netkey_idx, uint16_t addr)
{
    BT_WARN("netkey_idx %d addr %d", netkey_idx, addr);
    s_provision_ctx.local_addr = addr;
    s_provision_ctx.netkey_idx = netkey_idx;
}

static void user_prov_node_added(uint16_t netkey_idx, uint8_t uuid[16], uint16_t addr,
                                 uint8_t num_elem)
{
    BT_ERR("netkey_idx %d addr %d num %d uuid %s", netkey_idx, addr, num_elem, bt_hex(uuid, 16));

    if (provision_role == PROVISION_ROLE_PROVISIONER)
    {
        for (uint32_t i = 0; i < sizeof(s_provisioner_ctx) / sizeof(s_provisioner_ctx[0]); ++i)
        {
            if (0 == memcmp(s_provisioner_ctx[i].peer_uuid_mac, uuid + 4, sizeof(s_provisioner_ctx[i].peer_uuid_mac)) &&
                s_provisioner_ctx[i].status == PROVISION_STATUS_PROVISIONING &&
                addr == s_provisioner_ctx[i].peer_addr)
            {
                BT_WARN("success, mac %02X:%02X:%02X:%02X:%02X:%02X addr %d i %d", uuid[9], uuid[8], uuid[7], uuid[6], uuid[5], uuid[4],
                        addr, i);
                s_provisioner_ctx[i].status = PROVISION_STATUS_WAIT_CLOSE;

                return;
            }
        }
    }

    return;

    //    if (0)//provision_type == PROVISION_TYPE_AUTO)
    //    {
    //        LOG_INF("Device addr 0x%04x net_idx %d provisioned", prov_addr_index, net_idx);
    //        current_dev_addr = prov_addr_index++;
    //        //  k_sem_give(&prov_sem);
    //    }
}

static int my_cfg_mod_app_bind(uint16_t net_idx, uint16_t addr, uint16_t elem_addr,
                               uint16_t mod_app_idx, uint16_t mod_id)
{
    BT_ERR("my_cfg_mod_app_bind to 0x%04X mod_id: 0x%04X", addr, mod_id);
    int err = 0;
    uint8_t status = 0;
    err = bt_mesh_cfg_mod_app_bind(net_idx, addr, elem_addr, mod_app_idx,
                                   mod_id, &status);

    if (err)
    {
        BT_ERR("Unable to send Model App Bind (err %d) mod_id: 0x%04X", err, mod_id);
        deprovision_device_inter(addr);
        // reset_device_ctx(i);
        return -1;
    }

    if (status)
    {
        BT_ERR("Model App Bind failed with status 0x%02x mod_id: 0x%04X", status, mod_id);
        deprovision_device_inter(addr);
        // reset_device_ctx(i);
        return -1;
    }

    return 0;
}

static int my_cfg_mod_app_bind_vnd(uint16_t net_idx, uint16_t addr, uint16_t elem_addr,
                                   uint16_t mod_app_idx, uint16_t mod_id, uint16_t cid)
{
    BT_ERR("my_cfg_mod_app_bind_vnd to 0x%04X mod_id: 0x%04X, cid: 0x%04X", addr, mod_id, cid);
    int err = 0;
    uint8_t status = 0;
    err = bt_mesh_cfg_mod_app_bind_vnd(net_idx, addr, elem_addr, mod_app_idx,
                                       mod_id, cid, &status);

    if (err)
    {
        BT_ERR("Unable to send Model App Bind (err %d) mod_id: 0x%04X", err, mod_id);
        deprovision_device_inter(addr);
        // reset_device_ctx(i);
        return -1;
    }

    if (status)
    {
        BT_ERR("Model App Bind failed with status 0x%02x mod_id: 0x%04X", status, mod_id);
        deprovision_device_inter(addr);
        // reset_device_ctx(i);
        return -1;
    }

    return 0;
}

static int get_composition_data(uint16_t addr)
{
    BT_ERR("get_composition_data to 0x%04X", addr);
    NET_BUF_SIMPLE_DEFINE(buf, BT_MESH_RX_SDU_MAX);
    struct bt_mesh_comp_p0_elem elem;
    struct bt_mesh_comp_p0 comp;
    uint8_t page = 0x00;
    int err;

    err = bt_mesh_cfg_comp_data_get(s_net_idx, addr, page, &page,
                                    &buf);
    if (err)
    {
        BT_ERR("Getting composition failed (err %d)", err);
        return 0;
    }

    if (page != 0x00)
    {
        BT_WARN("Got page 0x%02x. No parser available.",
                page);
        return 0;
    }

    err = bt_mesh_comp_p0_get(&comp, &buf);
    if (err)
    {
        BT_ERR("Couldn't parse Composition data (err %d)",
               err);
        return 0;
    }

    BT_WARN("Got Composition Data for 0x%04x:", addr);
    BT_WARN("\tCID      0x%04x", comp.cid);
    BT_WARN("\tPID      0x%04x", comp.pid);
    BT_WARN("\tVID      0x%04x", comp.vid);
    BT_WARN("\tCRPL     0x%04x", comp.crpl);
    BT_WARN("\tFeatures 0x%04x", comp.feat);

    int elem_idx = 0;
    while (bt_mesh_comp_p0_elem_pull(&comp, &elem))
    {
        int i;

        BT_WARN("\tElement @ %d - %d:", elem.loc, elem_idx);

        if (elem.nsig)
        {
            BT_WARN("\t\tSIG Models:");
        }
        else
        {
            BT_WARN("\t\tNo SIG Models");
        }

        for (i = 0; i < elem.nsig; i++)
        {
            uint16_t mod_id = bt_mesh_comp_p0_elem_mod(&elem, i);

            BT_WARN("\t\t\t0x%04x", mod_id);
            if (mod_id >= BT_MESH_MODEL_ID_GEN_ONOFF_SRV &&
                my_cfg_mod_app_bind(s_net_idx, addr,
                                    addr + elem_idx, s_appkey_idx,
                                    mod_id) != 0)
            {
                return -1;
            }
        }

        if (elem.nvnd)
        {
            BT_WARN("\t\tVendor Models:");
        }
        else
        {
            BT_WARN("\t\tNo Vendor Models");
        }

        for (i = 0; i < elem.nvnd; i++)
        {
            struct bt_mesh_mod_id_vnd mod =
                bt_mesh_comp_p0_elem_mod_vnd(&elem, i);

            BT_WARN("\t\t\tCompany 0x%04x: 0x%04x",
                    mod.company, mod.id);

            if (my_cfg_mod_app_bind_vnd(s_net_idx, addr,
                                        addr + elem_idx, s_appkey_idx,
                                        mod.id, mod.company) != 0)
            {
                return -1;
            }
        }
        elem_idx++;
    }

    if (buf.len)
    {
        BT_WARN("\t\t...truncated data!");
    }

    return 0;
}

static int32_t do_add_appkey_cb(void *arg)
{
    int err = 0;
    uint8_t status = 0;
    uint32_t i = (typeof(i))arg;

    s_provisioner_ctx[i].status = PROVISION_STATUS_PROVISION_COMPL;

    for (int i = 2; i < 12; i++)
    {
        models[i].keys[0] = s_appkey_idx;
    }

    vnd_models[0].keys[0] = s_appkey_idx;
    // vnd_models[1].keys[0] = s_appkey_idx;

    BT_WARN("%d", i);

    err = bt_mesh_cfg_app_key_add(s_net_idx, s_provisioner_ctx[i].peer_addr, s_netkey_idx, s_appkey_idx, app_key, &status);

    if (err)
    {
        BT_ERR("Unable to send App Key Add (err %d)", err);
        deprovision_device_inter(s_provisioner_ctx[i].peer_addr);
        reset_device_ctx(i);
        return -1;
    }

    if (status)
    {
        BT_ERR("AppKeyAdd failed with status 0x%02x", status);
        deprovision_device_inter(s_provisioner_ctx[i].peer_addr);
        reset_device_ctx(i);
        return -1;
    }

    BT_WARN("AppKey send, NetKeyIndex 0x%04x AppKeyIndex 0x%04x", s_netkey_idx, s_appkey_idx);

    err = bt_mesh_app_key_add(s_appkey_idx, s_netkey_idx, app_key);

    if (err)
    {
        BT_ERR("App key add failed (err: %d)", err);
        return -1;
    }

    get_composition_data(s_provisioner_ctx[i].peer_addr);

    BT_WARN("bind success, %02X:%02X:%02X:%02X:%02X:%02X addr %d appaddr %d",
            s_provisioner_ctx[i].peer_uuid_mac[5],
            s_provisioner_ctx[i].peer_uuid_mac[4],
            s_provisioner_ctx[i].peer_uuid_mac[3],
            s_provisioner_ctx[i].peer_uuid_mac[2],
            s_provisioner_ctx[i].peer_uuid_mac[1],
            s_provisioner_ctx[i].peer_uuid_mac[0],
            s_provisioner_ctx[i].peer_addr, s_provisioner_ctx[i].peer_addr);

    set_gw_addr(s_provisioner_ctx[i].peer_addr);

    return 0;
}

static void user_link_close(bt_mesh_prov_bearer_t bearer)
{
    if (provision_role == PROVISION_ROLE_PROVISIONER)
    {
        for (uint32_t i = 0; i < sizeof(s_provisioner_ctx) / sizeof(s_provisioner_ctx[0]); ++i)
        {
            if (s_provisioner_ctx[i].status == PROVISION_STATUS_WAIT_CLOSE)
            {
                BT_WARN("success close, bear %d index %d", bearer, i);

                zephyr_ble_mesh_push_msg(BLE_MESH_MSG_FUNC, do_add_appkey_cb, sizeof(do_add_appkey_cb), (void *)i, sizeof(i));

                break;
            }
            else if (PROVISION_STATUS_PROVISIONING == s_provisioner_ctx[i].status)
            {
                BT_WARN("fail close, bear %d index %d", bearer, i);
                reset_device_ctx(i);
            }
        }
    }
}

static const struct bt_mesh_prov prov = {
    .uuid = dev_uuid,
    .output_size = 4,
    .output_actions = BT_MESH_DISPLAY_NUMBER,
    .output_number = user_output_number,
    .reset = user_prov_reset,
    .unprovisioned_beacon = user_unprovisioned_beacon,
    .complete = user_prov_complete,     // provisioner self/ provisionee callback
    .node_added = user_prov_node_added, // provisioner callback
    .link_close = user_link_close,
};

// static void setup_cdb(uint16_t netidx, uint16_t appidx)
// {
//     struct bt_mesh_cdb_app_key *key;

//     key = bt_mesh_cdb_app_key_alloc(netidx, appidx);

//     if (key == NULL)
//     {
//         BT_ERR("Failed to allocate app-key 0x%04x\n", appidx);
//         return;
//     }

//     bt_rand(key->keys[0].app_key, 16);

//     if (IS_ENABLED(CONFIG_BT_SETTINGS))
//     {
//         bt_mesh_cdb_app_key_store(key);
//     }
// }

// static void button_pressed(struct k_work *work)
// {
//     (void)setup_cdb;

//     if (bt_mesh_is_provisioned())
//     {
//         (void)gen_onoff_send(!onoff.val);
//         return;
//     }

//     //    self_provision();
// }

static void ble_mesh_provision_ready(int err)
{
    (void)user_output_number;

    if (err)
    {

        BT_ERR("Bluetooth init failed (err %d)", err);
        return;
    }

    BT_WARN("Bluetooth initialized");

    err = bt_mesh_init(&prov, &comp);

    if (err)
    {
        BT_ERR("Initializing mesh failed (err %d)", err);
        return;
    }

    if (IS_ENABLED(CONFIG_SETTINGS))
    {
        settings_load();
    }

    if (provision_role == PROVISION_ROLE_PROVISIONER)
    {
        //        self_provision();

        bt_rand(net_key, sizeof(net_key));

        BT_WARN("netkey %s", bt_hex(net_key, sizeof(net_key)));

        err = bt_mesh_cdb_create(net_key);

        if (err == -EALREADY)
        {
            BT_ERR("Using stored CDB");
        }
        else if (err)
        {
            BT_ERR("Failed to create CDB (err %d)", err);
            return;
        }

        s_provision_ctx.local_addr = 1 & BIT_MASK(15);
        err = bt_mesh_provision(net_key, s_netkey_idx, 0, 0, s_provision_ctx.local_addr, dev_key);

        if (err)
        {
            BT_ERR("Provisioning failed (err %d)", err);
            return;
        }
    }
    else if (provision_role == PROVISION_ROLE_PROVISIONEE)
    {
        bt_mesh_prov_enable(BT_MESH_PROV_ADV); // | BT_MESH_PROV_GATT);
    }

    BT_WARN("Mesh initialized");
}

static void bt_mesh_triggle_button(void)
{
    k_work_submit(&button_work);
}

int bt_mesh_provision_init(void)
{
    int err = 0;
    uint8_t *index = dev_uuid;

    (void)index;

    // k_work_init(&button_work, button_pressed);

    if (err)
    {
        BT_ERR("Board init failed (err: %d)\n", err);
        return err;
    }

    // k_work_init_delayable(&onoff.work, onoff_timeout);

    /* Initialize the Bluetooth Subsystem */
    err = bt_enable(ble_mesh_provision_ready);

    if (err)
    {
        BT_ERR("Bluetooth init failed (err %d)\n", err);
    }

    return err;
}

// static void hexstring_to_array(uint8_t *out, uint32_t *out_len, char *buff, uint8_t len)
//{
//     char temp[3] = {0};
//
//     uint32_t i = 0, j = 0;
//
//     for (i = 0; i < len && j < *out_len;)
//     {
//         memcpy(temp, buff + i, 2);
//         i = i + 2;
//         out[j++] = strtoul(temp, NULL, 16) & 0xFF;
//     }
//
//     *out_len = j;
// }

void bt_mesh_provision_shell(int32_t argc, char **argv)
{
    int ret = 0;
    uint8_t mac[6] = {0};

    if (argc <= 0)
    {
        BT_ERR("argc is 0");
        return;
    }

    if (!strcasecmp(argv[0], "init"))
    {
        if (argc < 2)
        {
            BT_ERR("param err, need provisioner or provisionee");
            return;
        }

        if (!strcasecmp(argv[1], "provisioner"))
        {
            bk_bluetooth_get_address(mac);
            memcpy(dev_uuid, provisioner_uuid, sizeof(provisioner_uuid));
            memcpy(dev_uuid + 4, mac, sizeof(mac));

            provision_role = PROVISION_ROLE_PROVISIONER;
            bt_mesh_provision_init();
        }
        else if (!strcasecmp(argv[1], "provisionee"))
        {
            if (provision_role == PROVISION_ROLE_UNKNOW)
            {
                bk_bluetooth_get_address(mac);
                memcpy(dev_uuid, provisionee_uuid, sizeof(provisionee_uuid));
                memcpy(dev_uuid + 4, mac, sizeof(mac));

                provision_role = PROVISION_ROLE_PROVISIONEE;
                bt_mesh_provision_init();
            }
            else if (provision_role == PROVISION_ROLE_PROVISIONEE)
            {
                bt_mesh_prov_enable(BT_MESH_PROV_ADV);
            }
        }
        else
        {
            BT_ERR("param err, need provisioner or provisionee");
            return;
        }
    }
    else if (!strcasecmp(argv[0], "deprovision"))
    {
        if (argc < 2)
        {
            BT_ERR("param err, need provisionee's addr");
            return;
        }

        uint32_t mac[6] = {0};
        uint8_t mac_final[6] = {0};
        uint32_t len = sizeof(mac);
        (void)(len);
        // sscanf bug: cant detect uint8_t size point
        ret = sscanf(argv[1], "%02x:%02x:%02x:%02x:%02x:%02x", // argv[1], "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
                     mac + 5,
                     mac + 4,
                     mac + 3,
                     mac + 2,
                     mac + 1,
                     mac);

        if (ret != 6)
        {
            BT_ERR("param err, need mac, %d %s", ret, argv[1]);
            return;
        }

        for (uint8_t i = 0; i < sizeof(mac_final) / sizeof(mac_final[0]); ++i)
        {
            mac_final[i] = mac[i];
        }

        BT_WARN("%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
                mac_final[5],
                mac_final[4],
                mac_final[3],
                mac_final[2],
                mac_final[1],
                mac_final[0]);

        deprovision_device(mac_final);
    }
    else if (!strcasecmp(argv[0], "click"))
    {
        bt_mesh_triggle_button();
    }
    else
    {
        BT_ERR("unknow cmd %s", argv[0]);
    }

    return;
}
