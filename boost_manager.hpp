/**
 * @file boost_manager.hpp
 * @brief Менеджер бустов: появление, время жизни, подбор и эффекты.
 *
 * Класс знает всё про все бусты. Хранит один активный бустер на поле,
 * спаунит новый по таймеру, трекает подбор игроком и применяет эффект,
 * раскидывая его по адресатам:
 *  1. Эффекты игрока -> методы Player.
 *  2. Эффекты карты -> методы GameMap (дверь призраков).
 *  3. Эффекты призраков -> интерфейс GhostController (если подключён).
 *  4. Скримеры -> интерфейс ScareTrigger (если подключён).
 *
 * Зависимость от призраков и от скримеров только через абстрактные
 * интерфейсы, поэтому систему можно собирать и тестировать без них.
 */

#pragma once

#include <SDL3/SDL.h>

#include <optional>
#include <random>
#include <vector>

#include "boost.hpp"
#include "game_map.hpp"
#include "player.hpp"

class GhostController;
class ScareTrigger;

///@brief Менеджер бустеров.
class BoostManager {
 public:
  /**
   * @brief Конструктор.
   *
   * @param[in] player Ссылка на игрока.
   * @param[in] map    Ссылка на карту.
   */
  BoostManager(Player& player, GameMap& map);

  /**
   * @brief Готовит менеджер к работе на загруженной карте.
   *
   * Запоминает тег спрайтлиста бустов, сбрасывает активный бустер и
   * собирает все достижимые игроком пустые клетки под будущий спаун
   * с помощью collect_spawn_tiles(). Первый бустер появится через
   * SPAWN_INTERVAL_MS после вызова.
   *
   * @param[in] boost_texture_tag Тег спрайтлиста бустов в TextureManager.
   * @pre Карта уже загружена.
   */
  void init(const std::string& boost_texture_tag);

  /// @brief Подключает реализацию призраков (вызывает команда призраков).
  void set_ghost_controller(GhostController* ghosts) { ghosts_ = ghosts; }

  /// @brief Подключает реализацию скримеров (Game).
  void set_scare_trigger(ScareTrigger* scare) { scare_ = scare; }

  /**
   * @brief Тик логики бустов. Вызывается каждый кадр в режиме активной игры.
   *
   * Спаунит новый бустер по истечении интервала (если поле свободно),
   * следит за временем жизни активного бустера и трекает подбор игроком.
   * При подборе применяет эффект.
   */
  void update();

  /// @brief Отрисовывает активный бустер, если он есть на поле.
  void render(SDL_Renderer* renderer);

 private:
  /// @brief Применяет эффект бустера указанного типа.
  void apply_effect(BoostType type);

  /// @brief Спаунит случайный бустер на случайной достижимой клетке.
  void spawn_random_boost();

  /// @brief Собирает все достижимые игроком пустые клетки под спаун. Реализует
  /// поиск в ширину.
  void collect_spawn_tiles();

  Player& player_;  ///< Адресат эффектов игрока.
  GameMap& map_;    ///< Источник точек и адресат эффектов карты.

  GhostController* ghosts_ = nullptr;  ///< Посредник к призракам.
  ScareTrigger* scare_ = nullptr;      ///< Посредник к скримерам.

  std::string boost_texture_tag_;      ///< Тег спрайтлиста бустов.
  std::optional<Boost> active_boost_;  ///< Буст на поле, если есть.
  GridPos active_tile_ = {-1, -1};     ///< Клетка текущего активного буста.

  Uint64 boost_despawn_time_ = 0;  ///< Время, когда буст исчезнет.
  Uint64 next_spawn_time_ = 0;     ///< Когда спаунить следующий буст.

  std::vector<GridPos> spawn_tiles_;  ///< Достижимые клетки под спаун.

  std::mt19937 rng_;  ///< Генератор для случайного выбора типа и исходов.
};
