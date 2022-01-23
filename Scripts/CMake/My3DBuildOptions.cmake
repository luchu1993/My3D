# engine build options

# engine build type
set(MY3D_SHARED ON CACHE BOOL "Build shared engine")
if (MY3D_SHARED)
    add_definitions(-DMY3D_SHARED)
endif()

# setup logging
set(MY3D_LOGGING ON CACHE BOOL "Enable logging subsystem.")
if (MY3D_LOGGING)
    add_definitions(-DMY3D_LOGGING)
endif()

# setup testing
set(MY3D_TESTING ON CACHE BOOL "Enable testing.")
