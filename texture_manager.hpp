/**
 * @file texture_manager.hpp
 * @brief Singleton для загрузки, хранения и отрисовки текстур.
 */

#pragma once

#include <SDL3/SDL.h>

#include <map>
#include <string>

/**
 * @brief Менеджер текстур.
 *
 * Загружает изображения с диска, хранит их в виде SDL_Texture
 * в словаре по строковому тегу и предоставляет методы
 * для отрисовки - как целиком, так и покадрово из спрайт-листа.
 *
 * @note Singleton: копирование и перемещение запрещены - экземпляр
 * единственный.
 */
class TextureManager {
 private:
  /// @brief Приватный конструктор - экземпляр создаётся только через
  /// Instance().
  TextureManager() = default;

  TextureManager(const TextureManager&) = delete;
  TextureManager& operator=(const TextureManager&) = delete;
  TextureManager(TextureManager&&) = delete;
  TextureManager& operator=(TextureManager&&) = delete;

  /// Словарь текстур: ключ - строковый тег, значение - указатель на текстуру
  std::map<std::string, SDL_Texture*> textureMap_;

 public:
  /**
   * @brief Загружает изображение из файла и сохраняет текстуру под указанным
   * тегом.
   *
   * Создаёт промежуточный SDL_Surface через IMG_Load, конвертирует его
   * в SDL_Texture и помещает в словарь. Surface освобождается сразу после
   * конвертации.
   *
   * @param fileName Путь к файлу изображения.
   * @param tag      Строковый тег, по которому текстура будет доступна позднее.
   * @param renderer Отрисовщик, на базе которого создаётся текстура.
   * @return true - текстура успешно загружена, false - файл не найден или
   * повреждён.
   */
  bool load(std::string fileName, std::string tag, SDL_Renderer* renderer);

  /**
   * @brief Отрисовать текстуру целиком в заданной позиции и размере.
   *
   * @param tag      Тег ранее загруженной текстуры.
   * @param x        Координата X левого верхнего угла на экране.
   * @param y        Координата Y левого верхнего угла на экране.
   * @param width    Ширина области отрисовки (и исходного фрагмента).
   * @param height   Высота области отрисовки (и исходного фрагмента).
   * @param renderer Отрисовщик, в который производится отрисовка.
   * @param flip     Режим отражения (по умолчанию SDL_FLIP_NONE).
   */
  void draw(std::string tag, float x, float y, float width, float height,
            SDL_Renderer* renderer, SDL_FlipMode flip = SDL_FLIP_NONE);

  /**
   * @brief Отрисовывает один кадр из спрайт-листа.
   *
   * Спрайт-лист представляет собой сетку кадров одинакового размера.
   * Метод вычисляет координаты нужного кадра по номеру строки и столбца.
   *
   * @param tag          Тег ранее загруженной текстуры (спрайт-листа).
   * @param x            Координата X на экране.
   * @param y            Координата Y на экране.
   * @param width        Ширина одного кадра.
   * @param height       Высота одного кадра.
   * @param currentRow   Номер строки в спрайт-листе (нумерация с 1).
   * @param currentFrame Номер кадра в строке (нумерация с 0).
   * @param renderer     Рендерер, в который производится отрисовка.
   * @param flip         Режим отражения (по умолчанию SDL_FLIP_NONE).
   */
  void drawFrame(std::string tag, float x, float y, float width, float height,
                 int currentRow, int currentFrame, SDL_Renderer* renderer,
                 SDL_FlipMode flip = SDL_FLIP_NONE);

  /**
   * @brief Выдаёт ссылку на единственный экземпляр TextureManager.
   *
   * При первом вызове создаёт статический локальный объект.
   *
   * @return Ссылка на экземпляр TextureManager.
   */
  static TextureManager& Instance() {
    static TextureManager Instance_;
    return Instance_;
  }
};