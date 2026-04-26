#include "EngineManager.h"
#include <iostream>

namespace Amuse {

EngineManager::EngineManager() 
    : m_currentModelType(ModelType::TEXT), m_currentModelId("") {}

EngineManager::~EngineManager() {
    shutdown();
}

void EngineManager::initialize(HardwareConfig config) {
    std::cout << "[EngineManager] Initializing with hardware config..." << std::endl;
    m_config = config;
    
    // Детекция железа и применение настроек
    m_profiler.detectHardware();
    m_profiler.setConfig(config);
    
    std::cout << "[EngineManager] Initialization complete." << std::endl;
}

bool EngineManager::createEngine(ModelType type, const std::string& modelPath) {
    std::cout << "[EngineManager] Creating engine for type: " 
              << (type == ModelType::TEXT ? "TEXT" : 
                  type == ModelType::IMAGE ? "IMAGE" : "VIDEO") 
              << std::endl;
    
    switch (type) {
        case ModelType::TEXT:
            m_textEngine = createTextEngine();
            if (m_textEngine && m_textEngine->loadModel(modelPath)) {
                m_currentModelType = ModelType::TEXT;
                m_currentModelId = modelPath;
                return true;
            }
            break;
            
        case ModelType::IMAGE:
            m_imageEngine = createImageEngine();
            if (m_imageEngine && m_imageEngine->loadModel(modelPath)) {
                m_currentModelType = ModelType::IMAGE;
                m_currentModelId = modelPath;
                return true;
            }
            break;
            
        case ModelType::VIDEO:
            m_videoEngine = createVideoEngine();
            if (m_videoEngine && m_videoEngine->loadModel(modelPath)) {
                m_currentModelType = ModelType::VIDEO;
                m_currentModelId = modelPath;
                return true;
            }
            break;
    }
    
    std::cerr << "[EngineManager] Failed to create engine for: " << modelPath << std::endl;
    return false;
}

bool EngineManager::switchModel(const std::string& modelId) {
    // TODO: Реализация переключения модели
    std::cout << "[EngineManager] Switching to model: " << modelId << std::endl;
    m_currentModelId = modelId;
    return true;
}

template<typename T>
std::shared_ptr<T> EngineManager::getEngine() const {
    // Шаблоная функция для получения движка
    // Реализация в заголовке или явная специализация
    return nullptr;
}

// Явные специализации шаблона
template<>
inline std::shared_ptr<ITextEngine> EngineManager::getEngine<ITextEngine>() const {
    return m_textEngine;
}

template<>
inline std::shared_ptr<IImageEngine> EngineManager::getEngine<IImageEngine>() const {
    return m_imageEngine;
}

template<>
inline std::shared_ptr<IVideoEngine> EngineManager::getEngine<IVideoEngine>() const {
    return m_videoEngine;
}

void EngineManager::updateHardwareConfig(const HardwareConfig& config) {
    std::cout << "[EngineManager] Updating hardware config..." << std::endl;
    m_config = config;
    m_profiler.setConfig(config);
    
    // Пересоздание движков с новыми настройками если нужно
    // TODO: Реализация hot-swap конфигурации
}

void EngineManager::shutdown() {
    std::cout << "[EngineManager] Shutting down all engines..." << std::endl;
    
    if (m_textEngine) {
        m_textEngine->unloadModel();
        m_textEngine.reset();
    }
    
    if (m_imageEngine) {
        m_imageEngine->unloadModel();
        m_imageEngine.reset();
    }
    
    if (m_videoEngine) {
        m_videoEngine->unloadModel();
        m_videoEngine.reset();
    }
    
    m_currentModelId = "";
    std::cout << "[EngineManager] Shutdown complete." << std::endl;
}

bool EngineManager::isEngineReady(ModelType type) const {
    switch (type) {
        case ModelType::TEXT:
            return m_textEngine != nullptr && m_textEngine->isModelLoaded();
        case ModelType::IMAGE:
            return m_imageEngine != nullptr && m_imageEngine->isModelLoaded();
        case ModelType::VIDEO:
            return m_videoEngine != nullptr && m_videoEngine->isModelLoaded();
    }
    return false;
}

std::string EngineManager::getEngineStatus() const {
    std::string status = "Engines Status:\n";
    status += "  TEXT: " + std::string(isEngineReady(ModelType::TEXT) ? "READY" : "NOT LOADED") + "\n";
    status += "  IMAGE: " + std::string(isEngineReady(ModelType::IMAGE) ? "READY" : "NOT LOADED") + "\n";
    status += "  VIDEO: " + std::string(isEngineReady(ModelType::VIDEO) ? "READY" : "NOT LOADED") + "\n";
    status += "  Current Model: " + m_currentModelId + "\n";
    return status;
}

std::shared_ptr<ITextEngine> EngineManager::createTextEngine() {
    // TODO: Интеграция с llama.cpp
    std::cout << "[EngineManager] Text engine stub created (llama.cpp integration pending)" << std::endl;
    return nullptr; // Заглушка до интеграции llama.cpp
}

std::shared_ptr<IImageEngine> EngineManager::createImageEngine() {
    // TODO: Интеграция со stable-diffusion.cpp
    std::cout << "[EngineManager] Image engine stub created (stable-diffusion.cpp integration pending)" << std::endl;
    return nullptr; // Заглушка до интеграции stable-diffusion.cpp
}

std::shared_ptr<IVideoEngine> EngineManager::createVideoEngine() {
    // TODO: Интеграция с ONNX Runtime / AnimateDiff
    std::cout << "[EngineManager] Video engine stub created (ONNX/AnimateDiff integration pending)" << std::endl;
    return nullptr; // Заглушка до интеграции видео-движка
}

} // namespace Amuse
