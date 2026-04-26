#pragma once

#include "ModelInfo.h"
#include <string>
#include <vector>
#include <filesystem>

namespace Amuse {

class ModelScanner {
public:
    ModelScanner();
    
    // Сканирование папки models/ и подпапок
    std::vector<ModelInfo> scanDirectory(const std::string& basePath);
    
    // Анализ одной модели по пути
    ModelInfo analyzeModelFile(const std::string& filePath);
    
    // Определение типа модели по расширению и содержимому
    ModelType detectModelType(const std::string& filePath);
    
    // Оценка требований к железу (примерная)
    PerformanceTier estimateTier(const std::string& filePath, ModelType type);
    
private:
    // Извлечение метаданных из файла (если есть)
    void extractMetadata(ModelInfo& info, const std::string& filePath);
    
    // Парсинг имени файла для получения названия и автора
    void parseFileName(ModelInfo& info, const std::string& filePath);
    
    // Проверка валидности модели
    bool isValidModel(const std::string& filePath);
};

} // namespace Amuse
