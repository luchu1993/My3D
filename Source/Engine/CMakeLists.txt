set(TARGET_NAME Engine)

define_source_files(RECURSE GROUP)
add_definitions(-DBUILD_ENGINE)

set (INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set (LIBS SDL2)

setup_library(${ENGINE_BUILD_TYPE})
