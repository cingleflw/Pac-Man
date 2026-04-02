#pragma once  // Защита от повторного включения

#include <SDL3/SDL.h>

#include <map>  // Для словаря
#include <string>

class TextureManager {
 private:
  // Конструктор по умолчанию приватный - нельзя создать экземпляр извне
  TextureManager() = default;

  // Запрещаем копирование и перемещение - экземпляр должен быть единственным
  // для Singleton
  TextureManager(const TextureManager&) = delete;
  TextureManager& operator=(const TextureManager&) = delete;
  TextureManager(TextureManager&&) = delete;
  TextureManager& operator=(TextureManager&&) = delete;

  // Словарь текстур: ключ - строковый тег, значение - указатель на текстуру
  std::map<std::string, SDL_Texture*> textureMap_;

 public:
  // Загрузка изображения из файла, сохранение под указанным тегом.
  bool load(std::string fileName, std::string tag, SDL_Renderer* renderer);

  // Отрисовка текстуры целиком в указанных координатах и размерах
  void draw(std::string tag, float x, float y, float width, float height,
            SDL_Renderer* renderer, SDL_FlipMode flip = SDL_FLIP_NONE);

  // Отрисовка отдельного кадра из спрайт-листа
  void drawFrame(std::string tag, float x, float y, float width, float height,
                 int currentRow, int currentFrame, SDL_Renderer* renderer,
                 SDL_FlipMode flip = SDL_FLIP_NONE);

  // Точка доступа к единственному экземпляру (синглтон)
  static TextureManager& Instance() {
    static TextureManager Instance_;  // Создаётся при первом вызове
    return Instance_;
  }
};