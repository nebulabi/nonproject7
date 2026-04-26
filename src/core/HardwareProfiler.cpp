#include "HardwareProfiler.h"
#include <iostream>
#include <thread>
#include <algorithm>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/sysinfo.h>
#endif

namespace Amuse {

HardwareProfiler::HardwareProfiler() {}

HardwareInfo HardwareProfiler::detectHardware() {
    std::cout << "[HardwareProfiler] Detecting system hardware..." << std::endl;
    
    detectCPU(m_hardwareInfo);
    detectGPU_CUDA(m_hardwareInfo);
    detectGPU_Vulkan(m_hardwareInfo);
    detectRAM(m_hardwareInfo);
    
    // Автосохранение рекомендованной конфигурации
    m_config = recommendConfig(m_hardwareInfo);
    
    std::cout << "[HardwareProfiler] Detection complete." << std::endl;
    std::cout << "  CPU: " << m_hardwareInfo.cpuName 
              << " (" << m_hardwareInfo.cpuThreads << " threads)" << std::endl;
    std::cout << "  GPU: " << m_hardwareInfo.gpuName 
              << " (" << m_hardwareInfo.gpuVRAM << " MB VRAM)" << std::endl;
    std::cout << "  RAM: " << m_hardwareInfo.systemRAM << " MB" << std::endl;
    
    return m_hardwareInfo;
}

void HardwareProfiler::detectCPU(HardwareInfo& info) {
    info.cpuThreads = std::thread::hardware_concurrency();
    info.cpuCores = info.cpuThreads / 2; // Приблизительно
    
    #ifdef _WIN32
        // Windows: получение имени CPU через реестр (упрощенно)
        info.cpuName = "x86_64 CPU";
    #else
        // Linux: чтение из /proc/cpuinfo
        info.cpuName = "Linux CPU";
    #endif
    
    // По умолчанию используем половину потоков
    m_config.cpuThreads = std::max(2, info.cpuThreads / 2);
}

void HardwareProfiler::detectGPU_CUDA(HardwareInfo& info) {
    // Заглушка для CUDA
    // В реальной реализации: cudaGetDeviceProperties()
    
    #ifdef USE_CUDA
        info.hasCUDA = true;
        info.gpuName = "NVIDIA CUDA GPU";
        info.gpuVRAM = 8192; // Примерно 8GB
        m_config.useCUDA = true;
        
        std::cout << "[HardwareProfiler] CUDA detected!" << std::endl;
    #else
        info.hasCUDA = false;
        std::cout << "[HardwareProfiler] CUDA not enabled in build." << std::endl;
    #endif
}

void HardwareProfiler::detectGPU_Vulkan(HardwareInfo& info) {
    // Заглушка для Vulkan
    // В реальной реализации: Vulkan API enumeration
    
    #ifdef USE_VULKAN
        info.hasVulkan = true;
        if (info.gpuName.empty()) {
            info.gpuName = "Vulkan GPU";
            info.gpuVRAM = 4096;
        }
        m_config.useVulkan = true;
        
        std::cout << "[HardwareProfiler] Vulkan detected!" << std::endl;
    #else
        info.hasVulkan = false;
        std::cout << "[HardwareProfiler] Vulkan not enabled in build." << std::endl;
    #endif
}

void HardwareProfiler::detectRAM(HardwareInfo& info) {
    #ifdef _WIN32
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        info.systemRAM = status.ullTotalPhys / (1024 * 1024);
    #else
        struct sysinfo sys_inf;
        sysinfo(&sys_inf);
        info.systemRAM = sys_inf.totalram * sys_inf.mem_unit / (1024 * 1024);
    #endif
}

HardwareConfig HardwareProfiler::recommendConfig(const HardwareInfo& info) {
    HardwareConfig config;
    
    // Рекомендации на основе VRAM
    if (info.gpuVRAM >= 16000) {
        // Мощная система: 16GB+ VRAM
        config.gpuLoadPercent = 100;
        config.batchSize = 4;
        std::cout << "[HardwareProfiler] Powerful system detected - max GPU usage" << std::endl;
    }
    else if (info.gpuVRAM >= 8000) {
        // Средняя система: 8-12GB VRAM
        config.gpuLoadPercent = 75;
        config.batchSize = 2;
        std::cout << "[HardwareProfiler] Medium system detected - balanced GPU/CPU" << std::endl;
    }
    else if (info.gpuVRAM >= 4000) {
        // Слабая система: 4-6GB VRAM
        config.gpuLoadPercent = 50;
        config.batchSize = 1;
        std::cout << "[HardwareProfiler] Low VRAM detected - hybrid mode" << std::endl;
    }
    else {
        // Очень слабая или без GPU
        config.gpuLoadPercent = 0;
        config.batchSize = 1;
        std::cout << "[HardwareProfiler] No GPU or very low VRAM - CPU only mode" << std::endl;
    }
    
    // Настройка потоков CPU
    config.cpuThreads = std::max(2, info.cpuThreads - 2);
    
    // Включение доступных ускорителей
    config.useCUDA = info.hasCUDA;
    config.useVulkan = info.hasVulkan && !info.hasCUDA;
    
    return config;
}

void HardwareProfiler::setConfig(const HardwareConfig& config) {
    m_config = config;
    std::cout << "[HardwareProfiler] Manual config applied:" << std::endl;
    std::cout << "  GPU Load: " << config.gpuLoadPercent << "%" << std::endl;
    std::cout << "  CPU Threads: " << config.cpuThreads << std::endl;
    std::cout << "  CUDA: " << (config.useCUDA ? "ON" : "OFF") << std::endl;
    std::cout << "  Vulkan: " << (config.useVulkan ? "ON" : "OFF") << std::endl;
}

int HardwareProfiler::calculateGPULayers(int totalLayers, int gpuLoadPercent) {
    // Линейное распределение слоев на основе процента
    // Например: 32 слоя модели, 50% GPU = 16 слоев на GPU
    int layers = (totalLayers * gpuLoadPercent) / 100;
    
    // Округление до ближайшего четного числа (оптимизация для некоторых бэкендов)
    layers = (layers / 2) * 2;
    
    // Гарантируем минимум 0 и максимум totalLayers
    return std::max(0, std::min(layers, totalLayers));
}

} // namespace Amuse
