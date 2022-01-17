# set cmake minimum version
cmake_minimum_required(VERSION 3.20)

# set project name
project(My3D)

# set cmake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Scripts/CMake)

include(My3DCommon)
include(My3DBuildOptions)

# set cpp standard
set(CMAKE_CXX_STANDARD 14)

# set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Binaries")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Binaries")

add_subdirectory(Source)
