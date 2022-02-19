
# Define target name
set(TARGET_NAME Engine)

# Define source files
define_source_files(RECURSE GROUP)
add_definitions(-DBUILD_ENGINE)

# Define include directory
set (INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND INCLUDE_DIRS ${THIRD_PARTY_INCLUDE_DIRS})

# Define libraries
set (LIBS ${THIRD_PARTY_LIBS})

# Define build type
set(ENGINE_BUILD_TYPE STATIC)
if (MY3D_SHARED)
    set(ENGINE_BUILD_TYPE SHARED)
endif()

# Setup target
setup_library(${ENGINE_BUILD_TYPE})
