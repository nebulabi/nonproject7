#pragma once

#include "IModelEngine.h"
#include <memory>
#include <string>

namespace Amuse {

// Настройки распределения нагрузки CPU/GPU
struct HardwareConfig {
    // Процент нагрузки на GPU (0-100)
    // 0 = полностью CPU, 100 = полностью GPU
    int gpuLoadPercent;
    
    // Количество слоев для GPU (вычисляется автоматически)
    int gpuLayers;
    
    // Количество потоков CPU
    int cpuThreads;
    
    // Использование CUDA
    bool useCUDA;
    
    // Использование Vulkan
    bool useVulkan;
    
    // Размер батча
    int batchSize;
    
    HardwareConfig() 
        : gpuLoadPercent(50), gpuLayers(0), cpuThreads(4),
          useCUDA(false), useVulkan(false), batchSize(1) {}
};

// Информация о железе
struct HardwareInfo {
    std::string cpuName;
    int cpuCores;
    int cpuThreads;
    
    std::string gpuName;
    size_t gpuVRAM; // в МБ
    bool hasCUDA;
    bool hasVulkan;
    
    size_t systemRAM; // в МБ
    
    HardwareInfo() 
        : cpuCores(4), cpuThreads(4), gpuVRAM(0), 
          hasCUDA(false), hasVulkan(false), systemRAM(8192) {}
};

class HardwareProfiler {
public:
    HardwareProfiler();
    
    // Определение характеристик системы
    HardwareInfo detectHardware();
    
    // Рекомендация настроек на основе железа
    HardwareConfig recommendConfig(const HardwareInfo& info);
    
    // Ручная настройка
    void setConfig(const HardwareConfig& config);
    
    // Получение текущей конфигурации
    const HardwareConfig& getConfig() const { return m_config; }
    
    // Вычисление количества слоев для GPU на основе ползунка
    int calculateGPULayers(int totalLayers, int gpuLoadPercent);
    
private:
    HardwareInfo m_hardwareInfo;
    HardwareConfig m_config;
    
    // Детекция CPU
    void detectCPU(HardwareInfo& info);
    
    // Детекция GPU (CUDA)
    void detectGPU_CUDA(HardwareInfo& info);
    
    // Детекция GPU (Vulkan)
    void detectGPU_Vulkan(HardwareInfo& info);
    
    // Детекция RAM
    void detectRAM(HardwareInfo& info);
};

} // namespace Amuse
