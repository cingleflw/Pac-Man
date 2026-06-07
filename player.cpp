/**
 * @file player.cpp
 * @brief Реализация класса Player.
 */

#include "player.hpp"

#include <algorithm>
#include <iostream>

Player::Player(float x, float y, float w, float h,
               const std::string& texture_tag)
    : GameObject(x, y, w, h, texture_tag) {
  current_row_ = 1;
}

// Проверяет являются ли заданные направления противоположными
bool is_opposite(Direction a, Direction b) {
  return ((a == Direction::Left) && (b == Direction::Right)) ||
         ((a == Direction::Right) && (b == Direction::Left)) ||
         ((a == Direction::Up) && (b == Direction::Down)) ||
         ((a == Direction::Down) && (b == Direction::Up));
}

void Player::update(GameMap& map) {
  const int tile_size = map.get_tile_size();
  int x;
  int y;
  // Обрабатывает изменения направления игрока при вводе.
  if ((desired_direction_ != Direction::None) &&
      (desired_direction_ != current_direction_)) {
    if (is_opposite(desired_direction_, current_direction_)) {
      current_direction_ = desired_direction_;
    } else if ((static_cast<int>(x_) % tile_size == 0) &&
               (static_cast<int>(y_) % tile_size == 0)) {
      int col = static_cast<int>(x_) / tile_size;
      int row = static_cast<int>(y_) / tile_size;
      switch (desired_direction_) {
        case Direction::Left:
          x = -1;
          y = 0;
          break;
        case Direction::Right:
          x = 1;
          y = 0;
          break;
        case Direction::Up:
          x = 0;
          y = -1;
          break;
        case Direction::Down:
          x = 0;
          y = 1;
          break;
        default:
          x = 0;
          y = 0;
          break;
      }
      if (map.is_walkable_by_player(col + x, row + y)) {
        current_direction_ = desired_direction_;
      }
    }
  }
  // Обрабатывает движение игрока в текущем направлении.
  if ((current_direction_ != Direction::None) &&
      (static_cast<int>(x_) % tile_size == 0) &&
      (static_cast<int>(y_) % tile_size == 0)) {
    int col = static_cast<int>(x_) / tile_size;
    int row = static_cast<int>(y_) / tile_size;
    switch (current_direction_) {
      case Direction::Left:
        x = -1;
        y = 0;
        break;
      case Direction::Right:
        x = 1;
        y = 0;
        break;
      case Direction::Up:
        x = 0;
        y = -1;
        break;
      case Direction::Down:
        x = 0;
        y = 1;
        break;
      default:
        x = 0;
        y = 0;
        break;
    }
    if (!map.is_walkable_by_player(col + x, row + y)) {
      current_direction_ = Direction::None;
    }
    map.eating_dot(col + x, row + y, *this);
  }
  // Обновляет координаты игрока.
  if (current_direction_ != Direction::None) {
    switch (current_direction_) {
      case Direction::Left:
        x = -1;
        y = 0;
        break;
      case Direction::Right:
        x = 1;
        y = 0;
        break;
      case Direction::Up:
        x = 0;
        y = -1;
        break;
      case Direction::Down:
        x = 0;
        y = 1;
        break;
      default:
        x = 0;
        y = 0;
        break;
    }
    const float speed = current_speed();
    x_ += x * speed;
    y_ += y * speed;
    teleport();
  }
  // Обновляет текущий кадр анимации движения.
  switch (current_direction_) {
    case Direction::Right:
      current_row_ = 1;
      break;
    case Direction::Left:
      current_row_ = 2;
      break;
    case Direction::Up:
      current_row_ = 3;
      break;
    case Direction::Down:
      current_row_ = 4;
      break;
    case Direction::None:
      break;
    default:
      break;
  }
  if (current_direction_ != Direction::None) {
    current_frame_ = static_cast<int>((SDL_GetTicks() / 100) % 3);
  } else {
    current_frame_ = 0;
  }
}

void Player::teleport() {
  if (x_ < LEFT_X_BORDER) {
    x_ = 860;
  } else if (x_ >= RIGHT_X_BORDER) {
    x_ = 1.25;
  }
}

void Player::clean() {
  GameObject::clean();
  std::cout << "Player cleaned" << std::endl;
}

void Player::set_desired_direction(Direction d) { desired_direction_ = d; }

float Player::get_speed() const { return current_speed(); }

void Player::activate_energizer() {
  energizer_expires_ = SDL_GetTicks() + ENERGIZER_DURATION;
}

void Player::apply_speed_boost(float multiplier, Uint64 duration) {
  speed_boost_mult_ = multiplier;
  speed_boost_expires_ = SDL_GetTicks() + duration;
}

float Player::current_speed() const {
  const Uint64 now = SDL_GetTicks();
  float mult = 1.0f;  // Базовый множитель.

  if (now < energizer_expires_) {
    mult = std::max(mult, ENERGIZER_SPEED_MULT);
  }

  if (now < speed_boost_expires_) {
    mult = std::max(mult, speed_boost_mult_);
  }

  return DEFAULT_SPEED * mult;  // Эффекты кончились.
}

bool Player::consume_shield() {
  if (has_shield_) {
    has_shield_ = false;
    return true;
  } else {
    return false;
  }
}

void Player::reset_to_spawn(float x, float y) {
  set_position(x, y);
  current_direction_ = Direction::None;
  desired_direction_ = Direction::None;
  energizer_expires_ = 0;
  speed_boost_expires_ = 0;
  speed_boost_mult_ = 1.0f;
  has_shield_ = false;
  current_row_ = 1;
  current_frame_ = 0;
}
