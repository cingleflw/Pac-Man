/**
 * @file game_object.cpp
 * @brief Реализация базового класса GameObject.
 */
#include <iostream>

#include "game_object.hpp"
#include "texture_manager.hpp"

GameObject::GameObject(float x, float y, float w, float h,
                       const std::string &texture_tag)
    : x_(x), y_(y), w_(w), h_(h), texture_tag_(texture_tag) {
  // Тело конструктора пустое — вся инициализация в списке выше
}

void GameObject::render(SDL_Renderer *renderer) {
  TextureManager::instance().draw_frame(texture_tag_, x_, y_, w_, h_, current_row_, current_frame_, renderer);
}

void GameObject::update() {
  // Базовая реализация пуста.
  // Классы-наследники (Player, Ghost) переопределяют этот метод.
}

void GameObject::clean() {
  std::cout << "Clean game object" << std::endl;
}