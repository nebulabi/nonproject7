#pragma once

#include <string>
#include <vector>

namespace Amuse {

// Простой класс для эмуляции UI (в реальной версии будет Dear ImGui)
class AppUI {
public:
    AppUI();
    ~AppUI();
    
    // Инициализация UI
    bool initialize(int width = 1280, int height = 720);
    
    // Главный цикл рендеринга
    void render();
    
    // Остановка UI
    void shutdown();
    
    // Проверка должен ли работать цикл
    bool shouldClose() const;
    
private:
    bool m_running;
    int m_width;
    int m_height;
    
    // Рендер отдельных панелей
    void renderMenuBar();
    void renderModelLibraryPanel();
    void renderSettingsPanel();
    void renderGenerationPanel();
    void renderHardwareStatusPanel();
};

} // namespace Amuse
