/**
 * @file boost.cpp
 * @brief Реализация класса Boost.
 */

#include "boost.hpp"

/// Все бусты лежат в первой строке спрайтлиста, кадр совпадает с числовым
/// значением BoostType.
Boost::Boost(BoostType type, float x, float y, float size,
             const std::string& texture_tag)
    : GameObject(x, y, size, size, texture_tag), type_(type) {
  current_row_ = 1;
  current_frame_ = static_cast<int>(type);
}
