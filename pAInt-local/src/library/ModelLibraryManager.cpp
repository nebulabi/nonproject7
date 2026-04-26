#include "ModelLibraryManager.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

// Простой JSON парсер для config.json
#include <regex>

namespace paint {

ModelLibraryManager::ModelLibraryManager() {}

void ModelLibraryManager::initialize(const std::filesystem::path& baseModelsPath) {
    models_.clear();
    modelIndex_.clear();
    
    if (!std::filesystem::exists(baseModelsPath)) {
        std::filesystem::create_directories(baseModelsPath);
    }
    
    // Сканирование подпапок по типам
    std::vector<std::pair<std::string, ModelType>> typeDirs = {
        {"text", ModelType::TEXT},
        {"image", ModelType::IMAGE},
        {"video", ModelType::VIDEO},
        {"lora", ModelType::IMAGE},  // LoRA для изображений
        {"custom", ModelType::TEXT}   // По умолчанию текст для custom
    };
    
    for (const auto& [dirName, type] : typeDirs) {
        auto dirPath = baseModelsPath / dirName;
        if (std::filesystem::exists(dirPath)) {
            scanDirectory(dirPath, type);
        }
    }
    
    // Добавление встроенного каталога
    auto builtin = getBuiltinCatalog();
    for (auto& model : builtin) {
        if (modelIndex_.find(model.id) == modelIndex_.end()) {
            modelIndex_[model.id] = models_.size();
            models_.push_back(std::move(model));
        }
    }
    
    std::cout << "[Library] Loaded " << models_.size() << " models" << std::endl;
}

void ModelLibraryManager::scanDirectory(const std::filesystem::path& dir, ModelType type) {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_directory()) {
            // Многофайловая модель (папка)
            auto model = analyzeModelStructure(entry.path(), type);
            if (!model.name.empty()) {
                modelIndex_[model.id] = models_.size();
                models_.push_back(model);
            }
        } else if (entry.is_regular_file()) {
            // Однофайловая модель
            auto ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            
            if (ext == ".gguf" || ext == ".safetensors" || ext == ".onnx" || ext == ".bin") {
                auto model = analyzeModelStructure(entry.path(), type);
                if (!model.name.empty()) {
                    modelIndex_[model.id] = models_.size();
                    models_.push_back(model);
                }
            }
        }
    }
}

ModelDefinition ModelLibraryManager::analyzeModelStructure(const std::filesystem::path& path, ModelType type) {
    ModelDefinition model;
    model.basePath = path;
    model.type = type;
    
    // Генерация ID из пути
    model.id = std::filesystem::relative(path).string();
    std::replace(model.id.begin(), model.id.end(), '/', '_');
    std::replace(model.id.begin(), model.id.end(), '\\', '_');
    
    // Имя файла/папки как название
    model.name = path.filename().string();
    
    // Удаление расширения из имени
    if (path.has_extension()) {
        model.name = path.stem().string();
    }
    
    // Попытка прочитать config.json
    std::filesystem::path configPath;
    if (path.is_directory()) {
        configPath = path / "config.json";
    } else {
        configPath = path.parent_path() / "config.json";
    }
    
    if (std::filesystem::exists(configPath)) {
        parseConfigFile(configPath, model);
    }
    
    // Если это папка, собираем все файлы
    if (path.is_directory()) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                ModelFile file;
                file.path = entry.path().string();
                
                // Определение типа файла
                auto filename = entry.path().filename().string();
                if (filename.find("vae") != std::string::npos) {
                    file.type = "vae";
                } else if (filename.find("encoder") != std::string::npos) {
                    file.type = "encoder";
                } else if (filename.find("decoder") != std::string::npos) {
                    file.type = "decoder";
                } else if (filename.find("config") != std::string::npos) {
                    file.type = "config";
                } else if (entry.path().extension() == ".json") {
                    file.type = "config";
                } else {
                    file.type = "weights";
                }
                
                model.files.push_back(file);
            }
        }
    } else {
        // Один файл
        ModelFile file;
        file.path = path.string();
        file.type = "weights";
        model.mainFile = path.string();
    }
    
    // Оценка памяти
    estimateMemoryRequirements(model);
    
    // Авто-категоризация по размеру
    size_t totalSize = model.estimatedVRAM + model.estimatedRAM;
    if (totalSize > 10ULL * 1024 * 1024 * 1024) {
        model.tier = PerformanceTier::POWERFUL;
    } else if (totalSize > 4ULL * 1024 * 1024 * 1024) {
        model.tier = PerformanceTier::MEDIUM;
    } else {
        model.tier = PerformanceTier::WEAK;
    }
    
    return model;
}

ModelType ModelLibraryManager::detectModelType(const std::filesystem::path& path) {
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".gguf") {
        // GGUF может быть и текст, и изображение, проверяем имя
        auto name = path.stem().string();
        if (name.find("sd") != std::string::npos || 
            name.find("stable") != std::string::npos ||
            name.find("diffusion") != std::string::npos) {
            return ModelType::IMAGE;
        }
        return ModelType::TEXT;
    }
    
    if (ext == ".safetensors" || ext == ".ckpt" || ext == ".bin") {
        return ModelType::IMAGE;
    }
    
    if (ext == ".onnx") {
        return ModelType::VIDEO;  // Обычно ONNX для видео
    }
    
    return ModelType::TEXT;  // По умолчанию
}

void ModelLibraryManager::estimateMemoryRequirements(ModelDefinition& model) {
    size_t totalSize = 0;
    
    if (model.isMultiFile()) {
        for (const auto& file : model.files) {
            if (std::filesystem::exists(file.path)) {
                totalSize += std::filesystem::file_size(file.path);
            }
        }
    } else if (!model.mainFile.empty() && std::filesystem::exists(model.mainFile)) {
        totalSize = std::filesystem::file_size(model.mainFile);
    }
    
    // Эвристика: VRAM ~ 70% от размера модели, RAM ~ остальное
    model.estimatedVRAM = static_cast<size_t>(totalSize * 0.7);
    model.estimatedRAM = static_cast<size_t>(totalSize * 0.3);
}

bool ModelLibraryManager::parseConfigFile(const std::filesystem::path& configPath, ModelDefinition& model) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    // Очень простой парсинг JSON (для production лучше использовать nlohmann/json)
    std::regex nameRegex("\"name\"\\s*:\\s*\"([^\"]+)\"");
    std::regex authorRegex("\"author\"\\s*:\\s*\"([^\"]+)\"");
    std::regex descRegex("\"description\"\\s*:\\s*\"([^\"]+)\"");
    
    std::smatch match;
    if (std::regex_search(content, match, nameRegex)) {
        model.name = match[1];
    }
    if (std::regex_search(content, match, authorRegex)) {
        model.author = match[1];
    }
    if (std::regex_search(content, match, descRegex)) {
        model.description = match[1];
    }
    
    return true;
}

const std::vector<ModelDefinition>& ModelLibraryManager::getAllModels() const {
    return models_;
}

std::vector<ModelDefinition> ModelLibraryManager::getModelsByType(ModelType type) const {
    std::vector<ModelDefinition> result;
    for (const auto& model : models_) {
        if (model.type == type) {
            result.push_back(model);
        }
    }
    return result;
}

std::vector<ModelDefinition> ModelLibraryManager::getModelsByTier(PerformanceTier tier) const {
    std::vector<ModelDefinition> result;
    for (const auto& model : models_) {
        if (model.tier == tier) {
            result.push_back(model);
        }
    }
    return result;
}

const ModelDefinition* ModelLibraryManager::getModelById(const std::string& id) const {
    auto it = modelIndex_.find(id);
    if (it != modelIndex_.end()) {
        return &models_[it->second];
    }
    return nullptr;
}

bool ModelLibraryManager::addManualModel(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    auto type = detectModelType(path);
    auto model = analyzeModelStructure(path, type);
    
    if (model.name.empty()) {
        return false;
    }
    
    if (modelIndex_.find(model.id) != modelIndex_.end()) {
        return false;  // Уже существует
    }
    
    modelIndex_[model.id] = models_.size();
    models_.push_back(model);
    return true;
}

bool ModelLibraryManager::removeModel(const std::string& id) {
    auto it = modelIndex_.find(id);
    if (it == modelIndex_.end()) {
        return false;
    }
    
    size_t index = it->second;
    models_.erase(models_.begin() + index);
    modelIndex_.erase(it);
    
    // Пересчёт индексов
    modelIndex_.clear();
    for (size_t i = 0; i < models_.size(); ++i) {
        modelIndex_[models_[i].id] = i;
    }
    
    return true;
}

std::vector<ModelDefinition> ModelLibraryManager::getBuiltinCatalog() {
    // Встроенный каталог популярных моделей (без цензуры)
    std::vector<ModelDefinition> catalog;
    
    // Текстовые модели
    catalog.push_back({
        "llama-3-8b-uncensored",
        "Llama 3 8B Uncensored",
        "LocalLLM Community",
        "Мощная языковая модель без цензуры",
        ModelType::TEXT,
        PerformanceTier::MEDIUM,
        "",  // mainFile
        {},  // files
        {{"uncensored", "true"}, {"roleplay", "true"}},
        8ULL * 1024 * 1024 * 1024,
        4ULL * 1024 * 1024 * 1024,
        {}
    });
    
    catalog.push_back({
        "mistral-7b-instruct",
        "Mistral 7B Instruct",
        "Mistral AI",
        "Эффективная модель для инструкций",
        ModelType::TEXT,
        PerformanceTier::WEAK,
        "",
        {},
        {{"instruct", "true"}, {"fast", "true"}},
        4ULL * 1024 * 1024 * 1024,
        2ULL * 1024 * 1024 * 1024,
        {}
    });
    
    // Модели изображений
    catalog.push_back({
        "sdxl-pony-diffusion",
        "Pony Diffusion V6 XL",
        "Pony",
        "Универсальная модель для аниме и реализма без цензуры",
        ModelType::IMAGE,
        PerformanceTier::POWERFUL,
        "",
        {},
        {{"uncensored", "true"}, {"anime", "true"}, {"realistic", "true"}},
        12ULL * 1024 * 1024 * 1024,
        6ULL * 1024 * 1024 * 1024,
        {}
    });
    
    catalog.push_back({
        "sd15-realistic-vision",
        "Realistic Vision V5.1",
        "SG_161222",
        "Фотореалистичная модель на базе SD 1.5",
        ModelType::IMAGE,
        PerformanceTier::WEAK,
        "",
        {},
        {{"realistic", "true"}, {"photography", "true"}},
        4ULL * 1024 * 1024 * 1024,
        2ULL * 1024 * 1024 * 1024,
        {}
    });
    
    // Видео модели
    catalog.push_back({
        "animatediff-v3",
        "AnimateDiff V3",
        "Guoyww",
        "Генерация видео из изображений",
        ModelType::VIDEO,
        PerformanceTier::POWERFUL,
        "",
        {},
        {{"video", "true"}, {"animation", "true"}},
        16ULL * 1024 * 1024 * 1024,
        8ULL * 1024 * 1024 * 1024,
        {}
    });
    
    return catalog;
}

} // namespace paint
