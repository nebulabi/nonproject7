"""
pAInt Local - Personal AI Neural Toolkit (Local Edition)
Без цензуры. Полный контроль. Поддержка Text, Image, Video.
"""

import os
import sys
import json
import threading
import tkinter as tk
from tkinter import filedialog, messagebox
from pathlib import Path
from typing import Dict, List, Optional, Any

# Проверка зависимостей
try:
    import customtkinter as ctk
    from PIL import Image, ImageTk
except ImportError:
    print("Ошибка: Отсутствуют необходимые библиотеки.")
    print("Установите их командой: pip install customtkinter pillow torch diffusers transformers accelerate")
    sys.exit(1)

# Настройка темы
ctk.set_appearance_mode("Dark")
ctk.set_default_color_theme("blue")

class ModelConfig:
    """Конфигурация модели"""
    def __init__(self, name: str, path: str, model_type: str, performance_tier: str, author: str, files: List[str]):
        self.name = name
        self.path = path
        self.model_type = model_type  # text, image, video
        self.performance_tier = performance_tier  # weak, medium, powerful
        self.author = author
        self.files = files  # Список файлов для мульти-файловых моделей
        self.is_loaded = False

    def to_dict(self):
        return {
            "name": self.name,
            "path": self.path,
            "type": self.model_type,
            "tier": self.performance_tier,
            "author": self.author,
            "files": self.files
        }

class ModelLibraryManager:
    """Менеджер библиотеки моделей"""
    def __init__(self, base_models_dir: str):
        self.base_dir = Path(base_models_dir)
        self.models: Dict[str, ModelConfig] = {}
        self.builtin_catalog = [
            # Пример встроенного каталога (можно расширять)
            {"name": "Stable Diffusion XL Base", "type": "image", "tier": "powerful", "author": "StabilityAI", "files": ["sd_xl_base.safetensors"]},
            {"name": "Llama 3 8B Uncensored", "type": "text", "tier": "medium", "author": "Meta/Community", "files": ["llama-3-8b.gguf"]},
            {"name": "Pony Diffusion V6", "type": "image", "tier": "medium", "author": "Pony", "files": ["ponyV6.safetensors"]},
        ]
        self.scan_models()

    def scan_models(self):
        """Сканирование папок и добавление моделей"""
        self.models.clear()
        
        # Сканирование локальных папок
        for category in ["text", "image", "video"]:
            cat_path = self.base_dir / category
            if not cat_path.exists():
                cat_path.mkdir(parents=True, exist_ok=True)
                continue
            
            # Поиск одиночных файлов и папок моделей
            for item in cat_path.iterdir():
                if item.is_file() and item.suffix in ['.safetensors', '.gguf', '.onnx', '.ckpt', '.pt']:
                    self._add_model_from_file(item, category)
                elif item.is_dir():
                    # Проверка на мульти-файловую модель (наличие config.json или весов)
                    config_file = item / "config.json"
                    model_files = list(item.glob("*.safetensors")) + list(item.glob("*.bin")) + list(item.glob("*.gguf"))
                    if config_file.exists() or model_files:
                        self._add_multifile_model(item, category, model_files)

        # Добавление встроенных (заглушки, если файлов нет)
        for entry in self.builtin_catalog:
            key = f"{entry['type']}_{entry['name']}"
            if key not in self.models:
                # Создаем запись, но помечаем как отсутствующую локально
                pass 

    def _add_model_from_file(self, file_path: Path, category: str):
        name = file_path.stem
        # Простая эвристика для определения автора и мощности (можно улучшить)
        tier = "medium"
        if "xl" in name.lower() or "large" in name.lower(): tier = "powerful"
        if "tiny" in name.lower() or "small" in name.lower(): tier = "weak"
        
        author = "Unknown"
        if "stability" in name.lower(): author = "StabilityAI"
        elif "llama" in name.lower(): author = "Meta"
        
        self.models[f"{category}_{name}"] = ModelConfig(
            name=name,
            path=str(file_path),
            model_type=category,
            performance_tier=tier,
            author=author,
            files=[str(file_path)]
        )

    def _add_multifile_model(self, folder_path: Path, category: str, files: List[Path]):
        name = folder_path.name
        tier = "medium" # Можно парсить config.json для точного определения
        author = "Unknown"
        
        self.models[f"{category}_{name}"] = ModelConfig(
            name=name,
            path=str(folder_path),
            model_type=category,
            performance_tier=tier,
            author=author,
            files=[str(f) for f in files]
        )

    def add_manual_model(self, file_path: str, model_type: str):
        path = Path(file_path)
        if not path.exists():
            return False
        
        category = model_type.lower()
        if category not in ["text", "image", "video"]:
            return False
            
        self._add_model_from_file(path, category)
        return True

    def get_models_by_type(self, m_type: str) -> List[ModelConfig]:
        return [m for m in self.models.values() if m.model_type == m_type]

class AIEngine:
    """Базовый класс движка (заглушка для реальной логики загрузки)"""
    def __init__(self):
        self.current_model: Optional[ModelConfig] = None
        self.device_map = "auto" # auto, cpu, cuda
        self.gpu_ratio = 0.5 # 0.0 - CPU, 1.0 - GPU

    def load_model(self, model_config: ModelConfig, gpu_ratio: float):
        """Загрузка модели с учетом гибридного режима"""
        self.gpu_ratio = gpu_ratio
        self.current_model = model_config
        print(f"Загрузка модели: {model_config.name}")
        print(f"Режим: {'GPU' if gpu_ratio > 0.8 else 'CPU' if gpu_ratio < 0.2 else 'Гибридный'} ({gpu_ratio*100:.0f}% GPU)")
        
        # Здесь будет реальная логика:
        # if model_config.model_type == 'image': load_diffusers(...)
        # if model_config.model_type == 'text': load_llama_cpp(...)
        
        return True

    def generate_text(self, prompt: str, max_tokens: int) -> str:
        if not self.current_model: return "Модель не загружена"
        # Имитация генерации
        return f"[Текстовый ответ от {self.current_model.name} на запрос: '{prompt}']..."

    def generate_image(self, prompt: str, negative_prompt: str, steps: int) -> Any:
        if not self.current_model: return None
        # Имитация генерации
        print(f"Генерация изображения: {prompt}, Steps: {steps}")
        return None # Вернет PIL.Image

class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("pAInt Local - Unchained AI")
        self.geometry("1200x800")
        
        self.library_manager = ModelLibraryManager("./models")
        self.engine = AIEngine()
        
        # Переменные интерфейса
        self.current_tab = "image"
        self.selected_model_var = ctk.StringVar(value="Выберите модель")
        self.gpu_ratio_var = ctk.DoubleVar(value=0.8)
        
        self._setup_ui()

    def _setup_ui(self):
        # Сетка
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        # --- Левая панель (Настройки и Модели) ---
        self.sidebar = ctk.CTkFrame(self, width=250, corner_radius=0)
        self.sidebar.grid(row=0, column=0, sticky="nsew")
        
        self.logo_label = ctk.CTkLabel(self.sidebar, text="pAInt Local", font=ctk.CTkFont(size=20, weight="bold"))
        self.logo_label.grid(row=0, column=0, padx=20, pady=(20, 10))
        
        # Выбор типа задачи
        self.type_segmented = ctk.CTkSegmentedButton(self.sidebar, values=["Image", "Text", "Video"], command=self.switch_tab)
        self.type_segmented.grid(row=1, column=0, padx=20, pady=10)
        self.type_segmented.set("Image")
        
        # Выбор модели
        self.model_label = ctk.CTkLabel(self.sidebar, text="Модель:", anchor="w")
        self.model_label.grid(row=2, column=0, padx=20, pady=(20, 5), sticky="w")
        
        self.model_menu = ctk.CTkOptionMenu(self.sidebar, variable=self.selected_model_var, command=self.on_model_select)
        self.model_menu.grid(row=3, column=0, padx=20, pady=5, sticky="ew")
        self.refresh_model_list()
        
        # Кнопка добавления модели
        self.add_model_btn = ctk.CTkButton(self.sidebar, text="+ Добавить модель", command=self.add_model_dialog)
        self.add_model_btn.grid(row=4, column=0, padx=20, pady=5, sticky="ew")

        # Настройки железа
        self.hw_frame = ctk.CTkFrame(self.sidebar)
        self.hw_frame.grid(row=5, column=0, padx=20, pady=20, sticky="ew")
        
        ctk.CTkLabel(self.hw_frame, text="Нагрузка GPU/CPU").pack(pady=(10, 5))
        
        self.gpu_slider = ctk.CTkSlider(self.hw_frame, from_=0, to=1, number_of_steps=100, variable=self.gpu_ratio_var, command=self.update_gpu_label)
        self.gpu_slider.pack(padx=10, pady=5, fill="x")
        
        self.gpu_label = ctk.CTkLabel(self.hw_frame, text="GPU: 80%")
        self.gpu_label.pack(pady=(0, 10))
        
        ctk.CTkLabel(self.hw_frame, text="← CPU | GPU →", font=ctk.CTkFont(size=10)).pack()

        # --- Центральная область ---
        self.main_area = ctk.CTkFrame(self, corner_radius=0, fg_color="transparent")
        self.main_area.grid(row=0, column=1, sticky="nsew", padx=10, pady=10)
        
        self.tabs_frame = ctk.CTkFrame(self.main_area)
        self.tabs_frame.pack(fill="both", expand=True)
        
        # Контейнеры для вкладок (скрытые по умолчанию)
        self.image_view = self._create_image_view()
        self.text_view = self._create_text_view()
        self.video_view = self._create_video_view()
        
        self.show_tab("image")

    def _create_image_view(self):
        frame = ctk.CTkFrame(self.tabs_frame)
        
        ctk.CTkLabel(frame, text="Prompt", anchor="w").pack(padx=10, pady=(10, 5), fill="x")
        self.img_prompt = ctk.CTkTextbox(frame, height=100)
        self.img_prompt.pack(padx=10, pady=5, fill="x")
        
        ctk.CTkLabel(frame, text="Negative Prompt", anchor="w").pack(padx=10, pady=(5, 5), fill="x")
        self.img_neg_prompt = ctk.CTkTextbox(frame, height=50)
        self.img_neg_prompt.pack(padx=10, pady=5, fill="x")
        
        # Параметры
        params_frame = ctk.CTkFrame(frame)
        params_frame.pack(padx=10, pady=10, fill="x")
        
        ctk.CTkLabel(params_frame, text="Steps:").pack(side="left", padx=5)
        self.steps_entry = ctk.CTkEntry(params_frame, width=50)
        self.steps_entry.insert(0, "25")
        self.steps_entry.pack(side="left", padx=5)
        
        ctk.CTkLabel(params_frame, text="Seed:").pack(side="left", padx=5)
        self.seed_entry = ctk.CTkEntry(params_frame, width=50)
        self.seed_entry.insert(0, "-1")
        self.seed_entry.pack(side="left", padx=5)
        
        # Кнопка генерации
        self.gen_img_btn = ctk.CTkButton(frame, text="Генерировать изображение", command=self.run_image_generation, height=40)
        self.gen_img_btn.pack(padx=10, pady=20, fill="x")
        
        # Превью
        self.preview_label = ctk.CTkLabel(frame, text="Предварительный просмотр", fg_color="gray20")
        self.preview_label.pack(padx=10, pady=10, fill="both", expand=True)
        
        return frame

    def _create_text_view(self):
        frame = ctk.CTkFrame(self.tabs_frame)
        
        ctk.CTkLabel(frame, text="System Prompt (Optional)", anchor="w").pack(padx=10, pady=(10, 5), fill="x")
        self.sys_prompt = ctk.CTkEntry(frame)
        self.sys_prompt.pack(padx=10, pady=5, fill="x")
        
        ctk.CTkLabel(frame, text="User Input", anchor="w").pack(padx=10, pady=(5, 5), fill="x")
        self.text_input = ctk.CTkTextbox(frame, height=150)
        self.text_input.pack(padx=10, pady=5, fill="both", expand=True)
        
        self.gen_text_btn = ctk.CTkButton(frame, text="Отправить", command=self.run_text_generation)
        self.gen_text_btn.pack(padx=10, pady=10, fill="x")
        
        self.text_output = ctk.CTkTextbox(frame, state="disabled")
        self.text_output.pack(padx=10, pady=10, fill="both", expand=True)
        
        return frame

    def _create_video_view(self):
        frame = ctk.CTkFrame(self.tabs_frame)
        ctk.CTkLabel(frame, text="Видео генерация (Экспериментально)", font=ctk.CTkFont(weight="bold")).pack(pady=20)
        ctk.CTkLabel(frame, text="Требуется мощная видеокарта (16GB+ VRAM)").pack()
        # Заглушка
        return frame

    def switch_tab(self, value):
        val = value.lower()
        self.current_tab = val
        self.show_tab(val)
        self.refresh_model_list() # Обновить список под тип

    def show_tab(self, tab_name):
        self.image_view.pack_forget()
        self.text_view.pack_forget()
        self.video_view.pack_forget()
        
        if tab_name == "image":
            self.image_view.pack(fill="both", expand=True)
        elif tab_name == "text":
            self.text_view.pack(fill="both", expand=True)
        elif tab_name == "video":
            self.video_view.pack(fill="both", expand=True)

    def refresh_model_list(self):
        models = self.library_manager.get_models_by_type(self.current_tab)
        values = [m.name for m in models]
        if not values:
            values = ["Нет моделей в папке models/" + self.current_tab]
        
        current_val = self.selected_model_var.get()
        self.model_menu.configure(values=values)
        if current_val in values:
            self.selected_model_var.set(current_val)
        elif values and values[0] != "Нет моделей...":
            self.selected_model_var.set(values[0])

    def update_gpu_label(self, value):
        pct = int(value * 100)
        self.gpu_label.configure(text=f"GPU: {pct}%")

    def on_model_select(self, selection):
        if "Нет моделей" in selection:
            return
        # Найти конфиг
        models = self.library_manager.get_models_by_type(self.current_tab)
        config = next((m for m in models if m.name == selection), None)
        if config:
            ratio = self.gpu_ratio_var.get()
            # В реальном приложении здесь была бы асинхронная загрузка
            threading.Thread(target=self._load_model_thread, args=(config, ratio)).start()

    def _load_model_thread(self, config, ratio):
        self.engine.load_model(config, ratio)
        # Уведомление можно добавить через after

    def add_model_dialog(self):
        file_path = filedialog.askopenfilename(
            title="Выберите файл модели",
            filetypes=[
                ("Model Files", "*.safetensors *.gguf *.ckpt *.bin"),
                ("All Files", "*.*")
            ]
        )
        if file_path:
            if self.library_manager.add_manual_model(file_path, self.current_tab):
                messagebox.showinfo("Успех", "Модель добавлена!")
                self.refresh_model_list()
            else:
                messagebox.showerror("Ошибка", "Не удалось добавить модель.")

    def run_image_generation(self):
        prompt = self.img_prompt.get("1.0", "end-1c")
        neg = self.img_neg_prompt.get("1.0", "end-1c")
        try:
            steps = int(self.steps_entry.get())
        except:
            steps = 25
            
        if not self.engine.current_model:
            messagebox.showwarning("Внимание", "Сначала выберите и загрузите модель!")
            return

        # Запуск в потоке
        def task():
            res = self.engine.generate_image(prompt, neg, steps)
            # Обработка результата (показать картинку)
            self.after(0, lambda: self.preview_label.configure(text="Генерация завершена (эмуляция)"))
            
        threading.Thread(target=task).start()

    def run_text_generation(self):
        prompt = self.text_input.get("1.0", "end-1c")
        if not self.engine.current_model:
            messagebox.showwarning("Внимание", "Сначала выберите модель!")
            return
            
        def task():
            res = self.engine.generate_text(prompt, 256)
            self.after(0, lambda: self._append_text(res))
            
        threading.Thread(target=task).start()

    def _append_text(self, text):
        self.text_output.configure(state="normal")
        self.text_output.insert("end", text + "\n\n")
        self.text_output.configure(state="disabled")

if __name__ == "__main__":
    app = App()
    app.mainloop()
