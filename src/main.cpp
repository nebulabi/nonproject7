#include <iostream>
#include <filesystem>
#include "library/ModelLibraryManager.h"
#include "core/HardwareProfiler.h"
#include "core/EngineManager.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  Amuse Unchained v1.0 - C++ Edition" << std::endl;
    std::cout << "  Uncensored AI Generation Platform" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Определение пути к папке models
    std::string appPath = fs::current_path().string();
    std::string modelsPath = appPath + "/models";
    
    std::cout << "\n[Main] Application path: " << appPath << std::endl;
    std::cout << "[Main] Models directory: " << modelsPath << std::endl;
    
    // Инициализация менеджера библиотек
    Amuse::ModelLibraryManager libraryManager;
    libraryManager.initialize(modelsPath);
    
    // Показать доступные модели из каталога
    auto catalogModels = libraryManager.getCatalogModels();
    std::cout << "\n[Main] Built-in catalog has " << catalogModels.size() << " models available:" << std::endl;
    
    for (const auto& model : catalogModels) {
        std::string typeStr = (model.type == Amuse::ModelType::TEXT) ? "TEXT" :
                              (model.type == Amuse::ModelType::IMAGE) ? "IMAGE" : "VIDEO";
        std::string tierStr = (model.tier == Amuse::PerformanceTier::WEAK) ? "WEAK" :
                              (model.tier == Amuse::PerformanceTier::MEDIUM) ? "MEDIUM" : "POWERFUL";
        
        std::cout << "  - [" << typeStr << "][" << tierStr << "] " 
                  << model.name << " by " << model.author << std::endl;
        std::cout << "    Tags: ";
        for (size_t i = 0; i < model.tags.size() && i < 3; ++i) {
            std::cout << model.tags[i] << " ";
        }
        std::cout << "(Rating: " << model.rating << ")" << std::endl;
    }
    
    // Показать локальные модели
    auto localModels = libraryManager.getLocalModels();
    std::cout << "\n[Main] Found " << localModels.size() << " local models in " << modelsPath << std::endl;
    
    for (const auto& model : localModels) {
        std::cout << "  - " << model.name << " (" << model.fileExtension << ")" << std::endl;
    }
    
    // Профилирование железа
    Amuse::HardwareProfiler profiler;
    auto hardwareInfo = profiler.detectHardware();
    
    // Рекомендованная конфигурация
    auto config = profiler.recommendConfig(hardwareInfo);
    
    std::cout << "\n[Main] Recommended hardware configuration:" << std::endl;
    std::cout << "  GPU Load: " << config.gpuLoadPercent << "%" << std::endl;
    std::cout << "  CPU Threads: " << config.cpuThreads << std::endl;
    std::cout << "  CUDA: " << (config.useCUDA ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Vulkan: " << (config.useVulkan ? "Enabled" : "Disabled") << std::endl;
    
    // Инициализация менеджера движков
    Amuse::EngineManager engineManager;
    engineManager.initialize(config);
    
    // Пример фильтрации моделей
    std::cout << "\n[Main] Demo: Filtering models by type (IMAGE)..." << std::endl;
    auto imageModels = libraryManager.filterByType(Amuse::ModelType::IMAGE);
    std::cout << "  Found " << imageModels.size() << " image models (local)" << std::endl;
    
    // Пример поиска
    std::cout << "\n[Main] Demo: Searching for 'pony'..." << std::endl;
    // Для демонстрации добавим тестовую модель в поиск по каталогу
    bool found = false;
    for (const auto& model : catalogModels) {
        std::string lowerName = model.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        if (lowerName.find("pony") != std::string::npos) {
            std::cout << "  Found in catalog: " << model.name << std::endl;
            found = true;
        }
    }
    if (!found) {
        std::cout << "  Not found" << std::endl;
    }
    
    // Статус движков
    std::cout << "\n" << engineManager.getEngineStatus() << std::endl;
    
    // Демонстрация гибридного режима
    std::cout << "\n[Main] Hybrid mode demonstration:" << std::endl;
    std::cout << "  Slider at 0% = All CPU processing" << std::endl;
    std::cout << "  Slider at 50% = Hybrid CPU+GPU (balanced)" << std::endl;
    std::cout << "  Slider at 100% = All GPU processing" << std::endl;
    std::cout << "  Current setting: " << config.gpuLoadPercent << "%" << std::endl;
    
    int totalLayers = 32; // Пример для SDXL или Llama-7B
    int gpuLayers = profiler.calculateGPULayers(totalLayers, config.gpuLoadPercent);
    std::cout << "  For a " << totalLayers << "-layer model: " 
              << gpuLayers << " layers on GPU, " 
              << (totalLayers - gpuLayers) << " layers on CPU" << std::endl;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Ready! Place your models in /models/" << std::endl;
    std::cout << "  Supported: .gguf, .safetensors, .onnx" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
