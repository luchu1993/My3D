cmake_minimum_required(VERSION 3.10)
project(My3D)

set(CMAKE_CXX_STANDARD 14)

# compile options
option(LOGGING "Enable logging subsystem" ON)

if (LOGGING)
    add_definitions(-DMY3D_LOGGING)
endif()

add_subdirectory(Source)
