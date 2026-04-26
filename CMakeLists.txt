cmake_minimum_required(VERSION 3.20)
project(AmuseUnchained VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опции для сборки
option(USE_CUDA "Enable CUDA support for GPU acceleration" OFF)
option(USE_VULKAN "Enable Vulkan support" OFF)

# Поиск зависимостей (предполагается, что подмодули инициализированы)
# find_package(llama.cpp REQUIRED)
# find_package(stable-diffusion.cpp REQUIRED)
# find_package(OpenCV REQUIRED)
# find_package(imgui REQUIRED)

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/core/EngineManager.cpp
    src/core/HardwareProfiler.cpp
    src/library/ModelLibraryManager.cpp
    src/library/ModelScanner.cpp
    src/ui/AppUI.cpp
    src/utils/FileSystem.cpp
)

set(HEADERS
    src/core/EngineManager.h
    src/core/IModelEngine.h
    src/core/HardwareProfiler.h
    src/library/ModelLibraryManager.h
    src/library/ModelInfo.h
    src/library/ModelScanner.h
    src/ui/AppUI.h
    src/utils/FileSystem.h
    src/utils/Config.h
)

# Создаем исполняемый файл
add_executable(AmuseUnchained ${SOURCES} ${HEADERS})

# Подключение директорий
target_include_directories(AmuseUnchained PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Линковка библиотек (заглушки для примера)
# target_link_libraries(AmuseUnchained PRIVATE llama stable_diffusion opencv_core imgui)

if(USE_CUDA)
    target_compile_definitions(AmuseUnchained PRIVATE USE_CUDA)
    # find_package(CUDAToolkit REQUIRED)
    # target_link_libraries(AmuseUnchained PRIVATE CUDA::cudart)
endif()

if(USE_VULKAN)
    target_compile_definitions(AmuseUnchained PRIVATE USE_VULKAN)
endif()

# Пост-билд шаг: Создание структуры папок
add_custom_command(TARGET AmuseUnchained POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/models/text"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/models/image"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/models/video"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/models/lora"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/models/custom"
    COMMENT "Creating default models directory structure..."
)
