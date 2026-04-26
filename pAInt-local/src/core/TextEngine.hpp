#pragma once

#include "core/types.hpp"
#include <string>
#include <functional>
#include <memory>

// Forward declarations для библиотек
struct llama_context;
struct llama_model;

namespace paint {

// Результат генерации текста
struct TextGenerationResult {
    std::string text;
    int tokensGenerated = 0;
    float processingTime = 0.0f;
    bool success = true;
    std::string error;
};

// Callback для потоковой генерации
using TextStreamCallback = std::function<void(const std::string& token)>;

class TextEngine {
public:
    TextEngine();
    ~TextEngine();
    
    // Загрузка модели
    bool loadModel(const ModelDefinition& model, const HardwareConfig& hwConfig);
    
    // Выгрузка модели
    void unloadModel();
    
    // Проверка загруженности
    bool isLoaded() const;
    
    // Генерация текста (полный ответ)
    TextGenerationResult generate(const GenerationParams& params);
    
    // Генерация с потоковым выводом
    TextGenerationResult generateStream(const GenerationParams& params, TextStreamCallback callback);
    
    // Применение настроек оборудования
    void updateHardwareConfig(const HardwareConfig& hwConfig);
    
    // Получить информацию о загруженной модели
    const ModelDefinition* getCurrentModel() const;
    
private:
    llama_model* model_ = nullptr;
    llama_context* ctx_ = nullptr;
    
    ModelDefinition currentModel_;
    HardwareConfig hwConfig_;
    bool loaded_ = false;
    
    // Инициализация контекста
    bool initializeContext();
};

} // namespace paint
