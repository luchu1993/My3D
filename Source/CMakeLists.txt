define_platform_config()

# Add third party targets
set(THIRD_PARTY_LIBS SDL2 LZ4 PugiXML ETCPACK)

foreach (LIB ${THIRD_PARTY_LIBS})
    add_subdirectory (ThirdParty/${LIB})
endforeach ()

# Header only target
set(THIRD_PARTY_INCLUDE_DIRS)
set(HEADER_ONLY_LIBS STB)
foreach (LIB ${HEADER_ONLY_LIBS})
    add_subdirectory(ThirdParty/${LIB})
endforeach ()

# Engine source
add_subdirectory(Engine)

# Samples
add_subdirectory(Samples)

# Testing
if (MY3D_TESTING)
    add_subdirectory(Testing)
endif()

