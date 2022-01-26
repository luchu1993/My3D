
# Define target name
set (TARGET_NAME LZ4)

# Define include directory
set (LZ4_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (INCLUDE_DIRS ${LZ4_SOURCE_DIR}/include)

# Define source files
define_source_files (RECURSE GROUP GLOB_CPP_PATTERNS *.c)

# Setup target
setup_library ()

