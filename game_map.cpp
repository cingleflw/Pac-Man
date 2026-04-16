#pragma once

#include "game_map.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

bool GameMap::loadFromFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "GameMap: failed to open " << path << "\n";
    return false;
  }

  data_.clear();

  std::string line;
  while (std::getline(file, line)) {
    std::vector<TileType> row;
    for (char ch : line) {
      // Каждый символ - это одна цифра.
      // Вычитание '0' превращает ASCII-код в число.
      int value = ch - '0';
      row.push_back(static_cast<TileType>(value));
    }
    data_.push_back(row);
  }

  rows_ = data_.size();
  if (rows_ > 0) {
    cols_ = static_cast<int>(data_[0].size());
  } else {
    cols_ = 0;
  }

  std::cout << "GameMap: map loaded\n";
  return true;
}

TileType GameMap::getTile(int col, int row) const {
  // Запрос за пределами карты -> возвращаем стену.
  if (col < 0 || col >= cols_ || row < 0 || row >= rows_) {
    return TileType::Wall;
  }
  return data_[row][col];
}

void GameMap::setTile(int col, int row, TileType type) {
  if (col < 0 || col >= cols_ || row < 0 || row >= rows_) {
    return;
  }
  data_[row][col] = type;
}

bool GameMap::isWalkable(int col, int row) const {
  TileType tile = getTile(col, row);
  return tile != TileType::Wall && tile != TileType::GhostDoor;
}

void GameMap::render(SDL_Renderer* renderer) {
  for (int row = 0; row < rows_; ++row) {
    for (int col = 0; col < cols_; ++col) {
      TileType tile = data_[row][col];

      SDL_FRect dst = {static_cast<float>(col * tile_size_),
                       static_cast<float>(row * tile_size_),
                       static_cast<float>(tile_size_),
                       static_cast<float>(tile_size_)};

      switch (tile) {
        case TileType::Wall:
          SDL_SetRenderDrawColor(renderer, 33, 33, 222, 255);
          SDL_RenderFillRect(renderer, &dst);
          break;

        case TileType::Dot: {
          float dot_size = tile_size_ / 6;
          SDL_FRect dot = {dst.x + (tile_size_ - dot_size) / 2,
                           dst.y + (tile_size_ - dot_size) / 2, dot_size,
                           dot_size};
          SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);
          SDL_RenderFillRect(renderer, &dot);
          break;
        }

        case TileType::Energizer: {
          float e_size = tile_size_ / 2;
          SDL_FRect e = {dst.x + (tile_size_ - e_size) / 2,
                         dst.y + (tile_size_ - e_size) / 2, e_size, e_size};
          SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);
          SDL_RenderFillRect(renderer, &e);
          break;
        }

        case TileType::GhostDoor:
          SDL_SetRenderDrawColor(renderer, 255, 180, 200, 255);
          SDL_RenderFillRect(renderer, &dst);
          break;

        default:
          break;
      }
    }
  }
}