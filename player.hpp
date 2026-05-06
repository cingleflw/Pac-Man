/**
 * @file player.hpp
 * @brief Класс игрока (Pac-Man).
 */

#pragma once

#include <vector>

#include "game_object.hpp"

const int X_BORDER = 882;
const int Y_BORDER = 419;

/**
 * @brief Игрок. Управляется с клавиатуры.
 */
class Player : public GameObject {
public:
  Player(float x, float y, float w, float h, const std::string &texture_tag);
  Player() {};

  void update() override;
  void set_direction(int dx, int dy); // Вызывается при нажатии клавиш
  void clean() override;

  void set_moving(std::vector<bool> b);
  float get_speed();

private:
  float speed_ = 8.0f; ///< Скорость в пикселях в секунду
  int dir_x_ = 0;        ///< Направление по X: -1 (влево), 0, 1 (вправо)
  int dir_y_ = 0;        ///< Направление по Y: -1 (вверх), 0, 1 (вниз)

  std::vector<bool> moving_{false, false, false, false};
  int x_border_ = 882;
  int y_border_ = 419;
};