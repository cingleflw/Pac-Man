/**
 * @file ghost_controller.hpp
 * @brief Абстрактный интерфейс управления призраками.
 *
 * Это посредник между системой бустов и системой призраков. Система бустов
 * зависит только от этого интерфейса и ничего не знает о
 * реализации призраков. Команда призраков реализует
 * GhostController у себя и передаёт указатель в
 * BoostManager через set_ghost_controller().
 *
 * @note Пока призраков нет указатель в BoostManager равен nullptr,
 * и все эффекты бустов, направленные на призраков не выполняются.
 * Так мы можем разрабатывать бусты и призраков независимо.
 */

#pragma once

#include <SDL3/SDL.h>

/**
 * @brief Интерфейс воздействия на всех призраков.
 *
 * Каждый метод соответствует эффекту одного из бустов. Длительности
 * измеряются в миллисекундах (через SDL_GetTicks()),
 * а не в кадрах, поэтому поведение не зависит от FPS.
 */
class GhostController {
 public:
  virtual ~GhostController() = default;

  /// @brief Останавливает всех призраков на duration миллисекунд (буст Bell).
  virtual void freeze(Uint64 duration) = 0;

  /// @brief Лишает призраков цели - шатаются случайно duration миллисекунд, но
  /// остаются опасными (то есть не Frightened, буст Potion).
  virtual void confuse(Uint64 duration) = 0;

  ///@brief Переводит призраков в съедобный режим на duration миллисекунд.
  virtual void frighten(Uint64 duration) = 0;

  /// @brief Отправляет всех призраков домой (режим Eaten, буст Wand).
  virtual void banish_all() = 0;

  /// @brief Временно ускоряет призраков на duration миллисекунд (плохой
  /// исход Surprise box).
  virtual void enrage(Uint64 duration) = 0;
};
