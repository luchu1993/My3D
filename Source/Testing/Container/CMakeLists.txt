set(TARGET_NAME TestContainer)

set(LIBS Engine)
define_source_files()

setup_main_executable()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Catch)
