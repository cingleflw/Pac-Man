#include "texture_manager.hpp"

#include <SDL3/SDL_rect.h>         // Для работы с прямоугольниками (SDL_FRect)
#include <SDL3_image/SDL_image.h>  // Для загрузки изображений (IMG_Load)

bool TextureManager::load(std::string fileName, std::string tag,
                          SDL_Renderer* renderer) {
  // 1. Загружаем изображение в surface (промежуточную поверхность)
  //    c_str() конвертирует C++-строку в C-строку, как требует SDL
  SDL_Surface* tmpSurface = IMG_Load(fileName.c_str());
  if (tmpSurface == 0) {
    return false;  // Файл не найден или повреждён
  }

  // 2. Создаём текстуру из surface (переносим в видеопамять)
  SDL_Texture* tmpTexture = SDL_CreateTextureFromSurface(renderer, tmpSurface);

  // 3. Освобождаем surface - он больше не нужен, данные уже в текстуре
  SDL_DestroySurface(tmpSurface);

  // 4. Сохраняем текстуру в словарь по тегу
  textureMap_[tag] = tmpTexture;
  return true;
}

void TextureManager::draw(std::string tag, float x, float y, float width,
                          float height, SDL_Renderer* renderer,
                          SDL_FlipMode flip) {
  SDL_FRect srcRect;  // Исходный прямоугольник (какую часть текстуры берём)
  SDL_FRect dstRect;  // Целевой прямоугольник (куда и какого размера рисуем)

  // Берём текстуру целиком - начало в точке (0, 0)
  srcRect.x = srcRect.y = 0;
  srcRect.w = dstRect.w = width;
  srcRect.h = dstRect.h = height;

  // Размещаем в указанных координатах на экране
  dstRect.x = x;
  dstRect.y = y;

  // Отрисовка с возможностью поворота и отражения
  // 0 - угол поворота, NULL - центр вращения (по умолчанию центр текстуры)
  SDL_RenderTextureRotated(renderer, textureMap_[tag], &srcRect, &dstRect, 0,
                           NULL, flip);
}

void TextureManager::drawFrame(std::string tag, float x, float y, float width,
                               float height, int currentRow, int currentFrame,
                               SDL_Renderer* renderer, SDL_FlipMode flip) {
  SDL_FRect srcRect;  // Какой кадр вырезаем из спрайт-листа
  SDL_FRect dstRect;  // Куда рисуем на экране

  // Вычисляем позицию нужного кадра в спрайт-листе:
  // По горизонтали: номер кадра * ширина одного кадра
  srcRect.x = width * currentFrame;
  // По вертикали: (номер строки - 1) * высота одного кадра
  // Строки нумеруются с 1, поэтому вычитаем единицу
  srcRect.y = height * (currentRow - 1);

  srcRect.w = dstRect.w = width;
  srcRect.h = dstRect.h = height;
  dstRect.x = x;
  dstRect.y = y;

  SDL_RenderTextureRotated(renderer, textureMap_[tag], &srcRect, &dstRect, 0,
                           NULL, flip);
}