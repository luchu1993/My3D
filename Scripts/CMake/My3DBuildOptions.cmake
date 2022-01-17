# engine build options
# setup logging config
set(ENABLE_LOGGING ON CACHE BOOL "Enable logging subsystem." FORCE)
if (ENABLE_LOGGING)
    add_definitions(-DMY3D_LOGGING)
endif()
