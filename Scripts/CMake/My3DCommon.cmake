
# platform predefine macros
function (define_platform_config)
    if (MSVC)
        add_definitions(-DPLATFORM_MSVC)
    elseif (MINGW)
        add_definitions(-DPLATFORM_MINGW)
    elseif(UNIX)
        add_definitions(-DPLATFORM_UNIX)
    endif()
endfunction()

#  macro for setting up a library target
macro(setup_library)
    cmake_parse_arguments(ARGS "" "" "" ${ARGN})
    check_source_file()
    add_library(${TARGET_NAME} ${ARG_UNPARSED_ARGUMENTS} ${SOURCE_FILES})
    _setup_target ()
endmacro()

# macro for pre-compiling header
macro(enable_pch HEADER_PATHNAME)
    set (EXT cpp)
    set (LANG CXX)
    if (IS_ABSOLUTE ${HEADER_PATHNAME})
        set (ABS_HEADER_PATHNAME ${HEADER_PATHNAME})
    else()
        set (ABS_HEADER_PATHNAME ${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_PATHNAME})
    endif()

    get_filename_component(HEADER_FILENAME ${HEADER_PATHNAME} NAME)
    if (CMAKE_COMPILER_IS_GUNCXX)
        # GUN g++
        set (PCH_FILENAME ${HEADER_FILENAME}.gch)
    else()
        # Clang or MSVC
        set (PCH_FILENAME ${HEADER_FILENAME}.pch)
    endif()
endmacro()

# macro for checking SOURCE_FILES variable is property initialized
macro(check_source_file)
    if (NOT SOURCE_FILES)
        if (NOT ${ARGN} STREQUAL "")
            message(FATAL_ERROR ${ARGN})
        else()
            message(FATAL_ERROR "Count not configure and generate project file because no source files have been defined yet.")
        endif()
    endif()
endmacro()

# Macro for defining source files
# RECURSE - Option to glob recursively
# GROUP - Option to group source files based on its relative files in current source directory and also by including the extra source files if provided
macro(define_source_files)
    cmake_parse_arguments(ARG "RECURSE;GROUP" "" "PCH;EXTRA_CPP_FILES;EXTRA_H_FILES;GLOB_CPP_PATTERNS;GLOB_H_PATTERNS" ${ARGN})
    if (NOT ARG_GLOB_CPP_PATTERNS)
        set (ARG_GLOB_CPP_PATTERNS *.cpp)
    endif()

    if (NOT ARG_GLOB_H_PATTERNS)
        set(ARG_GLOB_H_PATTERNS *.h)
    endif()

    if (ARG_RECURSE)
        set(ARG_RECURSE _RECURSE)
    else()
        unset(ARG_RECURSE)
    endif()

    file(GLOB${ARG_RECURSE} CPP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARG_GLOB_CPP_PATTERNS})
    file(GLOB${ARG_RECURSE} H_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARG_GLOB_H_PATTERNS})

    list(APPEND CPP_FILES ${ARG_EXTRA_CPP_FILES})
    list(APPEND H_FILES ${ARG_EXTRA_H_FILES})
    set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

    # Optionally enable PCH
    if (ARG_PCH)
        enable_pch(${ARG_PCH})
    endif()

    if (ARG_GROUP)
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCE_FILES})
    endif()
endmacro()

macro(_setup_target)
    # include directories
    target_include_directories(${TARGET_NAME} PUBLIC ${INCLUDE_DIRS})
    # link libraries
    target_link_libraries(${TARGET_NAME} ${ABSOLUTE_PATH_LIBS} ${LIBS})

endmacro()

macro(setup_executable)
    cmake_parse_arguments(ARG "" "" "" ${ARGN})
    check_source_file()
    add_executable(${TARGET_NAME} ${ARG_UNPARSED_ARGUMENTS} ${SOURCE_FILES})
    _setup_target()
endmacro()


macro(setup_main_executable)
    cmake_parse_arguments(ARG "WIN32" "" "" ${ARGN})
    if (WIN32)
        set (EXEC_TYPE WIN32)
    endif()

    setup_executable(${EXEC_TYPE} ${ARG_UNPARSED_ARGUMENTS})
endmacro()