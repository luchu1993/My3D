
file (GLOB_RECURSE ENGINE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file (GLOB_RECURSE ENGINE_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_definitions(-DBUILD_ENGINE)
set (ENGINE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Engine SHARED
    ${ENGINE_SRCS}
    ${ENGINE_HDRS})

target_include_directories(Engine PUBLIC ${ENGINE_INCLUDE_DIR})

# set engine link libs
set (ENGINE_LINK_LIBS)
if (MSVC OR MINGW)
    list(APPEND ENGINE_LINK_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32)
endif()

target_link_libraries(Engine ${ENGINE_LINK_LIBS})
