/**
 * @file texture_manager.cpp
 * @brief Реализация методов класса TextureManager.
 */

#include "texture_manager.hpp"

#include <SDL3/SDL_rect.h>
#include <SDL3_image/SDL_image.h>

bool TextureManager::load(std::string file_name, std::string tag,
                          SDL_Renderer* renderer) {
  // Загружаем изображение в промежуточный surface. c_str() конвертирует
  // C++-строку в C-строку, как требует SDL.
  SDL_Surface* tmp_surface = IMG_Load(file_name.c_str());
  if (tmp_surface == 0) {
    return false;  // Файл не найден или повреждён
  }

  // Конвертируем surface в текстуру.
  SDL_Texture* tmp_texture =
      SDL_CreateTextureFromSurface(renderer, tmp_surface);

  // Освобождаем surface - данные уже в текстуре.
  SDL_DestroySurface(tmp_surface);

  // Сохраняем текстуру в словарь по тегу.
  texture_map_[tag] = tmp_texture;
  return true;
}

void TextureManager::draw(std::string tag, float x, float y, float width,
                          float height, SDL_Renderer* renderer,
                          SDL_FlipMode flip) {
  SDL_FRect src_rect;
  SDL_FRect dst_rect;

  // Берём текстуру целиком - начало в точке (0, 0).
  src_rect.x = src_rect.y = 0;
  src_rect.w = dst_rect.w = width;
  src_rect.h = dst_rect.h = height;

  // Размещаем в указанных координатах на экране.
  dst_rect.x = x;
  dst_rect.y = y;

  // 0 - угол поворота, NULL - центр вращения (по умолчанию центр текстуры).
  SDL_RenderTextureRotated(renderer, texture_map_[tag], &src_rect, &dst_rect, 0,
                           NULL, flip);
}

void TextureManager::draw_frame(std::string tag, float x, float y, float width,
                                float height, int current_row,
                                int current_frame, SDL_Renderer* renderer,
                                SDL_FlipMode flip) {
  SDL_FRect src_rect;
  SDL_FRect dst_rect;

  // Позиция нужного кадра в спрайт-листе.
  src_rect.x = width * current_frame;
  // Строки нумеруются с 1, поэтому вычитаем единицу.
  src_rect.y = height * (current_row - 1);

  src_rect.w = dst_rect.w = width;
  src_rect.h = dst_rect.h = height;
  dst_rect.x = x;
  dst_rect.y = y;

  SDL_RenderTextureRotated(renderer, texture_map_[tag], &src_rect, &dst_rect, 0,
                           NULL, flip);
}