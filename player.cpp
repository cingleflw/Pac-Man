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

void Player::update() {
  bool is_moving = false;
  if (moving_[0]){
    current_row_ = 1;
    is_moving = true;
  }
  else if (moving_[1]){
    current_row_ = 2;
    is_moving = true;
  }
  else if (moving_[2]){
    current_row_ = 3;
    is_moving = true;
  }
  else if (moving_[3]){
    current_row_ = 4;
    is_moving = true;
  }
  if (is_moving){
    current_frame_ = int((SDL_GetTicks() / 100) % 3);
  }
  else {
    current_frame_ = 0;
  }
}

void Player::set_direction(int dx, int dy) {
  dir_x_ = dx;
  dir_y_ = dy;
  x_ += dx;
  y_ += dy;
  if ((x_ > X_BORDER) && (y_ == Y_BORDER)){
    x_ = 0;
  }
  else if ((x_ < 0) && (y_ == Y_BORDER)){
    x_ = X_BORDER;
  }
}

void Player::clean(){
  GameObject::clean();
  std::cout << "Player cleaned" << std::endl;
}

void Player::set_moving(std::vector<bool> b){
  moving_ = b;
}

float Player::get_speed(){
  return speed_;
}