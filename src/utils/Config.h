#pragma once

#include <string>
#include <map>
#include <vector>

namespace Amuse {

struct AppConfig {
    // Путь к папке моделей
    std::string modelsPath;
    
    // Путь к папке выходов
    std::string outputPath;
    
    // Настройки оборудования
    int gpuLoadPercent;
    int cpuThreads;
    bool useCUDA;
    bool useVulkan;
    
    // Язык интерфейса
    std::string language;
    
    // Тема UI
    std::string uiTheme;
    
    // Автосохранение настроек
    bool autoSaveSettings;
    
    // Логирование
    bool enableLogging;
    std::string logLevel; // DEBUG, INFO, WARNING, ERROR
    
    AppConfig() 
        : modelsPath("./models"),
          outputPath("./output"),
          gpuLoadPercent(50),
          cpuThreads(4),
          useCUDA(false),
          useVulkan(false),
          language("en"),
          uiTheme("dark"),
          autoSaveSettings(true),
          enableLogging(true),
          logLevel("INFO") {}
};

class Config {
public:
    // Получить синглтон конфигурации
    static Config& getInstance();
    
    // Загрузка конфигурации из файла
    bool load(const std::string& configPath = "config.json");
    
    // Сохранение конфигурации в файл
    bool save(const std::string& configPath = "config.json");
    
    // Получить текущую конфигурацию
    const AppConfig& getConfig() const { return m_config; }
    
    // Обновить отдельное поле
    void setGPULoadPercent(int percent);
    void setCPUThreads(int threads);
    void setUseCUDA(bool enabled);
    void setUseVulkan(bool enabled);
    void setModelsPath(const std::string& path);
    
private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    AppConfig m_config;
};

} // namespace Amuse
