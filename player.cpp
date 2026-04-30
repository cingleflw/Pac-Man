/**
 * @file player.cpp
 * @brief Реализация класса Player.
 */

#include "player.hpp"

Player::Player(float x, float y, float w, float h,
               const std::string &texture_tag)
    : GameObject(x, y, w, h, texture_tag) {}

void Player::update(float delta_time) {
  x_ = x_ + dir_x_ * speed_ * delta_time;
  y_ = y_ + dir_y_ * speed_ * delta_time;
}

void Player::set_direction(int dx, int dy) {
  dir_x_ = dx;
  dir_y_ = dy;
}