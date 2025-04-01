cmake_minimum_required(VERSION 3.20)
project(CG_Lab1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Проверяем архитектуру
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Building for 64-bit architecture")
else()
    message(FATAL_ERROR "This project requires a 64-bit architecture. Please use a 64-bit MinGW.")
endif()

# Находим OpenMP
find_package(OpenMP REQUIRED)

# Находим остальные библиотеки
find_package(DirectXTex CONFIG REQUIRED)
find_package(DirectXMath CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

# Добавляем определения для Unicode
add_definitions(-DUNICODE -D_UNICODE)

# Создаём исполняемый файл
add_executable(CG_Lab1
        Window.cpp Window.h Render.cpp Render.h main.cpp CelestialBody.cpp CelestialBody.h
        FollowCamera.cpp FollowCamera.h Grid.cpp Grid.h ModelLoader.cpp ModelLoader.h
        Logger.cpp Logger.h
        Ground.cpp Ground.h
)

# Линкуем OpenMP, если он найден
if(OpenMP_CXX_FOUND)
    target_compile_options(CG_Lab1 PRIVATE ${OpenMP_CXX_FLAGS})
    target_link_libraries(CG_Lab1 PRIVATE OpenMP::OpenMP_CXX)
    target_link_options(CG_Lab1 PRIVATE ${OpenMP_CXX_FLAGS})
endif()

# Линкуем остальные библиотеки
target_link_libraries(CG_Lab1 PRIVATE
        Microsoft::DirectXTex
        Microsoft::DirectXMath
        d3d11
        d3dcompiler
        assimp::assimp
)

# Копируем текстуры в директорию сборки
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Textures/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Textures
        FILES_MATCHING
        PATTERN "*.obj"
        PATTERN "*.mtl"
        PATTERN "*.png"
        PATTERN "*.tif"
)

# Копируем шейдер в директорию сборки
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shader.hlsl DESTINATION ${CMAKE_CURRENT_BINARY_DIR})