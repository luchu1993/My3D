
# Define target name
set (TARGET_NAME PugiXML)

# Define include directory
set (PUGIXML_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (INCLUDE_DIRS ${PUGIXML_SOURCE_DIR}/include)

# Define source files
define_source_files(RECURSE GROUP GLOB_CPP_PATTERNS *.cpp GLOB_H_PATTERNS *.hpp)

# Setup target
setup_library()
