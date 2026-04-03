/**
 * @file texture_manager.cpp
 * @brief Реализация методов класса TextureManager.
 */

#include "texture_manager.hpp"

#include <SDL3/SDL_rect.h>
#include <SDL3_image/SDL_image.h>

bool TextureManager::load(std::string fileName, std::string tag,
                          SDL_Renderer* renderer) {
  // Загружаем изображение в промежуточный surface. c_str() конвертирует
  // C++-строку в C-строку, как требует SDL.
  SDL_Surface* tmpSurface = IMG_Load(fileName.c_str());
  if (tmpSurface == 0) {
    return false;  // Файл не найден или повреждён
  }

  // Конвертируем surface в текстуру.
  SDL_Texture* tmpTexture = SDL_CreateTextureFromSurface(renderer, tmpSurface);

  // Освобождаем surface - данные уже в текстуре.
  SDL_DestroySurface(tmpSurface);

  // Сохраняем текстуру в словарь по тегу.
  textureMap_[tag] = tmpTexture;
  return true;
}

void TextureManager::draw(std::string tag, float x, float y, float width,
                          float height, SDL_Renderer* renderer,
                          SDL_FlipMode flip) {
  SDL_FRect srcRect;
  SDL_FRect dstRect;

  // Берём текстуру целиком - начало в точке (0, 0).
  srcRect.x = srcRect.y = 0;
  srcRect.w = dstRect.w = width;
  srcRect.h = dstRect.h = height;

  // Размещаем в указанных координатах на экране.
  dstRect.x = x;
  dstRect.y = y;

  // 0 - угол поворота, NULL - центр вращения (по умолчанию центр текстуры).
  SDL_RenderTextureRotated(renderer, textureMap_[tag], &srcRect, &dstRect, 0,
                           NULL, flip);
}

void TextureManager::drawFrame(std::string tag, float x, float y, float width,
                               float height, int currentRow, int currentFrame,
                               SDL_Renderer* renderer, SDL_FlipMode flip) {
  SDL_FRect srcRect;
  SDL_FRect dstRect;

  // Позиция нужного кадра в спрайт-листе.
  srcRect.x = width * currentFrame;
  // Строки нумеруются с 1, поэтому вычитаем единицу.
  srcRect.y = height * (currentRow - 1);

  srcRect.w = dstRect.w = width;
  srcRect.h = dstRect.h = height;
  dstRect.x = x;
  dstRect.y = y;

  SDL_RenderTextureRotated(renderer, textureMap_[tag], &srcRect, &dstRect, 0,
                           NULL, flip);
}