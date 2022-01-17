set (SDL2_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (SDL2_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckCCompilerFlag)
include(CheckStructHasMember)
include(CMakeDependentOption)

include(${SDL2_SOURCE_DIR}/cmake/macros.cmake)
include(${SDL2_SOURCE_DIR}/cmake/sdlchecks.cmake)

set(SDL_MAJOR_VERSION 2)
set(SDL_MINOR_VERSION 0)
set(SDL_MICRO_VERSION 20)
set(SDL_INTERFACE_AGE 2)
set(SDL_BINARY_AGE 20)
set(SDL_VERSION "${SDL_MAJOR_VERSION}.${SDL_MINOR_VERSION}.${SDL_MICRO_VERSION}")

add_definitions(-DUSING_GENERATED_CONFIG_H)

include (CheckIncludeFile)

foreach (HEADER stdint.h inttypes.h malloc.h)
    string (TOUPPER HAVE_${HEADER} HAVE_HEADER)
    string (REPLACE . _ HAVE_HEADER ${HAVE_HEADER})
    check_include_file (${HEADER} ${HAVE_HEADER})
endforeach ()
include (CheckFunctionExists)
foreach (FUNCT __sincosf malloc_usable_size)
    string (TOUPPER HAVE_${FUNCT} HAVE_FUNCT)
    check_function_exists (${FUNCT} ${HAVE_FUNCT})
endforeach ()

if (WIN32)
    set(HAVE_LIBC TRUE)
    foreach(_HEADER stdio.h string.h wchar.h ctype.h math.h limits.h)
        string(TOUPPER "HAVE_${_HEADER}" _UPPER)
        string(REPLACE "." "_" _HAVE_H ${_UPPER})
        set(${_HAVE_H} 1)
    endforeach()
    set(HAVE_SIGNAL_H 1)
    foreach(_FN
            malloc calloc realloc free qsort abs memset memcpy memmove memcmp
            wcslen wcscmp
            strlen _strrev _strupr _strlwr strchr strrchr strstr itoa _ltoa
            _ultoa strtol strtoul strtoll strtod atoi atof strcmp strncmp
            _stricmp _strnicmp sscanf
            acos acosf asin asinf atan atanf atan2 atan2f ceil ceilf
            copysign copysignf cos cosf exp expf fabs fabsf floor floorf fmod fmodf
            log logf log10 log10f pow powf scalbn scalbnf sin sinf sqrt sqrtf tan tanf)
        string(TOUPPER ${_FN} _UPPER)
        set(HAVE_${_UPPER} 1)
    endforeach()
    set(HAVE_ALLOCA 1)
    set(HAVE_M_PI 1)
    add_definitions(-D_USE_MATH_DEFINES) # needed for M_PI
    set(STDC_HEADERS 1)
endif()

include (CheckLibraryExists)
check_library_exists (m sincosf "" HAVE_SINCOSF)

set (INCLUDE_DIRS ${SDL2_BINARY_DIR}/include/generated ${SDL2_SOURCE_DIR}/include)

# My3D - Always build internal sub-libraries as a STATIC library
set (SDL_STATIC_ENABLED_BY_DEFAULT ON)
set (SDL_SHARED_ENABLED_BY_DEFAULT OFF)
if (WIN32)
    add_definitions(-DSDL_STATIC_LIB)
endif()

# My3D - we prefer to disable 2D render subsystem by default

if (NOT DEFINED SDL_RENDER_ENABLED_BY_DEFAULT)
    set (SDL_RENDER_ENABLED_BY_DEFAULT OFF)
endif ()

set(SDL_SUBSYSTEMS
        Atomic Audio Video Render Events Power Threads Timers
        File Loadso CPUinfo Filesystem Dlopen )
foreach(_SUB ${SDL_SUBSYSTEMS})
    string(TOUPPER ${_SUB} _OPT)
    if (NOT DEFINED SDL_${_OPT}_ENABLED_BY_DEFAULT)
        set(SDL_${_OPT}_ENABLED_BY_DEFAULT ON)
    endif()
endforeach()

foreach(_SUB Joystick Haptic Sensor)
    string(TOUPPER ${_SUB} _OPT)
    if(NOT SDL_${_OPT})
        set(SDL_${_OPT}_DISABLED 1)
    endif()
endforeach()

set (SOURCE_FILE_PATTERNS
        ${SDL2_SOURCE_DIR}/src/*.c
        ${SDL2_SOURCE_DIR}/src/atomic/*.c
        ${SDL2_SOURCE_DIR}/src/libm/*.c
        ${SDL2_SOURCE_DIR}/src/stdlib/*.c
        ${SDL2_SOURCE_DIR}/src/thread/*.c
        ${SDL2_SOURCE_DIR}/src/timer/*.c)

foreach (_SUB AUDIO CPUINFO EVENTS FILE POWER RENDER VIDEO)
    string (TOLOWER ${_SUB} _DIR)
    list (APPEND SOURCE_FILE_PATTERNS ${SDL2_SOURCE_DIR}/src/${_DIR}/*.c)
endforeach ()

list (APPEND SOURCE_FILE_PATTERNS ${SDL2_SOURCE_DIR}/src/render/*/*.c)   # Recurse one more level
list (APPEND SOURCE_FILE_PATTERNS ${SDL2_SOURCE_DIR}/src/video/yuv2rgb/*.c)


file(GLOB SOURCE_FILES ${SOURCE_FILE_PATTERNS})

if(WIN32)
    set(HAVE_STDARG_H 1)
    set(HAVE_STDDEF_H 1)
endif()

if (WIN32)
    check_include_file (windows.h HAVE_WIN32_CC)

    file(GLOB CORE_SOURCES ${SDL2_SOURCE_DIR}/src/core/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${CORE_SOURCES})

    check_include_file(mmdeviceapi.h HAVE_MMDEVICEAPI_H)
    check_include_file(audioclient.h HAVE_AUDIOCLIENT_H)
    check_include_file(endpointvolume.h HAVE_ENDPOINTVOLUME_H)

    set(SDL_AUDIO_DRIVER_WINMM 1)
    file(GLOB WINMM_AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/winmm/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${WINMM_AUDIO_SOURCES})
    set(HAVE_SDL_AUDIO TRUE)

    set(SDL_VIDEO_DRIVER_WINDOWS 1)
    file(GLOB WIN_VIDEO_SOURCES ${SDL2_SOURCE_DIR}/src/video/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${WIN_VIDEO_SOURCES})

    if(RENDER_D3D AND HAVE_D3D_H)
        set(SDL_VIDEO_RENDER_D3D 1)
        set(HAVE_RENDER_D3D TRUE)
    endif()
    if(RENDER_D3D AND HAVE_D3D11_H)
        set(SDL_VIDEO_RENDER_D3D11 1)
        set(HAVE_RENDER_D3D TRUE)
    endif()

    set(HAVE_SDL_VIDEO TRUE)

    set(SDL_THREAD_WINDOWS 1)
    set(SOURCE_FILES ${SOURCE_FILES}
        ${SDL2_SOURCE_DIR}/src/thread/windows/SDL_sysmutex.c
        ${SDL2_SOURCE_DIR}/src/thread/windows/SDL_syssem.c
        ${SDL2_SOURCE_DIR}/src/thread/windows/SDL_systhread.c
        ${SDL2_SOURCE_DIR}/src/thread/windows/SDL_systls.c
        ${SDL2_SOURCE_DIR}/src/thread/generic/SDL_syscond.c)
    set(HAVE_SDL_THREADS TRUE)

    set(SDL_POWER_WINDOWS 1)
    set(SOURCE_FILES ${SOURCE_FILES} ${SDL2_SOURCE_DIR}/src/power/windows/SDL_syspower.c)
    set(HAVE_SDL_POWER TRUE)

    set(SDL_FILESYSTEM_WINDOWS 1)
    file(GLOB FILESYSTEM_SOURCES ${SDL2_SOURCE_DIR}/src/filesystem/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${FILESYSTEM_SOURCES})
    set(HAVE_SDL_FILESYSTEM TRUE)

    list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32)

    set(SDL_TIMER_WINDOWS 1)
    file(GLOB TIMER_SOURCES ${SDL2_SOURCE_DIR}/src/timer/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${TIMER_SOURCES})
    set(HAVE_SDL_TIMERS TRUE)

    set(SDL_LOADSO_WINDOWS 1)
    file(GLOB LOADSO_SOURCES ${SDL2_SOURCE_DIR}/src/loadso/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${LOADSO_SOURCES})
    set(HAVE_SDL_LOADSO TRUE)
endif()

configure_file("${SDL2_SOURCE_DIR}/include/SDL_config.h.cmake"
        "${SDL2_BINARY_DIR}/include/generated/SDL_config.h")

# Define target name
set (TARGET_NAME SDL2)

# Define source files
file (GLOB H_FILES include/*.h)     # Adding the headers into source files list is just for easier file browsing in the IDE
list (APPEND SOURCE_FILES ${H_FILES})

# Libraries for Win32
if (WIN32)
    set (LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32)
endif()

# Setup target as STATIC library (as the result we never use EXTRA_LDFLAGS linker flags)
setup_library ()