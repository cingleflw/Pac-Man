/**
 * @file player.hpp
 * @brief Класс игрока (Pac-Man).
 */

#pragma once

#include <vector>

#include "game_map.hpp"
#include "game_object.hpp"

/// @brief Левая и правая граница хода игрока.
const float LEFT_X_BORDER = 1.19;
const float RIGHT_X_BORDER = 864;

/// @brief Игровое поле.
class GameMap;

/// @brief Направление игрока.
enum class Direction { None, Up, Down, Left, Right };

/**
 * @brief Игрок. Управляется с клавиатуры.
 */
class Player : public GameObject {
 public:
  /**
   * @brief Конструктор.
   *
   * @param x Начальная координата X.
   * @param y Начальная координата Y.
   * @param w Ширина объекта.
   * @param h Высота объекта.
   * @param texture_tag Тег текстуры в TextureManager.
   */
  Player(float x, float y, float w, float h, const std::string& texture_tag);

  /// @brief Пустой конструктор.
  Player() {};

  /**
   * @brief Обновление логики.
   *
   * @param map Ссылка на экземпляр GameMap.
   */
  void update(GameMap& map);

  /// @brief Осуществляет перемещение игрока
  /// при попадании в тунель.
  void teleport();

  /// @brief Очищает состояние экземпляра Player.
  void clean() override;

  /** @brief Устанавливает желаемое направление движения.
   *
   * @param d Желаемое направление движения.
   */
  void set_desired_direction(Direction d);

  /// @brief Возвращает скорость игрока.
  float get_speed();

  /// @brief Активирует режим энерджайзера у Pac-man'a.
  void is_energizer();

  /// @brief Начисляет очки.
  void add_score(int points) { score_ += points; }

  /// @brief Возвращает текущий счёт.
  int get_score() const { return score_; }

 private:
  Direction current_direction_ = Direction::None;  /// Текущее направление
  Direction desired_direction_ = Direction::None;  /// Желаемое направление

  float speed_ = 0.3f;  ///< Скорость в пикселях в секунду
  int dir_x_ = 0;       ///< Направление по X: -1 (влево), 0, 1 (вправо)
  int dir_y_ = 0;       ///< Направление по Y: -1 (вверх), 0, 1 (вниз)

  int energizer_time_ = 0;  /// Время действия режима энерджайзера

  int score_ = 0;  ///< Счёт игрока
};
