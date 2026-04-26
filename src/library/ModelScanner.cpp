#include "ModelScanner.h"
#include <fstream>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace Amuse {

ModelScanner::ModelScanner() {}

std::vector<ModelInfo> ModelScanner::scanDirectory(const std::string& basePath) {
    std::vector<ModelInfo> models;
    
    if (!fs::exists(basePath)) {
        std::cerr << "[ModelScanner] Directory not found: " << basePath << std::endl;
        return models;
    }
    
    // Рекурсивный обход папки models/
    for (const auto& entry : fs::recursive_directory_iterator(basePath)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            std::string ext = entry.path().extension().string();
            
            // Поддерживаемые расширения
            if (ext == ".gguf" || ext == ".safetensors" || ext == ".onnx" || 
                ext == ".ckpt" || ext == ".bin") {
                
                ModelInfo info = analyzeModelFile(path);
                if (isValidModel(path)) {
                    models.push_back(info);
                    std::cout << "[ModelScanner] Found: " << info.name << " (" 
                              << entry.path().relative_path().string() << ")" << std::endl;
                }
            }
        }
    }
    
    return models;
}

ModelInfo ModelScanner::analyzeModelFile(const std::string& filePath) {
    ModelInfo info;
    info.filePath = filePath;
    info.fileExtension = fs::path(filePath).extension().string();
    info.isCustom = true; // Все найденные в папке считаем пользовательскими
    
    // Определение типа
    info.type = detectModelType(filePath);
    
    // Парсинг имени файла
    parseFileName(info, filePath);
    
    // Оценка требований
    info.tier = estimateTier(filePath, info.type);
    
    // Извлечение метаданных (если есть)
    extractMetadata(info, filePath);
    
    return info;
}

ModelType ModelScanner::detectModelType(const std::string& filePath) {
    std::string ext = fs::path(filePath).extension().string();
    std::string filename = fs::path(filePath).filename().string();
    std::string lowerName = filename;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    // Текстовые модели (LLM)
    if (ext == ".gguf") {
        return ModelType::TEXT;
    }
    
    // Изображения (Stable Diffusion)
    if (ext == ".safetensors" || ext == ".ckpt") {
        if (lowerName.find("xl") != std::string::npos || 
            lowerName.find("sd1") != std::string::npos ||
            lowerName.find("sd2") != std::string::npos ||
            lowerName.find("pony") != std::string::npos) {
            return ModelType::IMAGE;
        }
    }
    
    // Видео модели
    if (lowerName.find("svd") != std::string::npos || 
        lowerName.find("animate") != std::string::npos ||
        lowerName.find("video") != std::string::npos) {
        return ModelType::VIDEO;
    }
    
    // По умолчанию определяем по расширению
    if (ext == ".onnx") {
        // ONNX может быть чем угодно, проверяем имя
        if (lowerName.find("diffusion") != std::string::npos) {
            return ModelType::IMAGE;
        }
        return ModelType::VIDEO;
    }
    
    // Дефолт - изображение
    return ModelType::IMAGE;
}

PerformanceTier ModelScanner::estimateTier(const std::string& filePath, ModelType type) {
    // Получаем размер файла
    uintmax_t fileSize = fs::file_size(filePath);
    size_t sizeMB = fileSize / (1024 * 1024);
    
    // Эвристика по размеру файла
    if (type == ModelType::TEXT) {
        if (sizeMB < 2000) return PerformanceTier::WEAK;      // < 2GB
        if (sizeMB < 8000) return PerformanceTier::MEDIUM;    // 2-8GB
        return PerformanceTier::POWERFUL;                      // > 8GB
    }
    else if (type == ModelType::IMAGE) {
        if (sizeMB < 2000) return PerformanceTier::WEAK;      // SD 1.5
        if (sizeMB < 7000) return PerformanceTier::MEDIUM;    // SDXL
        return PerformanceTier::POWERFUL;                      // Большие модели
    }
    else if (type == ModelType::VIDEO) {
        // Видео модели всегда требовательны
        if (sizeMB < 4000) return PerformanceTier::MEDIUM;
        return PerformanceTier::POWERFUL;
    }
    
    return PerformanceTier::MEDIUM;
}

void ModelScanner::extractMetadata(ModelInfo& info, const std::string& filePath) {
    // TODO: Чтение метаданных из safetensors/gguf заголовков
    // Для примера оставляем пустым
    info.description = "Custom model loaded from local directory";
}

void ModelScanner::parseFileName(ModelInfo& info, const std::string& filePath) {
    std::string filename = fs::path(filePath).stem().string();
    
    // Простой парсинг: убираем версии и хеши
    // Пример: "myAwesomeModel_v1.2-pruned-fp16" -> "myAwesomeModel"
    size_t pos = filename.find('_');
    if (pos != std::string::npos) {
        info.name = filename.substr(0, pos);
    } else {
        info.name = filename;
    }
    
    info.author = "Unknown"; // Можно извлечь из пути или метаданных
    info.id = filename;
}

bool ModelScanner::isValidModel(const std::string& filePath) {
    // Проверка что файл существует и имеет минимальный размер
    if (!fs::exists(filePath)) return false;
    
    uintmax_t size = fs::file_size(filePath);
    return size > 1024 * 1024; // Минимум 1MB
}

} // namespace Amuse
