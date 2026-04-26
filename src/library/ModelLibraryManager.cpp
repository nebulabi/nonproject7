#include "ModelLibraryManager.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

namespace Amuse {

ModelLibraryManager::ModelLibraryManager() {}

void ModelLibraryManager::initialize(const std::string& modelsPath) {
    m_modelsPath = modelsPath;
    
    // Создаем структуру папок если не существует
    if (!fs::exists(m_modelsPath)) {
        fs::create_directories(m_modelsPath);
        fs::create_directories(m_modelsPath + "/text");
        fs::create_directories(m_modelsPath + "/image");
        fs::create_directories(m_modelsPath + "/video");
        fs::create_directories(m_modelsPath + "/lora");
        fs::create_directories(m_modelsPath + "/custom");
        
        std::cout << "[ModelLibrary] Created models directory structure at: " 
                  << m_modelsPath << std::endl;
    }
    
    // Загружаем встроенный каталог
    loadBuiltInCatalog();
    
    // Сканируем локальные модели
    scanLocalModels();
}

void ModelLibraryManager::loadBuiltInCatalog() {
    std::cout << "[ModelLibrary] Loading built-in catalog..." << std::endl;
    
    // Пример каталога моделей (в реальности загружается с сервера)
    // Текстовые модели (LLM)
    m_catalogModels.push_back({
        "llama-3.2-1b-q4", "Llama 3.2 1B Q4", "Meta AI",
        "Lightweight uncensored LLM for text generation",
        ModelType::TEXT, PerformanceTier::WEAK,
        "https://huggingface.co/...", "", 1200, 4.8f, 15000,
        {"uncensored", "roleplay", "fast", "cpu-friendly"}
    });
    
    m_catalogModels.push_back({
        "mistral-7b-v0.3-q4", "Mistral 7B v0.3 Q4", "Mistral AI",
        "Powerful 7B model with excellent reasoning",
        ModelType::TEXT, PerformanceTier::MEDIUM,
        "https://huggingface.co/...", "", 4500, 4.9f, 25000,
        {"uncensored", "creative", "coding"}
    });
    
    // Изображения
    m_catalogModels.push_back({
        "sd1.5-pony-xl-v6", "Pony Diffusion V6 XL", "PonyAI",
        "High quality anime-style image generation, no censorship",
        ModelType::IMAGE, PerformanceTier::MEDIUM,
        "https://huggingface.co/...", "", 6800, 4.7f, 30000,
        {"anime", "uncensored", "nsfw-allowed", "artistic"}
    });
    
    m_catalogModels.push_back({
        "sdxl-turbo-q4", "SDXL Turbo Quantized", "Stability AI",
        "Fast image generation in 1-4 steps",
        ModelType::IMAGE, PerformanceTier::WEAK,
        "https://huggingface.co/...", "", 3200, 4.5f, 20000,
        {"fast", "realistic", "low-vram"}
    });
    
    // Видео
    m_catalogModels.push_back({
        "svd-xt-1.1", "Stable Video Diffusion XT 1.1", "Stability AI",
        "Image-to-video generation model",
        ModelType::VIDEO, PerformanceTier::POWERFUL,
        "https://huggingface.co/...", "", 12000, 4.3f, 8000,
        {"video", "animation", "high-quality"}
    });
    
    m_catalogModels.push_back({
        "animatediff-v3", "AnimateDiff V3", "Guoyww",
        "Temporal motion module for stable diffusion",
        ModelType::VIDEO, PerformanceTier::MEDIUM,
        "https://huggingface.co/...", "", 5500, 4.6f, 12000,
        {"animation", "motion", "temporal"}
    });
    
    std::cout << "[ModelLibrary] Loaded " << m_catalogModels.size() 
              << " catalog models" << std::endl;
}

void ModelLibraryManager::scanLocalModels() {
    std::cout << "[ModelLibrary] Scanning local models in: " << m_modelsPath << std::endl;
    m_localModels = m_scanner.scanDirectory(m_modelsPath);
    std::cout << "[ModelLibrary] Found " << m_localModels.size() 
              << " local models" << std::endl;
}

std::vector<ModelInfo> ModelLibraryManager::getAllModels() const {
    return m_localModels;
}

std::vector<ModelInfo> ModelLibraryManager::getLocalModels() const {
    return m_localModels;
}

std::vector<CatalogModel> ModelLibraryManager::getCatalogModels() const {
    return m_catalogModels;
}

std::vector<ModelInfo> ModelLibraryManager::filterByType(ModelType type) const {
    std::vector<ModelInfo> result;
    for (const auto& model : m_localModels) {
        if (model.type == type) {
            result.push_back(model);
        }
    }
    return result;
}

std::vector<ModelInfo> ModelLibraryManager::filterByTier(PerformanceTier tier) const {
    std::vector<ModelInfo> result;
    for (const auto& model : m_localModels) {
        if (model.tier == tier) {
            result.push_back(model);
        }
    }
    return result;
}

std::vector<ModelInfo> ModelLibraryManager::filterByTags(
    const std::vector<std::string>& tags) const {
    
    std::vector<ModelInfo> result;
    for (const auto& model : m_localModels) {
        bool hasAllTags = true;
        for (const auto& tag : tags) {
            auto it = std::find(model.tags.begin(), model.tags.end(), tag);
            if (it == model.tags.end()) {
                hasAllTags = false;
                break;
            }
        }
        if (hasAllTags) {
            result.push_back(model);
        }
    }
    return result;
}

std::vector<ModelInfo> ModelLibraryManager::searchByName(const std::string& query) const {
    std::vector<ModelInfo> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& model : m_localModels) {
        std::string lowerName = model.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) != std::string::npos) {
            result.push_back(model);
        }
    }
    return result;
}

bool ModelLibraryManager::addCustomModel(const std::string& filePath) {
    if (!fs::exists(filePath)) {
        std::cerr << "[ModelLibrary] File not found: " << filePath << std::endl;
        return false;
    }
    
    ModelInfo info = m_scanner.analyzeModelFile(filePath);
    
    // Копируем файл в папку custom/
    std::string filename = fs::path(filePath).filename().string();
    std::string destPath = m_modelsPath + "/custom/" + filename;
    
    try {
        fs::copy_file(filePath, destPath, fs::copy_options::overwrite_existing);
        m_localModels.push_back(info);
        std::cout << "[ModelLibrary] Added custom model: " << info.name << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ModelLibrary] Error copying file: " << e.what() << std::endl;
        return false;
    }
}

bool ModelLibraryManager::removeModel(const std::string& modelId) {
    auto it = std::find_if(m_localModels.begin(), m_localModels.end(),
        [&modelId](const ModelInfo& m) { return m.id == modelId; });
    
    if (it == m_localModels.end()) {
        return false;
    }
    
    try {
        fs::remove(it->filePath);
        m_localModels.erase(it);
        std::cout << "[ModelLibrary] Removed model: " << modelId << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ModelLibrary] Error removing model: " << e.what() << std::endl;
        return false;
    }
}

bool ModelLibraryManager::downloadModel(const std::string& modelId,
    std::function<void(float progress)> callback) {
    
    // Поиск модели в каталоге
    auto it = std::find_if(m_catalogModels.begin(), m_catalogModels.end(),
        [&modelId](const CatalogModel& m) { return m.id == modelId; });
    
    if (it == m_catalogModels.end()) {
        std::cerr << "[ModelLibrary] Model not found in catalog: " << modelId << std::endl;
        return false;
    }
    
    std::cout << "[ModelLibrary] Downloading: " << it->name << std::endl;
    
    // TODO: Реализация скачивания через HTTP
    // Для примера просто эмулируем прогресс
    if (callback) {
        for (int i = 0; i <= 100; i += 10) {
            callback(i / 100.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    std::cout << "[ModelLibrary] Download complete!" << std::endl;
    return true;
}

void ModelLibraryManager::refreshCatalog() {
    std::cout << "[ModelLibrary] Refreshing catalog from server..." << std::endl;
    m_catalogModels.clear();
    loadBuiltInCatalog();
}

} // namespace Amuse
