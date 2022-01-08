
file (GLOB_RECURSE SAMPLE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(HelloWorld ${SAMPLE_SRCS})
target_link_libraries(HelloWorld Engine)
