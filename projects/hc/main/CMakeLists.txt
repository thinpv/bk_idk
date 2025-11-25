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
	protocol/uart
if(CONFIG_BT_MESH)
	device/ble
	device/ble/module
	device/ble/device
	protocol/ble
    protocol/ble/mesh
endif()
	database
	mqtt
	ota
	timer
	config
	util
    )

file(GLOB cpp_srcs 
	$(wildcard ./*.cpp)
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
	$(wildcard protocol/uart/*.cpp)
if(CONFIG_BT_MESH)
	$(wildcard device/ble/*.cpp)
	$(wildcard device/ble/module/*.cpp)
	$(wildcard device/ble/device/*.cpp)
	$(wildcard protocol/ble/*.cpp)
	$(wildcard protocol/ble/mesh/*.c)
	$(wildcard protocol/ble/mesh/*.cpp)
endif()
	$(wildcard database/*.cpp)
	$(wildcard mqtt/*.cpp)
	$(wildcard ota/*.cpp)
	$(wildcard timer/*.cpp)
	$(wildcard config/*.cpp)
	$(wildcard util/*.cpp)
    )


set(incs .
        log
    )

set(srcs
    armino_main.c
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
