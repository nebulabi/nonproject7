#include "TextEngine.hpp"
#include <iostream>
#include <chrono>

// Включаем заголовки llama.cpp
extern "C" {
    #include "llama.h"
}

namespace paint {

TextEngine::TextEngine() {}

TextEngine::~TextEngine() {
    unloadModel();
}

bool TextEngine::loadModel(const ModelDefinition& model, const HardwareConfig& hwConfig) {
    if (loaded_) {
        unloadModel();
    }
    
    currentModel_ = model;
    hwConfig_ = hwConfig;
    
    // Определение пути к главному файлу
    std::string modelPath;
    if (model.isMultiFile()) {
        // Поиск основного файла весов
        for (const auto& file : model.files) {
            if (file.type == "weights") {
                modelPath = file.path;
                break;
            }
        }
    } else {
        modelPath = model.mainFile;
    }
    
    if (modelPath.empty() || !std::filesystem::exists(modelPath)) {
        std::cerr << "[TextEngine] Model file not found: " << modelPath << std::endl;
        return false;
    }
    
    std::cout << "[TextEngine] Loading model: " << modelPath << std::endl;
    
    // Параметры загрузки модели
    llama_model_params params = llama_model_default_params();
    params.n_gpu_layers = static_cast<int>(hwConfig.gpuSplit * 100);  // Примерная конверсия
    params.n_threads = hwConfig.cpuThreads;
    
    // Загрузка модели
    model_ = llama_load_model_from_file(modelPath.c_str(), params);
    
    if (!model_) {
        std::cerr << "[TextEngine] Failed to load model" << std::endl;
        return false;
    }
    
    // Инициализация контекста
    if (!initializeContext()) {
        llama_free_model(model_);
        model_ = nullptr;
        return false;
    }
    
    loaded_ = true;
    std::cout << "[TextEngine] Model loaded successfully" << std::endl;
    return true;
}

bool TextEngine::initializeContext() {
    if (!model_) {
        return false;
    }
    
    llama_context_params ctxParams = llama_context_default_params();
    ctxParams.n_ctx = 4096;  // Размер контекста
    ctxParams.n_batch = 512;
    ctxParams.n_threads = hwConfig_.cpuThreads;
    
    ctx_ = llama_new_context_with_model(model_, ctxParams);
    
    return ctx_ != nullptr;
}

void TextEngine::unloadModel() {
    if (ctx_) {
        llama_free(ctx_);
        ctx_ = nullptr;
    }
    
    if (model_) {
        llama_free_model(model_);
        model_ = nullptr;
    }
    
    loaded_ = false;
    std::cout << "[TextEngine] Model unloaded" << std::endl;
}

bool TextEngine::isLoaded() const {
    return loaded_ && model_ != nullptr && ctx_ != nullptr;
}

TextGenerationResult TextEngine::generate(const GenerationParams& params) {
    TextGenerationResult result;
    
    if (!loaded_) {
        result.success = false;
        result.error = "Model not loaded";
        return result;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Токенизация промпта
    std::string prompt = params.prompt;
    std::vector<llama_token> tokens(prompt.size());
    int tokenCount = llama_tokenize(model_, prompt.c_str(), prompt.size(), tokens.data(), tokens.size(), true, true);
    
    if (tokenCount < 0) {
        result.success = false;
        result.error = "Tokenization failed";
        return result;
    }
    
    tokens.resize(tokenCount);
    
    // Инициализация генерации
    llama_kv_cache_clear(ctx_);
    
    // Оценка префикса
    if (llama_decode(ctx_, llama_batch_get_one(tokens.data(), tokens.size(), 0))) {
        result.success = false;
        result.error = "Failed to evaluate prompt";
        return result;
    }
    
    // Генерация токенов
    int maxTokens = params.maxTokens;
    int generatedCount = 0;
    
    while (generatedCount < maxTokens) {
        // Сэмплирование следующего токена
        llama_token newToken = llama_sampler_sample(nullptr, ctx_, -1);
        
        // Проверка на конец генерации
        if (newToken == llama_token_eos(model_)) {
            break;
        }
        
        // Декодирование токена в текст
        char buffer[256];
        int length = llama_token_to_piece(model_, newToken, buffer, sizeof(buffer), 0, true);
        
        if (length > 0) {
            result.text.append(buffer, length);
        }
        
        generatedCount++;
        
        // Оценка нового токена
        if (llama_decode(ctx_, llama_batch_get_one(&newToken, 1, generatedCount))) {
            break;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = endTime - startTime;
    
    result.tokensGenerated = generatedCount;
    result.processingTime = duration.count();
    
    return result;
}

TextGenerationResult TextEngine::generateStream(const GenerationParams& params, TextStreamCallback callback) {
    TextGenerationResult result;
    
    if (!loaded_) {
        result.success = false;
        result.error = "Model not loaded";
        return result;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Токенизация
    std::string prompt = params.prompt;
    std::vector<llama_token> tokens(prompt.size());
    int tokenCount = llama_tokenize(model_, prompt.c_str(), prompt.size(), tokens.data(), tokens.size(), true, true);
    
    if (tokenCount < 0) {
        result.success = false;
        result.error = "Tokenization failed";
        return result;
    }
    
    tokens.resize(tokenCount);
    
    llama_kv_cache_clear(ctx_);
    
    if (llama_decode(ctx_, llama_batch_get_one(tokens.data(), tokens.size(), 0))) {
        result.success = false;
        result.error = "Failed to evaluate prompt";
        return result;
    }
    
    int maxTokens = params.maxTokens;
    int generatedCount = 0;
    
    while (generatedCount < maxTokens) {
        llama_token newToken = llama_sampler_sample(nullptr, ctx_, -1);
        
        if (newToken == llama_token_eos(model_)) {
            break;
        }
        
        char buffer[256];
        int length = llama_token_to_piece(model_, newToken, buffer, sizeof(buffer), 0, true);
        
        if (length > 0) {
            std::string tokenStr(buffer, length);
            result.text += tokenStr;
            
            // Вызов callback для потокового вывода
            if (callback) {
                callback(tokenStr);
            }
        }
        
        generatedCount++;
        
        if (llama_decode(ctx_, llama_batch_get_one(&newToken, 1, generatedCount))) {
            break;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = endTime - startTime;
    
    result.tokensGenerated = generatedCount;
    result.processingTime = duration.count();
    
    return result;
}

void TextEngine::updateHardwareConfig(const HardwareConfig& hwConfig) {
    hwConfig_ = hwConfig;
    
    // Если модель загружена, может потребоваться перезагрузка для применения новых настроек
    if (loaded_) {
        std::cout << "[TextEngine] Hardware config updated, reload may be required" << std::endl;
    }
}

const ModelDefinition* TextEngine::getCurrentModel() const {
    return loaded_ ? &currentModel_ : nullptr;
}

} // namespace paint
