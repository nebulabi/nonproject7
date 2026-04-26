#pragma once

#include "IModelEngine.h"
#include "HardwareProfiler.h"
#include "../library/ModelInfo.h"
#include <memory>
#include <string>
#include <map>

namespace Amuse {

class EngineManager {
public:
    EngineManager();
    ~EngineManager();
    
    // Инициализация менеджера
    void initialize(HardwareConfig config);
    
    // Создание движка для типа модели
    bool createEngine(ModelType type, const std::string& modelPath);
    
    // Переключение активной модели
    bool switchModel(const std::string& modelId);
    
    // Получение активного движка
    template<typename T>
    std::shared_ptr<T> getEngine() const;
    
    // Обновление конфигурации оборудования на лету
    void updateHardwareConfig(const HardwareConfig& config);
    
    // Освобождение всех ресурсов
    void shutdown();
    
    // Статус движков
    bool isEngineReady(ModelType type) const;
    std::string getEngineStatus() const;
    
private:
    std::shared_ptr<ITextEngine> m_textEngine;
    std::shared_ptr<IImageEngine> m_imageEngine;
    std::shared_ptr<IVideoEngine> m_videoEngine;
    
    HardwareConfig m_config;
    HardwareProfiler m_profiler;
    
    ModelType m_currentModelType;
    std::string m_currentModelId;
    
    // Фабричные методы для создания движков
    std::shared_ptr<ITextEngine> createTextEngine();
    std::shared_ptr<IImageEngine> createImageEngine();
    std::shared_ptr<IVideoEngine> createVideoEngine();
};

} // namespace Amuse
