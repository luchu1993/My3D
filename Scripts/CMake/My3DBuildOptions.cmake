# engine build options

# setup logging
set(MY3D_LOGGING ON CACHE BOOL "Enable logging subsystem.")
if (ENABLE_LOGGING)
    add_definitions(-DMY3D_LOGGING)
endif()

# setup testing
set(MY3D_TESTING ON CACHE BOOL "Enable testing.")
