#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

/// @brief Типы тайлов игрового поля.
enum class TileType : int {
  Empty = 0,      // Пустой коридор.
  Wall = 1,       // Стена.
  Dot = 2,        // Обычная точка.
  Energizer = 3,  // Большая точка.
  GhostDoor = 4   // Дверь дома призраков
};

/// @brief Игровое поле, загружаемое из текстового файла.
class GameMap {
 public:
  /// Загружает карту из файла. Возвращает false при ошибке.
  bool loadFromFile(const std::string& path);

  /// Отрисовывает всё поле.
  void render(SDL_Renderer* renderer);

  /// Возвращает тип тайла по координатам сетки.
  TileType getTile(int col, int row) const;

  /// Устанавливает тип тайла.
  void setTile(int col, int row, TileType type);

  /// Проверяет, можно ли пройти в ячейку.
  bool isWalkable(int col, int row) const;

  int getCols() const { return cols_; }
  int getRows() const { return rows_; }
  int getTileSize() const { return tile_size_; }

 private:
  std::vector<std::vector<TileType>> data_;
  int cols_ = 0;        // Столбцы.
  int rows_ = 0;        // Строки.
  int tile_size_ = 24;  // Пикселей на тайл.
};