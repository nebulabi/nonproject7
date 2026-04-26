#include "types.hpp"
#include <iostream>
#include <fstream>
#include <regex>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#else
    #include <sys/sysinfo.h>
    // Для GPU памяти потребуется CUDA или специфичные API
#endif

namespace paint {

HardwareConfig HardwareConfig::detect() {
    HardwareConfig config;
    
    // Определение доступной RAM
#ifdef _WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    config.maxRAM = statex.ullTotalPhys;
#else
    struct sysinfo info;
    sysinfo(&info);
    config.maxRAM = info.totalram * info.mem_unit;
#endif
    
    // Авто-настройка потоков CPU
    config.cpuThreads = std::max(1, (int)std::thread::hardware_concurrency() - 1);
    
    // Попытка определить VRAM (упрощённо)
    // В полной версии нужно использовать CUDA/ROCm/Vulkan API
    config.maxVRAM = 0;  // 0 означает "не определено, использовать CPU по умолчанию"
    
    // Рекомендации на основе доступной памяти
    if (config.maxRAM >= 32ULL * 1024 * 1024 * 1024) {
        config.tier = PerformanceTier::POWERFUL;
    } else if (config.maxRAM >= 16ULL * 1024 * 1024 * 1024) {
        config.tier = PerformanceTier::MEDIUM;
    } else {
        config.tier = PerformanceTier::WEAK;
    }
    
    return config;
}

} // namespace paint
