#include "ImageEngine.hpp"
#include <iostream>
#include <chrono>
#include <fstream>
#include <cstring>

// Включаем заголовки stable-diffusion.cpp
extern "C" {
    #include "stable-diffusion.h"
}

namespace paint {

bool ImageGenerationResult::saveToFile(const std::string& path) const {
    if (imageData.empty() || width == 0 || height == 0) {
        return false;
    }
    
    // Простое сохранение в BMP (для production лучше использовать stb_image_write или libpng)
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // BMP заголовок
    uint32_t fileSize = 54 + width * height * 3;
    uint32_t reserved = 0;
    uint32_t dataOffset = 54;
    
    // BMP файловый заголовок
    file.put('B');
    file.put('M');
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file.write(reinterpret_cast<const char*>(&reserved), 4);
    file.write(reinterpret_cast<const char*>(&dataOffset), 4);
    
    // BMP информационный заголовок (DIB)
    uint32_t dibHeaderSize = 40;
    int32_t w = width;
    int32_t h = -height;  // Отрицательная высота для top-down bitmap
    uint16_t planes = 1;
    uint16_t bpp = 24;
    uint32_t compression = 0;
    uint32_t imageSize = width * height * 3;
    int32_t xPixelsPerMeter = 0;
    int32_t yPixelsPerMeter = 0;
    uint32_t colorsUsed = 0;
    uint32_t colorsImportant = 0;
    
    file.write(reinterpret_cast<const char*>(&dibHeaderSize), 4);
    file.write(reinterpret_cast<const char*>(&w), 4);
    file.write(reinterpret_cast<const char*>(&h), 4);
    file.write(reinterpret_cast<const char*>(&planes), 2);
    file.write(reinterpret_cast<const char*>(&bpp), 2);
    file.write(reinterpret_cast<const char*>(&compression), 4);
    file.write(reinterpret_cast<const char*>(&imageSize), 4);
    file.write(reinterpret_cast<const char*>(&xPixelsPerMeter), 4);
    file.write(reinterpret_cast<const char*>(&yPixelsPerMeter), 4);
    file.write(reinterpret_cast<const char*>(&colorsUsed), 4);
    file.write(reinterpret_cast<const char*>(&colorsImportant), 4);
    
    // Данные изображения (BGR формат для BMP)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            // Конвертация RGB -> BGR
            file.put(imageData[idx + 2]);  // B
            file.put(imageData[idx + 1]);  // G
            file.put(imageData[idx + 0]);  // R
        }
        
        // Выравнивание по 4 байтам
        int padding = (4 - (width * 3) % 4) % 4;
        for (int p = 0; p < padding; ++p) {
            file.put(0);
        }
    }
    
    file.close();
    return true;
}

ImageEngine::ImageEngine() {}

ImageEngine::~ImageEngine() {
    unloadModel();
}

bool ImageEngine::loadModel(const ModelDefinition& model, const HardwareConfig& hwConfig) {
    if (loaded_) {
        unloadModel();
    }
    
    currentModel_ = model;
    hwConfig_ = hwConfig;
    
    // Определение пути к главному файлу
    std::string modelPath;
    if (model.isMultiFile()) {
        for (const auto& file : model.files) {
            if (file.type == "weights") {
                modelPath = file.path;
                break;
            }
        }
    } else {
        modelPath = model.mainFile;
    }
    
    if (modelPath.empty() || !std::filesystem::exists(modelPath)) {
        std::cerr << "[ImageEngine] Model file not found: " << modelPath << std::endl;
        return false;
    }
    
    std::cout << "[ImageEngine] Loading model: " << modelPath << std::endl;
    
    // Параметры для stable-diffusion.cpp
    // В реальной реализации нужно настроить n_threads и использование GPU
    enum sd_type_t type = SD_TYPE_F32;  // Или autodetect
    
    ctx_ = new_sd_ctx(modelPath.c_str(), 
                      nullptr,  // vae_path
                      nullptr,  // taesd_path
                      hwConfig_.useGPU,
                      sd_type_t::SD_TYPE_F16,  // Используем F16 для экономии памяти
                      -1,  // rng_type (auto)
                      sd_schedule_default,
                      nullptr);  // progress_callback
    
    if (!ctx_) {
        std::cerr << "[ImageEngine] Failed to load model" << std::endl;
        return false;
    }
    
    loaded_ = true;
    std::cout << "[ImageEngine] Model loaded successfully" << std::endl;
    return true;
}

bool ImageEngine::initializeContext() {
    // В stable-diffusion.cpp контекст создаётся сразу при загрузке модели
    return ctx_ != nullptr;
}

void ImageEngine::unloadModel() {
    if (ctx_) {
        free_sd_ctx(ctx_);
        ctx_ = nullptr;
    }
    
    loaded_ = false;
    std::cout << "[ImageEngine] Model unloaded" << std::endl;
}

bool ImageEngine::isLoaded() const {
    return loaded_ && ctx_ != nullptr;
}

ImageGenerationResult ImageEngine::textToImage(const GenerationParams& params) {
    ImageGenerationResult result;
    
    if (!loaded_) {
        result.success = false;
        result.error = "Model not loaded";
        return result;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Настройки генерации
    int width = params.width > 0 ? params.width : 512;
    int height = params.height > 0 ? params.height : 512;
    int steps = params.steps > 0 ? params.steps : 20;
    float cfg = params.cfgScale > 0 ? params.cfgScale : 7.0f;
    int seed = params.seed >= 0 ? params.seed : static_cast<int>(std::time(nullptr));
    
    // Генерация через stable-diffusion.cpp
    // Функция txt2img возвращает массив пикселей
    uint8_t* imageData = txt2img(ctx_,
                                  params.prompt.c_str(),
                                  params.negativePrompt.c_str(),
                                  steps,
                                  width,
                                  height,
                                  seed,
                                  1,  // batch_count
                                  cfg,
                                  -1.0f,  // sample negative guidance
                                  -1, -1, -1,  // init_image, mask, strength
                                  nullptr);  // callback
    
    if (!imageData) {
        result.success = false;
        result.error = "Generation failed";
        return result;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = endTime - startTime;
    
    // Копирование данных
    result.imageData.assign(imageData, imageData + width * height * 3);
    result.width = width;
    result.height = height;
    result.processingTime = duration.count();
    
    // Освобождение памяти
    delete[] imageData;
    
    return result;
}

ImageGenerationResult ImageEngine::imageToImage(const GenerationParams& params, 
                                                  const std::vector<uint8_t>& inputImage,
                                                  int inputWidth, int inputHeight,
                                                  float strength) {
    ImageGenerationResult result;
    
    if (!loaded_) {
        result.success = false;
        result.error = "Model not loaded";
        return result;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    int width = params.width > 0 ? params.width : inputWidth;
    int height = params.height > 0 ? params.height : inputHeight;
    int steps = params.steps > 0 ? params.steps : 20;
    float cfg = params.cfgScale > 0 ? params.cfgScale : 7.0f;
    int seed = params.seed >= 0 ? params.seed : static_cast<int>(std::time(nullptr));
    
    // Генерация img2img
    uint8_t* imageData = txt2img(ctx_,
                                  params.prompt.c_str(),
                                  params.negativePrompt.c_str(),
                                  steps,
                                  width,
                                  height,
                                  seed,
                                  1,
                                  cfg,
                                  -1.0f,
                                  inputImage.data(),
                                  nullptr,  // no mask
                                  strength);
    
    if (!imageData) {
        result.success = false;
        result.error = "Generation failed";
        return result;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = endTime - startTime;
    
    result.imageData.assign(imageData, imageData + width * height * 3);
    result.width = width;
    result.height = height;
    result.processingTime = duration.count();
    
    delete[] imageData;
    
    return result;
}

ImageGenerationResult ImageEngine::inpaint(const GenerationParams& params,
                                             const std::vector<uint8_t>& inputImage,
                                             const std::vector<uint8_t>& mask,
                                             int width, int height) {
    ImageGenerationResult result;
    
    if (!loaded_) {
        result.success = false;
        result.error = "Model not loaded";
        return result;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    int steps = params.steps > 0 ? params.steps : 20;
    float cfg = params.cfgScale > 0 ? params.cfgScale : 7.0f;
    int seed = params.seed >= 0 ? params.seed : static_cast<int>(std::time(nullptr));
    
    // Генерация inpainting
    uint8_t* imageData = txt2img(ctx_,
                                  params.prompt.c_str(),
                                  params.negativePrompt.c_str(),
                                  steps,
                                  width,
                                  height,
                                  seed,
                                  1,
                                  cfg,
                                  -1.0f,
                                  inputImage.data(),
                                  mask.data(),
                                  1.0f);  // full strength for inpaint
    
    if (!imageData) {
        result.success = false;
        result.error = "Generation failed";
        return result;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = endTime - startTime;
    
    result.imageData.assign(imageData, imageData + width * height * 3);
    result.width = width;
    result.height = height;
    result.processingTime = duration.count();
    
    delete[] imageData;
    
    return result;
}

bool ImageEngine::applyLoRA(const std::string& loraPath, float weight) {
    if (!loaded_) {
        return false;
    }
    
    // В stable-diffusion.cpp есть функция для применения LoRA
    // apply_lora(ctx_, loraPath.c_str(), weight);
    
    std::cout << "[ImageEngine] Applying LoRA: " << loraPath << " (weight: " << weight << ")" << std::endl;
    
    // Заглушка для будущей реализации
    return true;
}

void ImageEngine::updateHardwareConfig(const HardwareConfig& hwConfig) {
    hwConfig_ = hwConfig;
    
    if (loaded_) {
        std::cout << "[ImageEngine] Hardware config updated, reload may be required" << std::endl;
    }
}

const ModelDefinition* ImageEngine::getCurrentModel() const {
    return loaded_ ? &currentModel_ : nullptr;
}

} // namespace paint
