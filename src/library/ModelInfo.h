#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Amuse {

enum class ModelType {
    TEXT,      // LLM (llama.cpp)
    IMAGE,     // Stable Diffusion
    VIDEO      // AnimateDiff / SVD
};

enum class PerformanceTier {
    WEAK,      // < 6GB VRAM / CPU only
    MEDIUM,    // 8-12GB VRAM
    POWERFUL   // 16GB+ VRAM
};

struct ModelInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string author;
    std::string filePath;
    std::string fileExtension; // .gguf, .safetensors, .onnx
    
    ModelType type;
    PerformanceTier tier;
    
    // Требования к памяти (в МБ)
    size_t requiredVRAM;
    size_t requiredRAM;
    
    // Теги для фильтрации
    std::vector<std::string> tags;
    
    // Пользовательская модель?
    bool isCustom;
    
    // Рейтинг/популярность (для встроенной библиотеки)
    float rating;
    int downloads;
    
    ModelInfo() : type(ModelType::TEXT), tier(PerformanceTier::MEDIUM), 
                  requiredVRAM(0), requiredRAM(0), isCustom(false), 
                  rating(0.0f), downloads(0) {}
};

} // namespace Amuse
