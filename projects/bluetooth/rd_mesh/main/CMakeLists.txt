set(incs .
        ./mesh
    )
set(srcs
    armino_main.c
    mesh/mesh_demo_cli.c
    mesh/ble_mesh_provision.c
    )

if (CONFIG_OVERRIDE_FLASH_PARTITION)
    list(APPEND srcs
        vendor_flash.c
    )
endif()


armino_component_register(
	SRCS "vendor_flash.c" "armino_main.c" "${srcs}"
	INCLUDE_DIRS "${incs}"
	PRIV_REQUIRES bk_init bk_cli mesh
)
