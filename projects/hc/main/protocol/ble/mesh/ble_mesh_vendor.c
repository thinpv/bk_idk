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

#include "mbedtls/aes.h"
#include "BleMeshDefine.h"

extern struct bt_mesh_model models[];
extern struct bt_mesh_model vnd_models[];

static const uint8_t keyAes[16] = {0x44, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x40, 0x32, 0x38, 0x31, 0x31, 0x32, 0x38, 0x30, 0x34};
static int aes_encrypt(const uint8_t key[16], const uint8_t input[16], uint8_t output[16])
{
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 128);
    int ret = mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input, output);
    mbedtls_aes_free(&aes);
    return ret;
}

int set_gw_addr(uint16_t addr)
{
    BT_WARN("set_gw_addr to 0x%04X", addr);
    struct bt_mesh_msg_ctx ctx = {
        .app_idx = vnd_models[0].keys[0], /* Use the bound key */
        .addr = addr,
        .send_ttl = BT_MESH_TTL_DEFAULT,
    };

    if (ctx.app_idx == BT_MESH_KEY_UNUSED)
    {
        BT_INFO("The Generic OnOff Client must be bound to a key before sending.");
        return -ENOENT;
    }

    BT_MESH_MODEL_BUF_DEFINE(buf, RD_VND_MODEL_OP_SEND_E0, 4);
    bt_mesh_model_msg_init(&buf, RD_VND_MODEL_OP_SEND_E0);
    net_buf_simple_add_le16(&buf, RD_HEADER_PROVISION_SET_GW_ADDR);
    net_buf_simple_add_le16(&buf, PROV_OWN_ADDR);

    BT_WARN("Sending set_gw_addr");

    return bt_mesh_model_send(&vnd_models[0], &ctx, &buf, NULL, NULL);
}

int get_device_type(uint16_t addr, uint8_t *mac)
{
    BT_WARN("get_device_type to 0x%04X", addr);
    static uint8_t input[16] = {0x24, 0x02, 0x28, 0x04, 0x28, 0x11, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    static uint8_t output[16];
    memcpy(input + 8, mac, 6);
    memcpy(input + 14, (uint8_t *)&addr, 2);
    if (aes_encrypt(keyAes, input, output) != 0)
    {
        BT_ERR("AES encrypt failed");
        return -1;
    }

    struct bt_mesh_msg_ctx ctx = {
        .app_idx = vnd_models[0].keys[0], /* Use the bound key */
        .addr = addr,
        .send_ttl = BT_MESH_TTL_DEFAULT,
    };

    if (ctx.app_idx == BT_MESH_KEY_UNUSED)
    {
        BT_INFO("The Generic OnOff Client must be bound to a key before sending.");
        return -ENOENT;
    }

    BT_MESH_MODEL_BUF_DEFINE(buf, RD_VND_MODEL_OP_SEND_E0, 8);
    bt_mesh_model_msg_init(&buf, RD_VND_MODEL_OP_SEND_E0);
    net_buf_simple_add_le16(&buf, RD_HEADER_PROVISION_GET_DEV_TYPE);
    for (int i = 0; i < 6; i++)
    {
        net_buf_simple_add_u8(&buf, output[i + 10]);
    }

    BT_WARN("Sending get_device_type");

    return bt_mesh_model_send(&vnd_models[0], &ctx, &buf, NULL, NULL);
}