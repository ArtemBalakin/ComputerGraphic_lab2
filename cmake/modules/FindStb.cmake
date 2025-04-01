# FindStb.cmake
find_path(STB_INCLUDE_DIR
        NAMES stb_image.h
        PATHS
        "${CMAKE_SOURCE_DIR}/thirdparty/stb"
        "C:/Users/Stalker/Desktop/vcpkg/vcpkg/installed/x64-mingw-static/include"
)

if(STB_INCLUDE_DIR)
    set(STB_FOUND TRUE)
else()
    set(STB_FOUND FALSE)
endif()

if(STB_FOUND)
    if(NOT TARGET stb::stb)
        add_library(stb::stb INTERFACE IMPORTED)
        set_target_properties(stb::stb PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${STB_INCLUDE_DIR}"
        )
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Stb DEFAULT_MSG STB_INCLUDE_DIR)