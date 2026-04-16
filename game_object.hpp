/**
 * @file game_object.hpp
 * @brief Базовый класс для всех игровых объектов.
 */

#pragma once

#include <SDL3/SDL.h>
#include <string>

/**
 * @brief Базовый класс игрового объекта.
 *
 * Содержит общие свойства: позицию, размеры, текстуру.
 * Все конкретные объекты (Player, Ghost, Dot) наследуются от него.
 */
class GameObject {
public:
  /**
   * @brief Конструктор.
   * @param x Начальная координата X.
   * @param y Начальная координата Y.
   * @param w Ширина объекта.
   * @param h Высота объекта.
   * @param texture_tag Тег текстуры в TextureManager.
   */
  GameObject(float x, float y, float w, float h,
             const std::string &texture_tag);

  /// Виртуальный деструктор.
  virtual ~GameObject() = default;

  /// Отрисовка объекта.
  virtual void render(SDL_Renderer *renderer);

  /// Обновление логики (движение и т.д.).
  virtual void update(float delta_time);

  // Геттеры.
  /// Возвращает координату X.
  float get_x() const { return x_; }

  /// Возвращает координату Y.
  float get_y() const { return y_; }

  /// Возвращает ширину.
  float get_w() const { return w_; }

  /// Возвращает высоту.
  float get_h() const { return h_; }

  /// Возвращает тег текстуры.
  const std::string &get_texture_tag() const { return texture_tag_; }

  //Сеттеры
  /// Устанавливает координату X.
  void set_x(float x) { x_ = x; }

  /// Устанавливает координату Y.
  void set_y(float y) { y_ = y; }

  /// Устанавливает позицию (X и Y одновременно).
  void set_position(float x, float y) {
    x_ = x;
    y_ = y;
  }

  /// Устанавливает тег текстуры.
  void set_texture_tag(const std::string &tag) { texture_tag_ = tag; }

protected:
  float x_ = 0.0f;          ///< Координата X (левый верхний угол)
  float y_ = 0.0f;          ///< Координата Y (левый верхний угол)
  float w_ = 0.0f;          ///< Ширина объекта в пикселях
  float h_ = 0.0f;          ///< Высота объекта в пикселях
  std::string texture_tag_; ///< Тег текстуры для отрисовки
};