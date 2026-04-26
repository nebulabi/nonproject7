#pragma once

#include "core/types.hpp"
#include <vector>
#include <string>
#include <filesystem>

namespace paint {

class ModelLibraryManager {
public:
    ModelLibraryManager();
    
    // Инициализация библиотеки (сканирование папок)
    void initialize(const std::filesystem::path& baseModelsPath);
    
    // Получить все модели
    const std::vector<ModelDefinition>& getAllModels() const;
    
    // Получить модели по типу
    std::vector<ModelDefinition> getModelsByType(ModelType type) const;
    
    // Получить модели по категории производительности
    std::vector<ModelDefinition> getModelsByTier(PerformanceTier tier) const;
    
    // Поиск модели по ID
    const ModelDefinition* getModelById(const std::string& id) const;
    
    // Добавить модель вручную (из файла или папки)
    bool addManualModel(const std::filesystem::path& path);
    
    // Удалить модель из списка (не удаляет файлы)
    bool removeModel(const std::string& id);
    
    // Получить встроенный каталог популярных моделей
    static std::vector<ModelDefinition> getBuiltinCatalog();
    
private:
    std::vector<ModelDefinition> models_;
    std::map<std::string, size_t> modelIndex_;  // id -> индекс в vector
    
    // Сканирование папки на наличие моделей
    void scanDirectory(const std::filesystem::path& dir, ModelType type);
    
    // Анализ структуры модели
    ModelDefinition analyzeModelStructure(const std::filesystem::path& path, ModelType type);
    
    // Определение типа модели по расширению/содержимому
    ModelType detectModelType(const std::filesystem::path& path);
    
    // Оценка требований к памяти
    void estimateMemoryRequirements(ModelDefinition& model);
    
    // Парсинг config.json если есть
    bool parseConfigFile(const std::filesystem::path& configPath, ModelDefinition& model);
};

} // namespace paint
