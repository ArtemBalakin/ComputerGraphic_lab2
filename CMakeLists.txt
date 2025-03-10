cmake_minimum_required(VERSION 3.20)
project(CG_Lab1)

set(CMAKE_CXX_STANDARD 17)
include_directories(.)

# DirectXTK и DirectXMath
include_directories("C:/Users/Stalker/Desktop/Новая папка (2)/DirectXTK/Inc")
include_directories("C:/Users/Stalker/Desktop/Новая папка (2)/DirectXMath/Inc")

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

add_executable(CG_Lab1
        Window.cpp
        Window.h
        Render.cpp
        Render.h
        main.cpp
        CelestialBody.cpp
        CelestialBody.h
        Camera.cpp
        Camera.h
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Линковка необходимых библиотек
set(LIBS
        gdi32
        d3d11
        d3dcompiler
        dxguid
        dinput8   # Для DirectInput
        d2d1      # Для Direct2D
        dwrite
        shlwapi# Для DirectWrite
)

target_link_libraries(CG_Lab1 PRIVATE ${LIBS})