#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

// SDL2
#include <SDL.h>
#include <SDL_opengl.h>

// Dear ImGui
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

// Наши компоненты
#include "core/types.hpp"
#include "core/TextEngine.hpp"
#include "core/ImageEngine.hpp"
#include "library/ModelLibraryManager.hpp"

using namespace paint;

int main(int argc, char* argv[]) {
    std::cout << "=== pAInt local - Personal AI Neural Toolkit ===" << std::endl;
    
    // Инициализация оборудования
    auto hwConfig = HardwareConfig::detect();
    std::cout << "[System] Detected RAM: " << (hwConfig.maxRAM / 1024 / 1024 / 1024) << " GB" << std::endl;
    std::cout << "[System] CPU Threads: " << hwConfig.cpuThreads << std::endl;
    std::cout << "[System] Performance Tier: ";
    switch(hwConfig.tier) {
        case PerformanceTier::WEAK: std::cout << "Weak (<6GB VRAM)"; break;
        case PerformanceTier::MEDIUM: std::cout << "Medium (8-12GB VRAM)"; break;
        case PerformanceTier::POWERFUL: std::cout << "Powerful (16GB+ VRAM)"; break;
    }
    std::cout << std::endl;
    
    // Инициализация библиотеки моделей
    ModelLibraryManager library;
    std::filesystem::path modelsPath = std::filesystem::current_path() / "models";
    library.initialize(modelsPath);
    
    // Движки
    TextEngine textEngine;
    ImageEngine imageEngine;
    
    // Текущее состояние UI
    int currentTab = 0;  // 0=Text, 1=Image, 2=Video
    std::string prompt;
    std::string negativePrompt;
    std::string selectedModelId;
    float gpuSplit = hwConfig.gpuSplit;
    int imageWidth = 512;
    int imageHeight = 512;
    int steps = 20;
    float cfgScale = 7.0f;
    
    // Инициализация SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "[Error] SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Настройка OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    SDL_Window* window = SDL_CreateWindow("pAInt local",
                                           SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           1280, 720,
                                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    
    if (!window) {
        std::cerr << "[Error] SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);  // VSync
    
    // Инициализация Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    bool running = true;
    SDL_Event event;
    
    std::cout << "[UI] Application started" << std::endl;
    
    // Главный цикл
    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                running = false;
            }
        }
        
        // Новый кадр ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        
        // Главное меню
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Add Model", "Ctrl+O")) {
                    // TODO: Открыть диалог выбора файла
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    running = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Settings")) {
                ImGui::Checkbox("Use GPU", &hwConfig.useGPU);
                ImGui::SliderFloat("GPU/CPU Split", &gpuSplit, 0.0f, 1.0f, "%.2f");
                ImGui::SliderInt("CPU Threads", &hwConfig.cpuThreads, 1, 32);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        
        // Боковая панель с выбором модели
        ImGui::Begin("Models Library", nullptr, ImGuiWindowFlags_NoCollapse);
        
        static int filterType = 0;
        ImGui::RadioButton("All", &filterType, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Text", &filterType, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Image", &filterType, 2);
        ImGui::SameLine();
        ImGui::RadioButton("Video", &filterType, 3);
        
        ImGui::Separator();
        
        auto allModels = library.getAllModels();
        for (const auto& model : allModels) {
            // Фильтрация по типу
            if (filterType != 0) {
                if (filterType == 1 && model.type != ModelType::TEXT) continue;
                if (filterType == 2 && model.type != ModelType::IMAGE) continue;
                if (filterType == 3 && model.type != ModelType::VIDEO) continue;
            }
            
            // Отображение модели
            std::string label = model.name + " (" + model.author + ")";
            if (ImGui::Selectable(label.c_str(), selectedModelId == model.id)) {
                selectedModelId = model.id;
                
                // Загрузка модели
                if (model.type == ModelType::TEXT) {
                    if (textEngine.isLoaded()) {
                        textEngine.unloadModel();
                    }
                    textEngine.loadModel(model, hwConfig);
                } else if (model.type == ModelType::IMAGE) {
                    if (imageEngine.isLoaded()) {
                        imageEngine.unloadModel();
                    }
                    imageEngine.loadModel(model, hwConfig);
                }
            }
            
            // Информация о модели
            ImGui::SameLine();
            std::string tierBadge;
            switch(model.tier) {
                case PerformanceTier::WEAK: tierBadge = "[W]"; break;
                case PerformanceTier::MEDIUM: tierBadge = "[M]"; break;
                case PerformanceTier::POWERFUL: tierBadge = "[P]"; break;
            }
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", tierBadge.c_str());
        }
        
        ImGui::Separator();
        if (ImGui::Button("Add Local Model")) {
            // TODO: Диалог добавления модели
        }
        
        ImGui::End();
        
        // Основная рабочая область
        ImGui::Begin("Workspace");
        
        // Вкладки
        ImGui::BeginTabBar("MainTabs");
        
        // Текстовая вкладка
        if (ImGui::BeginTabItem("Text Generation")) {
            currentTab = 0;
            ImGui::Text("Model: %s", textEngine.getCurrentModel() ? textEngine.getCurrentModel()->name.c_str() : "None loaded");
            ImGui::Separator();
            
            ImGui::InputTextMultiline("Prompt", &prompt[0], 1024, ImVec2(-FLT_MIN, 100));
            
            if (ImGui::Button("Generate")) {
                if (textEngine.isLoaded()) {
                    GenerationParams params;
                    params.prompt = prompt;
                    params.maxTokens = 512;
                    
                    std::string fullResponse;
                    auto result = textEngine.generateStream(params, [&fullResponse](const std::string& token) {
                        fullResponse += token;
                        std::cout << token << std::flush;
                    });
                    
                    std::cout << "\n[TextEngine] Generated " << result.tokensGenerated << " tokens in " << result.processingTime << "s" << std::endl;
                } else {
                    std::cout << "[Warning] No text model loaded" << std::endl;
                }
            }
            
            ImGui::EndTabItem();
        }
        
        // Вкладка изображений
        if (ImGui::BeginTabItem("Image Generation")) {
            currentTab = 1;
            ImGui::Text("Model: %s", imageEngine.getCurrentModel() ? imageEngine.getCurrentModel()->name.c_str() : "None loaded");
            ImGui::Separator();
            
            ImGui::InputTextMultiline("Prompt", &prompt[0], 1024, ImVec2(-FLT_MIN, 60));
            ImGui::InputTextMultiline("Negative Prompt", &negativePrompt[0], 1024, ImVec2(-FLT_MIN, 60));
            
            ImGui::SliderInt("Width", &imageWidth, 256, 1024);
            ImGui::SliderInt("Height", &imageHeight, 256, 1024);
            ImGui::SliderInt("Steps", &steps, 1, 50);
            ImGui::SliderFloat("CFG Scale", &cfgScale, 1.0f, 20.0f);
            
            if (ImGui::Button("Generate Image")) {
                if (imageEngine.isLoaded()) {
                    GenerationParams params;
                    params.prompt = prompt;
                    params.negativePrompt = negativePrompt;
                    params.width = imageWidth;
                    params.height = imageHeight;
                    params.steps = steps;
                    params.cfgScale = cfgScale;
                    
                    auto result = imageEngine.textToImage(params);
                    
                    if (result.success) {
                        std::string outputPath = "output_" + std::to_string(time(nullptr)) + ".bmp";
                        if (result.saveToFile(outputPath)) {
                            std::cout << "[ImageEngine] Saved to " << outputPath << " (" << result.processingTime << "s)" << std::endl;
                        }
                    } else {
                        std::cerr << "[ImageEngine] Error: " << result.error << std::endl;
                    }
                } else {
                    std::cout << "[Warning] No image model loaded" << std::endl;
                }
            }
            
            ImGui::EndTabItem();
        }
        
        // Видео вкладка (заглушка)
        if (ImGui::BeginTabItem("Video Generation")) {
            currentTab = 2;
            ImGui::Text("Video generation coming soon...");
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
        
        ImGui::End();
        
        // Рендеринг
        ImGui::Render();
        int display_w, display_h;
        SDL_GL_GetDrawableSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        SDL_GL_SwapWindow(window);
    }
    
    // Очистка
    textEngine.unloadModel();
    imageEngine.unloadModel();
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    std::cout << "[UI] Application closed" << std::endl;
    return 0;
}
