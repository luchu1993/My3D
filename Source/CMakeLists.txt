define_platform_config()

# Check existence of various header files and their functions required by some of the third-party libraries and Urho3D library
# Set the CMake variables in this scope but only add the compiler defines in the respective library's scope


# Add targets
foreach (TARGET SDL2)
    add_subdirectory (ThirdParty/${TARGET})
endforeach ()

add_subdirectory(Engine)
add_subdirectory(Samples)

# Force utf-8 encoding for MSVC
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Testing
if (MY3D_TESTING)
    add_subdirectory(Testing)
endif()

