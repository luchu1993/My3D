
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

define_platform_config()

add_subdirectory(Engine)
add_subdirectory(Samples)
