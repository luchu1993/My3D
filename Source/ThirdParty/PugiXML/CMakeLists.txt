# Define target name
set (TARGET_NAME PugiXML)

# Define source files
define_source_files(GLOB_CPP_PATTERNS src/*.cpp GLOB_CPP_PATTERNS src/*.cpp GLOB_H_PATTERNS src/*.hpp)

# Setup target
setup_library()
