set(incs .
        ./mesh
    )
set(srcs
    armino_main.c
    mesh/mesh_demo_cli.c
    mesh/ble_mesh_provision.c
    mesh/ble_mesh_vendor.c
    mesh/ble_mesh_op_gen_battery_cli.c
    mesh/ble_mesh_op_gen_onoff_cli.c
    mesh/ble_mesh_op_light_ctl_cli.c
    mesh/ble_mesh_op_light_hsl_cli.c
    mesh/ble_mesh_op_light_lightness_cli.c
    mesh/ble_mesh_op_scene_cli.c
    mesh/ble_mesh_op_scheduler_cli.c
    mesh/ble_mesh_op_sensor_cli.c
    mesh/ble_mesh_op_time_cli.c
    mesh/ble_mesh_op_vnd.c
    )

if (CONFIG_OVERRIDE_FLASH_PARTITION)
    list(APPEND srcs
        vendor_flash.c
    )
endif()


armino_component_register(
	SRCS "vendor_flash.c" "armino_main.c" "${srcs}"
	INCLUDE_DIRS "${incs}"
	PRIV_REQUIRES bk_init bk_cli mesh psa_mbedtls
)
