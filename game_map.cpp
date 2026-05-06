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

int GameMap::compute_corner_frame(int col, int row, Corner corner) const {
  // Смещения соседей
  int x, y = 0;
  switch (corner) {
    case Corner::TopLeft:
      x = -1;
      y = -1;
      break;
    case Corner::TopRight:
      x = +1;
      y = -1;
      break;
    case Corner::BottomLeft:
      x = -1;
      y = +1;
      break;
    case Corner::BottomRight:
      x = +1;
      y = +1;
      break;
  }

  bool x_wall = (get_tile(col + x, row) == TileType::Wall);  // Сосед  по x.
  bool y_wall = (get_tile(col, row + y) == TileType::Wall);  // Сосед  по y.
  bool d_wall =
      (get_tile(col + x, row + y) == TileType::Wall);  // Сосед по диагонали.

  if (!x_wall && !y_wall) {
    return 0;  // Внешний угол.
  }
  if (x_wall && !y_wall) {
    return 1;  // Горизонтальная стена.
  }
  if (!x_wall && y_wall) {
    return 2;  // Вертикальная стена.
  }
  if (x_wall && y_wall && d_wall) {
    return 3;  // Заливка.
  }
  return 4;  // Внутренний угол.
}

SDL_FlipMode GameMap::corner_flip(Corner corner) const {
  switch (corner) {
    case Corner::TopLeft:
      return SDL_FLIP_NONE;
    case Corner::TopRight:
      return SDL_FLIP_HORIZONTAL;
    case Corner::BottomLeft:
      return SDL_FLIP_VERTICAL;
    case Corner::BottomRight:
      return static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);

    default:
      return SDL_FLIP_NONE;
  }
}

void GameMap::corner_offset(Corner corner, float& dx, float& dy) const {
  float half = tile_size_ / 2.0;
  switch (corner) {
    case Corner::TopLeft:
      dx = 0;
      dy = 0;
      break;
    case Corner::TopRight:
      dx = half;
      dy = 0;
      break;
    case Corner::BottomLeft:
      dx = 0;
      dy = half;
      break;
    case Corner::BottomRight:
      dx = half;
      dy = half;
      break;
  }
}

void GameMap::draw_wall_corner(int col, int row, Corner corner,
                               SDL_Renderer* renderer) const {
  constexpr int CORNER_TILE_ROW = 3;  // Третья строка тайлсета со стенами (если
                                      // считать строки по tile_size_ / 2.0)

  int frame = compute_corner_frame(col, row, corner);
  SDL_FlipMode flip = corner_flip(corner);

  float dx, dy;
  corner_offset(corner, dx, dy);

  float half = tile_size_ / 2.0;
  TextureManager::instance().draw_frame(tileset_tag_, tile_to_pixel(col) + dx,
                                        tile_to_pixel(row) + dy, half, half,
                                        CORNER_TILE_ROW, frame, renderer, flip);
}

void GameMap::render(SDL_Renderer* renderer) {
  constexpr int BASE_TILE_ROW = 1;

  for (int row = 0; row < rows_; ++row) {
    for (int col = 0; col < cols_; ++col) {
      TileType tile = data_[row][col];

      if (tile == TileType::Wall) {
        // Стена рисуется как четыре угловых спрайта.
        draw_wall_corner(col, row, Corner::TopLeft, renderer);
        draw_wall_corner(col, row, Corner::TopRight, renderer);
        draw_wall_corner(col, row, Corner::BottomLeft, renderer);
        draw_wall_corner(col, row, Corner::BottomRight, renderer);
      } else {
        // Обычный тайл - один спрайт на всю ячейку, кадр = TileType.
        int frame_in_row = static_cast<int>(tile);

        TextureManager::instance().draw_frame(
            tileset_tag_, tile_to_pixel(col), tile_to_pixel(row), tile_size_,
            tile_size_, BASE_TILE_ROW, frame_in_row, renderer);
      }
    }
  }
}

void GameMap::eating_dot(int col, int row){
  if ((get_tile(col, row) == TileType::Dot) || (get_tile(col, row) == TileType::Energizer)){
    set_tile(col, row, TileType::Empty);
  }
}