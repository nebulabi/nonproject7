cmake_minimum_required(VERSION 3.20)
project(pAInt_local VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опции сборки
option(USE_CUDA "Enable CUDA support for GPU acceleration" ON)
option(USE_VULKAN "Enable Vulkan support" OFF)

# --- Зависимости (FetchContent) ---
include(FetchContent)

# 1. SDL2
FetchContent_Declare(
  sdl2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.30.0
)
FetchContent_MakeAvailable(sdl2)

# 2. llama.cpp (для текста)
FetchContent_Declare(
  llama_cpp
  GIT_REPOSITORY https://github.com/ggerganov/llama.cpp.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(llama_cpp)

# 3. stable-diffusion.cpp (для изображений)
FetchContent_Declare(
  sd_cpp
  GIT_REPOSITORY https://github.com/leejet/stable-diffusion.cpp.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)
set(SD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SD_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sd_cpp)

# --- Исходный код проекта ---
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")

add_executable(pAInt_local ${SOURCES})

target_include_directories(pAInt_local PRIVATE 
    ${sdl2_SOURCE_DIR}/include
    ${llama_cpp_SOURCE_DIR}
    ${sd_cpp_SOURCE_DIR}
    src/ui
)

# Линковка
target_link_libraries(pAInt_local PRIVATE 
    SDL2::SDL2
    ggml
    stable-diffusion
)

# Настройки компилятора
if(MSVC)
    target_compile_options(pAInt_local PRIVATE /W4 /WX-)
else()
    target_compile_options(pAInt_local PRIVATE -Wall -Wextra -pedantic)
endif()

if(USE_CUDA)
    target_compile_definitions(pAInt_local PRIVATE GGML_USE_CUDA)
endif()

# Копирование папки models при сборке
add_custom_command(TARGET pAInt_local POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/models
    $<TARGET_FILE_DIR:pAInt_local>/models
)
