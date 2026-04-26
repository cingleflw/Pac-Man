#include "game_map.hpp"

#include <fstream>
#include <iostream>

#include "texture_manager.hpp"

bool GameMap::load_from_file(const std::string& path,
                             const std::string& tileset_tag, int tile_size) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "GameMap: failed to open " << path << std::endl;
    return false;
  }
  tileset_tag_ = tileset_tag;
  tile_size_ = tile_size;

  data_.clear();
  dots_remaining_ = 0;
  pacman_spawn_ = {};
  ghost_spawns_.clear();

  std::string line;
  int line_number = 0;
  while (std::getline(file, line)) {
    ++line_number;
    if (line.empty()) {
      continue;
    }

    // Временный вектор для тайлов текущей строки карты.
    std::vector<TileType> row_vec;

    for (int col = 0; col < line.size(); ++col) {
      char ch = line[col];
      if (ch < '0' || ch > '9') {
        std::cerr << "GameMap: invalid char '" << ch << "' at line "
                  << line_number << std::endl;
        return false;
      }

      // Вычитание '0' превращает ASCII-код в число (символы идут подряд).
      int value = ch - '0';
      TileType tile;
      int row = data_.size();
      switch (value) {
        case 0:
          tile = TileType::Empty;
          break;
        case 1:
          tile = TileType::Wall;
          break;
        case 2:
          tile = TileType::Dot;
          break;
        case 3:
          tile = TileType::Energizer;
          break;
        case 4:
          tile = TileType::GhostDoor;
          break;

        case 5:  // Игрок.
          pacman_spawn_ = {col, row};
          tile = TileType::Empty;
          break;

        case 6:  // Blinky.
          ghost_spawns_.push_back({GhostId::Blinky, {col, row}});
          tile = TileType::Dot;
          break;

        case 7:  // Pinky.
          ghost_spawns_.push_back({GhostId::Pinky, {col, row}});
          tile = TileType::Empty;
          break;

        case 8:  // Inky.
          ghost_spawns_.push_back({GhostId::Inky, {col, row}});
          tile = TileType::Empty;
          break;

        case 9:  // Clyde.
          ghost_spawns_.push_back({GhostId::Clyde, {col, row}});
          tile = TileType::Empty;
          break;

        default:
          tile = TileType::Empty;
          break;
      }
      row_vec.push_back(tile);
    }
    data_.push_back(row_vec);
  }

  if (data_.empty()) {
    std::cerr << "GameMap: file is empty" << std::endl;
    return false;
  }

  // Проверяем, что все строки одинаковой ширины.
  const size_t expected_width = data_[0].size();
  for (size_t i = 1; i < data_.size(); ++i) {
    if (data_[i].size() != expected_width) {
      std::cerr << "GameMap: row " << i << " has width " << data_[i].size()
                << ", expected " << expected_width << std::endl;
      return false;
    }
  }

  rows_ = data_.size();
  cols_ = expected_width;

  for (const auto& row : data_) {
    for (TileType tile : row) {
      if (tile == TileType::Dot || tile == TileType::Energizer) {
        ++dots_remaining_;
      }
    }
  }

  std::cout << "GameMap: map loaded" << std::endl;
  return true;
}

TileType GameMap::get_tile(int col, int row) const {
  // Запрос за пределами карты -> возвращаем стену.
  if (col < 0 || col >= cols_ || row < 0 || row >= rows_) {
    return TileType::Wall;
  }
  return data_[row][col];
}

void GameMap::set_tile(int col, int row, TileType type) {
  if (col < 0 || col >= cols_ || row < 0 || row >= rows_) {
    return;
  }

  bool was_dot = (data_[row][col] == TileType::Dot ||
                  data_[row][col] == TileType::Energizer);
  bool is_dot = (type == TileType::Dot || type == TileType::Energizer);

  // Всегда dots_remaining_ = Dot + Energizer
  if (was_dot && !is_dot) {
    --dots_remaining_;
  }

  if (!was_dot && is_dot) {
    ++dots_remaining_;
  }

  data_[row][col] = type;
}

bool GameMap::is_walkable_by_player(int col, int row) const {
  TileType tile = get_tile(col, row);
  return tile != TileType::Wall && tile != TileType::GhostDoor;
}

bool GameMap::is_walkable_by_ghost(int col, int row) const {
  TileType tile = get_tile(col, row);
  return tile != TileType::Wall;
}

GridPos GameMap::normalize_position(int col, int row) const {
  // Выход за левый край переносит сущности в правый край, и наоборот.
  if (col < 0) {
    col = cols_ - 1;
  } else if (col >= cols_) {
    col = 0;
  }

  if (row < 0) {
    row = rows_ - 1;
  } else if (row >= rows_) {
    row = 0;
  }

  return {col, row};
}

int GameMap::compute_wall_mask(int col, int row) const {
  // Вычисляем маску (номер тайла стены во второй строке). Бит 0 -
  // слева, бит 1 - справа, бит 2 - снизу, бит 3 - сверху есть
  // стена.
  int mask = -1;
  if (get_tile(col - 1, row) == TileType::Wall) mask += 1;
  if (get_tile(col + 1, row) == TileType::Wall) mask += 2;
  if (get_tile(col, row - 1) == TileType::Wall) mask += 4;
  if (get_tile(col, row + 1) == TileType::Wall) mask += 8;

  return mask;
}

void GameMap::render(SDL_Renderer* renderer) {
  constexpr int BASE_TILE_ROW = 1;
  constexpr int WALL_TILE_ROW = 2;

  for (int row = 0; row < rows_; ++row) {
    for (int col = 0; col < cols_; ++col) {
      int row_in_tileset;
      int frame_in_row;

      if (data_[row][col] != TileType::Wall) {
        frame_in_row = static_cast<int>(data_[row][col]);
        row_in_tileset = BASE_TILE_ROW;

      } else {
        frame_in_row = compute_wall_mask(col, row);
        row_in_tileset = WALL_TILE_ROW;
      }

      TextureManager::instance().draw_frame(
          tileset_tag_, tile_to_pixel(col), tile_to_pixel(row), tile_size_,
          tile_size_, row_in_tileset, frame_in_row, renderer);
    }
  }
}