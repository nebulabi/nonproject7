#pragma once

#include "ModelInfo.h"
#include "ModelScanner.h"
#include <vector>
#include <map>
#include <string>
#include <functional>

namespace Amuse {

// Структура для встроенной библиотеки (онлайн каталог)
struct CatalogModel {
    std::string id;
    std::string name;
    std::string author;
    std::string description;
    ModelType type;
    PerformanceTier tier;
    std::string downloadUrl;
    std::string hash;
    size_t fileSize;
    float rating;
    int downloads;
    std::vector<std::string> tags;
};

class ModelLibraryManager {
public:
    ModelLibraryManager();
    
    // Инициализация менеджера
    void initialize(const std::string& modelsPath);
    
    // Получить все доступные модели (локальные + каталог)
    std::vector<ModelInfo> getAllModels() const;
    
    // Получить только локальные модели
    std::vector<ModelInfo> getLocalModels() const;
    
    // Получить модели из каталога (для скачивания)
    std::vector<CatalogModel> getCatalogModels() const;
    
    // Фильтрация моделей
    std::vector<ModelInfo> filterByType(ModelType type) const;
    std::vector<ModelInfo> filterByTier(PerformanceTier tier) const;
    std::vector<ModelInfo> filterByTags(const std::vector<std::string>& tags) const;
    
    // Поиск по названию
    std::vector<ModelInfo> searchByName(const std::string& query) const;
    
    // Добавление пользовательской модели (ручное)
    bool addCustomModel(const std::string& filePath);
    
    // Удаление модели (только локальные)
    bool removeModel(const std::string& modelId);
    
    // Скачивание модели из каталога
    bool downloadModel(const std::string& modelId, 
                       std::function<void(float progress)> callback = nullptr);
    
    // Обновление каталога (загрузка списка с сервера)
    void refreshCatalog();
    
    // Получить путь к папке моделей
    const std::string& getModelsPath() const { return m_modelsPath; }
    
private:
    std::string m_modelsPath;
    std::vector<ModelInfo> m_localModels;
    std::vector<CatalogModel> m_catalogModels;
    
    ModelScanner m_scanner;
    
    // Загрузка встроенного каталога (хардкод для примера)
    void loadBuiltInCatalog();
    
    // Сканирование локальных моделей
    void scanLocalModels();
};

} // namespace Amuse
