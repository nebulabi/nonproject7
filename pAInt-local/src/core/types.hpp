#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>

namespace paint {

// Типы моделей
enum class ModelType {
    TEXT,      // LLM (llama.cpp)
    IMAGE,     // Stable Diffusion
    VIDEO      // Video Diffusion / AnimateDiff
};

// Категория производительности
enum class PerformanceTier {
    WEAK,      // <6GB VRAM, CPU-focused
    MEDIUM,    // 8-12GB VRAM
    POWERFUL   // 16GB+ VRAM
};

// Информация о файле модели
struct ModelFile {
    std::string path;
    std::string type;  // "weights", "vae", "encoder", "config", etc.
    bool required = true;
};

// Определение модели (поддержка мульти-файловых структур)
struct ModelDefinition {
    std::string id;
    std::string name;
    std::string author;
    std::string description;
    
    ModelType type;
    PerformanceTier tier;
    
    // Для однофайловых моделей (.gguf, .safetensors)
    std::string mainFile;
    
    // Для многофайловых моделей (папки с компонентами)
    std::vector<ModelFile> files;
    
    // Метаданные
    std::map<std::string, std::string> tags;
    size_t estimatedVRAM = 0;  // в байтах
    size_t estimatedRAM = 0;   // в байтах
    
    // Путь к папке модели
    std::filesystem::path basePath;
    
    bool isMultiFile() const {
        return !files.empty();
    }
};

// Конфигурация оборудования
struct HardwareConfig {
    bool useGPU = true;
    float gpuSplit = 1.0f;  // 0.0 = CPU, 1.0 = GPU, 0.5 = гибридный
    
    int cpuThreads = 4;
    int gpuLayers = -1;     // -1 = все возможные
    
    size_t maxVRAM = 0;
    size_t maxRAM = 0;
    
    // Авто-определение доступной памяти
    static HardwareConfig detect();
};

// Параметры генерации
struct GenerationParams {
    // Общие
    int32_t seed = -1;
    int steps = 20;
    float cfgScale = 7.0f;
    
    // Для текста
    int maxTokens = 512;
    float temperature = 0.8f;
    
    // Для изображений
    int width = 512;
    int height = 512;
    std::string prompt;
    std::string negativePrompt;
    std::string sampler = "euler_a";
    
    // Для видео
    int frames = 16;
    float fps = 8.0f;
};

} // namespace paint
