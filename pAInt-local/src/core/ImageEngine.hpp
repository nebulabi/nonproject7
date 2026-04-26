#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <functional>

// Forward declaration
struct sd_ctx_t;

namespace paint {

// Результат генерации изображения
struct ImageGenerationResult {
    std::vector<uint8_t> imageData;  // RAW RGB данные
    int width = 0;
    int height = 0;
    float processingTime = 0.0f;
    bool success = true;
    std::string error;
    
    // Сохранить в файл
    bool saveToFile(const std::string& path) const;
};

class ImageEngine {
public:
    ImageEngine();
    ~ImageEngine();
    
    // Загрузка модели
    bool loadModel(const ModelDefinition& model, const HardwareConfig& hwConfig);
    
    // Выгрузка модели
    void unloadModel();
    
    // Проверка загруженности
    bool isLoaded() const;
    
    // Text-to-Image
    ImageGenerationResult textToImage(const GenerationParams& params);
    
    // Image-to-Image
    ImageGenerationResult imageToImage(const GenerationParams& params, const std::vector<uint8_t>& inputImage, int inputWidth, int inputHeight, float strength);
    
    // Inpainting
    ImageGenerationResult inpaint(const GenerationParams& params, 
                                   const std::vector<uint8_t>& inputImage, 
                                   const std::vector<uint8_t>& mask, 
                                   int width, int height);
    
    // Применение LoRA
    bool applyLoRA(const std::string& loraPath, float weight);
    
    // Применение настроек оборудования
    void updateHardwareConfig(const HardwareConfig& hwConfig);
    
    // Получить информацию о загруженной модели
    const ModelDefinition* getCurrentModel() const;
    
private:
    sd_ctx_t* ctx_ = nullptr;
    
    ModelDefinition currentModel_;
    HardwareConfig hwConfig_;
    bool loaded_ = false;
    
    // Внутренняя загрузка контекста
    bool initializeContext();
};

} // namespace paint
