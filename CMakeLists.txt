# set cmake minimum version
cmake_minimum_required(VERSION 3.20)

# set project name
project(My3D VERSION 0.1 LANGUAGES CXX)

# set cmake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Build/CMake)

# set cpp standard
set(CMAKE_CXX_STANDARD 14)

# set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Binaries")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Binaries")

# engine build type
set(ENGINE_BUILD_TYPE "SHARED" CACHE STRING "Specify My3D Engine build type, Shared (default) or Static" FORCE)
set_property(CACHE ENGINE_BUILD_TYPE PROPERTY STRINGS "STATIC" "SHARED")
message(STATUS "Engine build type = " ${ENGINE_BUILD_TYPE})
if (${ENGINE_BUILD_TYPE} STREQUAL "SHARED")
    add_definitions(-DENGINE_BUILD_SHARED)
else()
    add_definitions(-DENGINE_BUILD_STATIC)
endif()

# setup logging config
set(ENABLE_LOGGING ON CACHE BOOL "Enable logging subsystem." FORCE)
if (ENABLE_LOGGING)
    add_definitions(-DMY3D_LOGGING)
endif()


add_subdirectory(Source)
