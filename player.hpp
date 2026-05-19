/**
 * @file player.hpp
 * @brief Класс игрока (Pac-Man).
 */

#pragma once

#include <vector>

#include "game_object.hpp"
#include "game_map.hpp"

const float LEFT_X_BORDER = 1.19;
const float RIGHT_X_BORDER = 864;

/**
 * @brief Игрок. Управляется с клавиатуры.
 */

class GameMap;
 
enum class Direction {None, Up, Down, Left, Right};

class Player : public GameObject {
public:
  Player(float x, float y, float w, float h, const std::string &texture_tag);
  Player() {};

  void update(GameMap& map);
  void teleport();
  void clean() override;

  void set_desired_direction(Direction d);

  float get_speed();

  void is_energizer();

private:
  Direction current_direction_ = Direction::None;
  Direction desired_direction_ = Direction::None;

  float speed_ = 0.3f; ///< Скорость в пикселях в секунду
  int dir_x_ = 0;        ///< Направление по X: -1 (влево), 0, 1 (вправо)
  int dir_y_ = 0;        ///< Направление по Y: -1 (вверх), 0, 1 (вниз)

  int energizer_time_ = 0;
};