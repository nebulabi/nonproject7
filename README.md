# Amuse Unchained - C++ Edition

🚀 **Свободная платформа для AI-генерации без цензуры**

Быстрая, эффективная и полностью контролируемая версия AmuseAI на C++ с поддержкой текста, изображений и видео.

## 🔥 Особенности

- **Без цензуры**: Полный контроль над генерацией контента
- **Мульти-модельность**: Текст (LLM), Изображения (Diffusion), Видео (AnimateDiff/SVD)
- **Гибридный режим CPU/GPU**: Плавный ползунок распределения нагрузки (0-100%)
- **Встроенная библиотека моделей**: Каталог с фильтрацией по типу, мощности ПК и авторам
- **Ручное добавление моделей**: Поддержка `.gguf`, `.safetensors`, `.onnx`, `.ckpt`
- **Оптимизация под любое железо**: От слабых ПК до мощных workstation

## 📁 Структура проекта

```
AmuseUnchained/
├── src/
│   ├── main.cpp                    # Точка входа
│   ├── core/
│   │   ├── IModelEngine.h          # Интерфейсы движков
│   │   ├── EngineManager.h/cpp     # Менеджер движков
│   │   └── HardwareProfiler.h/cpp  # Профилирование железа
│   ├── library/
│   │   ├── ModelInfo.h             # Структура модели
│   │   ├── ModelScanner.h/cpp      # Сканирование моделей
│   │   └── ModelLibraryManager.h/cpp # Менеджер библиотеки
│   ├── ui/
│   │   └── AppUI.h/cpp             # Интерфейс (Dear ImGui)
│   └── utils/
│       ├── FileSystem.h/cpp        # Работа с файлами
│       └── Config.h                # Конфигурация
├── models/                         # Папка для моделей
│   ├── text/                       # LLM модели (.gguf)
│   ├── image/                      # Diffusion модели (.safetensors)
│   ├── video/                      # Видео модели (.onnx)
│   ├── lora/                       # LoRA адаптеры
│   └── custom/                     # Пользовательские модели
├── output/                         # Сгенерированные файлы
├── CMakeLists.txt                  # Сборка проекта
└── README.md
```

## 🏗️ Сборка

### Требования

- C++17 совместимый компилятор (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.20+
- (Опционально) CUDA Toolkit для GPU ускорения
- (Опционально) Vulkan SDK

### Инструкции

```bash
# Клонирование репозитория
git clone https://github.com/yourusername/AmuseUnchained.git
cd AmuseUnchained

# Создание директории сборки
mkdir build && cd build

# Конфигурация (базовая версия)
cmake ..

# Конфигурация с CUDA
cmake .. -DUSE_CUDA=ON

# Конфигурация с Vulkan
cmake .. -DUSE_VULKAN=ON

# Сборка
cmake --build . --config Release

# Запуск
./AmuseUnchained
```

## 🎮 Использование

### Папка моделей

Все модели хранятся в папке `models/`:

```
models/
├── text/           # Llama, Mistral, etc. (.gguf)
├── image/          # SD1.5, SDXL, Pony (.safetensors)
├── video/          # SVD, AnimateDiff (.onnx)
├── lora/           # LoRA адаптеры
└── custom/         # Автоматически добавленные модели
```

### Настройки оборудования

В приложении доступен ползунок **GPU Load**:
- **0%** → Все вычисления на CPU (для систем без GPU)
- **50%** → Гибридный режим (баланс)
- **100%** → Все вычисления на GPU (максимальная скорость)

Программа автоматически определяет:
- Объем VRAM видеокарты
- Количество ядер CPU
- Доступную оперативную память
- Поддержку CUDA/Vulkan

### Встроенная библиотека

Каталог включает популярные модели с классификацией:

| Тип | Модель | Мощность | Описание |
|-----|--------|----------|----------|
| TEXT | Llama 3.2 1B Q4 | Weak | Быстрая LLM для CPU |
| TEXT | Mistral 7B Q4 | Medium | Универсальная модель |
| IMAGE | Pony Diffusion V6 | Medium | Аниме без цензуры |
| IMAGE | SDXL Turbo | Weak | Генерация за 1-4 шага |
| VIDEO | Stable Video Diffusion | Powerful | Image-to-Video |
| VIDEO | AnimateDiff V3 | Medium | Motion модуль |

## 🔧 Архитектура

### Движки

1. **Text Engine** (на базе `llama.cpp`)
   - GGUF формат
   - Стриминг токенов
   - Контекстное окно

2. **Image Engine** (на базе `stable-diffusion.cpp`)
   - Txt2Img, Img2Img, Inpainting
   - ControlNet поддержка
   - LoRA интеграция

3. **Video Engine** (на базе `ONNX Runtime`)
   - Image-to-Video
   - Frame interpolation
   - Temporal consistency

### API Примеры

```cpp
// Инициализация
Amuse::ModelLibraryManager library;
library.initialize("./models");

// Поиск моделей
auto models = library.filterByType(Amuse::ModelType::IMAGE);
auto weakModels = library.filterByTier(Amuse::PerformanceTier::WEAK);

// Настройка железа
Amuse::HardwareProfiler profiler;
auto config = profiler.recommendConfig(profiler.detectHardware());
config.gpuLoadPercent = 75; // 75% на GPU

// Генерация (после интеграции бэкендов)
auto engine = engineManager.getEngine<Amuse::IImageEngine>();
engine->generateImage("beautiful landscape", "ugly, blurry", 512, 512, 20);
```

## 📝 Планы развития

- [ ] Интеграция `llama.cpp` для текстовых моделей
- [ ] Интеграция `stable-diffusion.cpp` для изображений
- [ ] Интеграция `ONNX Runtime` для видео
- [ ] GUI на Dear ImGui с OpenGL бэкендом
- [ ] Скачивание моделей из каталога
- [ ] Поддержка ControlNet
- [ ] Пакетная генерация
- [ ] Экспорт в различные форматы

## ⚖️ Лицензия

Проект создан в образовательных целях. Используйте ответственно.

## 🤝 Contributing

Pull requests приветствуются! Основные направления:
- Интеграция ML-библиотек
- Оптимизация производительности
- Улучшение UI/UX
- Документация

---

**Amuse Unchained** — Свобода творчества без ограничений 🎨
