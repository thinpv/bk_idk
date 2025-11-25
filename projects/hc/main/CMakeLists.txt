set(cpp_incs
	json
	object
	feature
    gateway
	gateway/cloud
	device
	group
	scene
	rule
	rule/input
	rule/output
if(CONFIG_BT_MESH)
	device/ble
	device/ble/module
	device/ble/device
	protocol/ble
endif()
	database
	mqtt
	ota
	timer
	config
	util
    )

file(GLOB cpp_srcs 
	$(wildcard json/*.cpp)
	$(wildcard object/*.cpp)
	$(wildcard feature/*.cpp)
    $(wildcard gateway/*.cpp)
	$(wildcard gateway/cloud/*.cpp)
	$(wildcard gateway/rpc/*.cpp)
	$(wildcard device/*.cpp)
	$(wildcard group/*.cpp)
	$(wildcard scene/*.cpp)
	$(wildcard rule/*.cpp)
	$(wildcard rule/input/*.cpp)
	$(wildcard rule/output/*.cpp)
if(CONFIG_BT_MESH)
	$(wildcard device/ble/*.cpp)
	$(wildcard device/ble/module/*.cpp)
	$(wildcard device/ble/device/*.cpp)
	$(wildcard protocol/ble/*.cpp)
endif()
	$(wildcard database/*.cpp)
	$(wildcard mqtt/*.cpp)
	$(wildcard ota/*.cpp)
	$(wildcard timer/*.cpp)
	$(wildcard config/*.cpp)
	$(wildcard util/*.cpp)
    )


set(incs .
        mesh
        log
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
    log/Log.c
    )

if (CONFIG_OVERRIDE_FLASH_PARTITION)
    list(APPEND srcs
        vendor_flash.c
    )
endif()


armino_component_register(
	SRCS "vendor_flash.c" "armino_main.c" "${srcs}" "${cpp_srcs}"
	INCLUDE_DIRS "${incs}" "${cpp_incs}"
	PRIV_REQUIRES bk_init bk_cli mesh psa_mbedtls
)
