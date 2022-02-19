# Platform and compiler specific options
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_definitions (-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP /utf-8")
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEBUG_RUNTIME}")
    set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELEASE} ${RELEASE_RUNTIME} /fp:fast /Zi /GS-")
    set (CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELWITHDEBINFO})

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /utf-8")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_RUNTIME}")
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} ${RELEASE_RUNTIME} /fp:fast /Zi /GS- /D _SECURE_SCL=0")
    set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /DEBUG")
    set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")
endif()

# DirectX
if (WIN32)
    set (DIRECTX_REQUIRED_COMPONENTS)
    set (DIRECTX_OPTIONAL_COMPONENTS DInput DSound XInput)
    if (NOT MY3D_OPENGL)
        list (APPEND DIRECTX_REQUIRED_COMPONENTS D3D11)
    endif()
    find_package (DirectX REQUIRED ${DIRECTX_REQUIRED_COMPONENTS} OPTIONAL_COMPONENTS ${DIRECTX_OPTIONAL_COMPONENTS})

    if (DIRECTX_FOUND)
        include_directories (SYSTEM ${DIRECTX_INCLUDE_DIRS})   # These variables may be empty when WinSDK or MinGW is being used
        link_directories (${DIRECTX_LIBRARY_DIRS})
    endif ()
endif()

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

macro (define_dependency_libs TARGET)
    if (${TARGET} MATCHES SDL2|Engine)
        if (WIN32)
            list (APPEND LIBS user32 gdi32 winmm imm32 ole32 oleaut32 setupapi version uuid)
        endif()
    endif()

    if (MY3D_OPENGL)
        if (WIN32)
            list (APPEND LIBS opengl32)
        else()
            list (APPEND LIBS GL)
        endif()
    elseif (DIRECT3D_LIBRARIES)
        list (APPEND LIBS ${DIRECT3D_LIBRARIES})
    endif()
endmacro()

# macro for setting up a library target
macro(setup_library)
    cmake_parse_arguments(ARG "" "" "" ${ARGN})
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
    define_dependency_libs(${TARGET_NAME})
    target_link_libraries(${TARGET_NAME} ${ABSOLUTE_PATH_LIBS} ${LIBS})

endmacro()

macro(_export_headers)
    # add to third party headers
    set(THIRD_PARTY_INCLUDE_DIRS ${THIRD_PARTY_INCLUDE_DIRS} ${INCLUDE_DIRS} PARENT_SCOPE)
endmacro()

macro(setup_executable)
    cmake_parse_arguments(ARG "" "" "" ${ARGN})
    check_source_file()
    add_executable(${TARGET_NAME} ${ARG_UNPARSED_ARGUMENTS} ${SOURCE_FILES})
    _setup_target()
endmacro()


macro(setup_main_executable)
    cmake_parse_arguments(ARG "WIN32" "" "" ${ARGN})
    if (ARG_WIN32)
        set (EXEC_TYPE WIN32)
    endif()

    setup_executable(${EXEC_TYPE} ${ARG_UNPARSED_ARGUMENTS})
endmacro()