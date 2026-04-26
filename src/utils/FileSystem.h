#pragma once

#include <string>
#include <filesystem>
#include <vector>

namespace Amuse {

class FileSystem {
public:
    // Получить путь к директории приложения
    static std::string getAppDirectory();
    
    // Получить путь к папке моделей
    static std::string getModelsDirectory();
    
    // Получить путь к папке конфигурации
    static std::string getConfigDirectory();
    
    // Получить путь к папке выходных данных (изображения, видео)
    static std::string getOutputDirectory();
    
    // Создать директорию если не существует
    static bool ensureDirectoryExists(const std::string& path);
    
    // Проверить существование файла
    static bool fileExists(const std::string& path);
    
    // Получить размер файла в байтах
    static size_t getFileSize(const std::string& path);
    
    // Получить расширение файла
    static std::string getFileExtension(const std::string& path);
    
    // Получить имя файла без расширения
    static std::string getFileNameWithoutExtension(const std::string& path);
    
    // Список файлов в директории с фильтром по расширению
    static std::vector<std::string> listFilesWithExtension(
        const std::string& directory, 
        const std::string& extension
    );
};

} // namespace Amuse
