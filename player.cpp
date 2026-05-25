/**
 * @file player.cpp
 * @brief Реализация класса Player.
 */

#include <iostream>

#include "player.hpp"

Player::Player(float x, float y, float w, float h,
               const std::string &texture_tag)
    : GameObject(x, y, w, h, texture_tag) {
      current_row_ = 1;
    }

// Проверяет являются ли заданные направления противоположными
bool is_opposite(Direction a, Direction b){
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
  (desired_direction_ != current_direction_)){
    if (is_opposite(desired_direction_, current_direction_)) {
      current_direction_ = desired_direction_;
    }
    else if ((static_cast<int>(x_) % tile_size == 0) && 
    (static_cast<int>(y_) % tile_size == 0)){
      int col = static_cast<int>(x_) / tile_size;
      int row = static_cast<int>(y_) / tile_size;
      switch(desired_direction_){
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
      if (map.is_walkable_by_player(col + x, row + y)){
        current_direction_ = desired_direction_;
      }
    } 
  }
  // Обрабатывает движение игрока в текущем направлении.
  if ((current_direction_ != Direction::None) &&
   (static_cast<int>(x_) % tile_size == 0) &&
    (static_cast<int>(y_) % tile_size == 0)){
      int col = static_cast<int>(x_) / tile_size;
      int row = static_cast<int>(y_) / tile_size;
      switch(current_direction_){
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
      if (!map.is_walkable_by_player(col + x, row + y)){
        current_direction_ = Direction::None;
      }
      map.eating_dot(col + x, row + y, *this);
      if (energizer_time_ > 0){
        energizer_time_--;
        if (energizer_time_ == 0){
          speed_ = 0.3f;
        }
      }
    }
    // Обновляет координаты игрока.
    if (current_direction_ != Direction::None){
      switch(current_direction_){
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
      x_ += x * speed_;
      y_ += y * speed_;
      teleport();
    }
    // Обновляет текущий кадр анимации движения.
    switch(current_direction_){
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
    if (current_direction_ != Direction::None){
      current_frame_ = static_cast<int>((SDL_GetTicks() / 100) % 3);
    }
    else {
      current_frame_ = 0;
    }
}

void Player::teleport() {
  if (x_ < LEFT_X_BORDER){
    x_ = 860;
  }
  else if (x_ >= RIGHT_X_BORDER){
    x_ = 1.25;
  }
}

void Player::clean(){
  GameObject::clean();
  std::cout << "Player cleaned" << std::endl;
}

void Player::set_desired_direction(Direction d){
  desired_direction_ = d;
}

float Player::get_speed(){
  return speed_;
}

void Player::is_energizer(){
  speed_ = 0.6f;
  energizer_time_ = 200;
}