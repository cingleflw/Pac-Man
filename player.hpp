/**
 * @file player.hpp
 * @brief Класс игрока (Pac-Man).
 */

#pragma once

#include "game_object.hpp"

/**
 * @brief Игрок. Управляется с клавиатуры.
 */
class Player : public GameObject {
public:
  Player(float x, float y, float w, float h, const std::string &texture_tag);

  void update(float delta_time) override;
  void set_direction(int dx, int dy); // Вызывается при нажатии клавиш

private:
  float speed_ = 200.0f; ///< Скорость в пикселях в секунду
  int dir_x_ = 0;        ///< Направление по X: -1 (влево), 0, 1 (вправо)
  int dir_y_ = 0;        ///< Направление по Y: -1 (вверх), 0, 1 (вниз)
};