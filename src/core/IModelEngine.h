#pragma once

#include <string>
#include <memory>
#include <functional>

namespace Amuse {

enum class ModelType; // Forward declaration from ModelInfo.h

// Абстрактный интерфейс для всех движков
class IModelEngine {
public:
    virtual ~IModelEngine() = default;
    
    // Загрузка модели из файла
    virtual bool loadModel(const std::string& modelPath) = 0;
    
    // Выгрузка модели из памяти
    virtual void unloadModel() = 0;
    
    // Проверка загружена ли модель
    virtual bool isModelLoaded() const = 0;
    
    // Получение информации о текущей модели
    virtual std::string getModelInfo() const = 0;
};

// Специализация для текстовых моделей (LLM)
class ITextEngine : public IModelEngine {
public:
    // Генерация текста с промптом
    virtual std::string generateText(
        const std::string& prompt,
        int maxTokens = 512,
        float temperature = 0.8f,
        std::function<void(const std::string& token)> streamCallback = nullptr
    ) = 0;
    
    // Установка контекста диалога
    virtual void setContext(const std::string& context) = 0;
    
    // Очистка истории
    virtual void clearHistory() = 0;
};

// Специализация для изображений
class IImageEngine : public IModelEngine {
public:
    // Генерация изображения из текста
    virtual bool generateImage(
        const std::string& prompt,
        const std::string& negativePrompt = "",
        int width = 512,
        int height = 512,
        int steps = 20,
        float cfgScale = 7.0f,
        int seed = -1,
        std::function<void(int step, int total)> progressCallback = nullptr
    ) = 0;
    
    // Изображение из изображения (img2img)
    virtual bool imageToImage(
        const std::string& inputImagePath,
        const std::string& prompt,
        float strength = 0.75f,
        int steps = 20,
        std::function<void(int step, int total)> progressCallback = nullptr
    ) = 0;
    
    // Inpainting (дорисовка по маске)
    virtual bool inpaint(
        const std::string& inputImagePath,
        const std::string& maskPath,
        const std::string& prompt,
        int steps = 20,
        std::function<void(int step, int total)> progressCallback = nullptr
    ) = 0;
    
    // Сохранение последнего сгенерированного изображения
    virtual bool saveLastImage(const std::string& outputPath) = 0;
};

// Специализация для видео
class IVideoEngine : public IModelEngine {
public:
    // Генерация видео из изображения
    virtual bool generateVideoFromImage(
        const std::string& inputImagePath,
        int frames = 25,
        int fps = 8,
        std::function<void(int frame, int total)> progressCallback = nullptr
    ) = 0;
    
    // Генерация видео из текста (через промежуточное изображение)
    virtual bool generateVideoFromText(
        const std::string& prompt,
        int frames = 25,
        int fps = 8,
        std::function<void(int step, int total)> progressCallback = nullptr
    ) = 0;
    
    // Интерполяция кадров
    virtual bool interpolateFrames(
        const std::string& inputVideoPath,
        float multiplier = 2.0f,
        std::function<void(int frame, int total)> progressCallback = nullptr
    ) = 0;
    
    // Сохранение последнего сгенерированного видео
    virtual bool saveLastVideo(const std::string& outputPath) = 0;
};

} // namespace Amuse
