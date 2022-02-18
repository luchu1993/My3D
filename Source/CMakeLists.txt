define_platform_config()

# Check existence of various header files and their functions required by some of the third-party libraries and Urho3D library
# Set the CMake variables in this scope but only add the compiler defines in the respective library's scope


# Add third party targets
set(THIRD_PARTY_LIBS SDL2 LZ4 PugiXML ETCPACK)

foreach (LIB ${THIRD_PARTY_LIBS})
    add_subdirectory (ThirdParty/${LIB})
endforeach ()

# Header only target
set(THIRD_PARTY_HEADERS STB)
foreach (LIB ${THIRD_PARTY_HEADERS})
    add_subdirectory (ThirdParty/${LIB})
endforeach ()

# Engine source
add_subdirectory(Engine)

# Samples
add_subdirectory(Samples)

# Testing
if (MY3D_TESTING)
    add_subdirectory(Testing)
endif()

