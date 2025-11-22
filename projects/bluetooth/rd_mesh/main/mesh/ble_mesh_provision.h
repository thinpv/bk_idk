#pragma once

#define RD_VENDOR_ID 0x0211
#define RD_VND_MODEL_CLIENT 0x0000
#define RD_VND_MODEL_SERVER 0x0001

#define BT_MESH_MODEL_OP_4(b0, cid) ((((b0) << 16) | 0xE20000) | (cid))            ///< Get opcode of Vendor Model
#define RD_VND_MODEL_OP_SEND_E2 BT_MESH_MODEL_OP_4(0x00, RD_VENDOR_ID)   ///< SEND Opcode of Vendor Model
#define RD_VND_MODEL_OP_STATUS_E2 BT_MESH_MODEL_OP_4(0x01, RD_VENDOR_ID) ///< STATUS Opcode Send of Vendor Model

#define BT_MESH_MODEL_OP_5(b0, cid) ((((b0) << 16) | 0xE00000) | (cid))            ///< Get Opcode of Vendor Model
#define RD_VND_MODEL_OP_SEND_E0 BT_MESH_MODEL_OP_5(0x00, RD_VENDOR_ID)   ///< SEND Opcode of Vendor Model
#define RD_VND_MODEL_OP_STATUS_E0 BT_MESH_MODEL_OP_5(0x01, RD_VENDOR_ID) ///< STATUS Opcode Send of Vendor Model

enum
{
    PROVISION_ROLE_UNKNOW,
    PROVISION_ROLE_PROVISIONER,
    PROVISION_ROLE_PROVISIONEE,
};

#define BEKEN_VND_PROVISIONER_UUID_HEAD RD_VENDOR_ID & 0xff, (RD_VENDOR_ID >> 8) & 0xff, 0x00 + 1, 0xfe
#define BEKEN_VND_PROVISIONEE_UUID_HEAD RD_VENDOR_ID & 0xff, (RD_VENDOR_ID >> 8) & 0xff, 0x00, 0xfe

enum
{
    PROVISION_STATUS_IDLE,
    PROVISION_STATUS_PROVISIONING,
    PROVISION_STATUS_WAIT_CLOSE,
    PROVISION_STATUS_PROVISION_COMPL,
};

int bt_mesh_provision_init(void);
void bt_mesh_provision_shell(int32_t argc, char **argv);

