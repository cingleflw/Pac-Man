/**
 * @file boost_manager.cpp
 * @brief Реализация класса BoostManager.
 */

#include "boost_manager.hpp"

#include <cmath>
#include <iostream>
#include <iterator>
#include <queue>

#include "ghost_controller.hpp"
#include "scare_trigger.hpp"

// Параметры баланса. Все длительности в миллисекундах.

constexpr Uint64 BOOST_LIFETIME = 12000;  ///< Сколько бустер ждёт подбора.
constexpr Uint64 SPAWN_INTERVAL = 8000;   ///< Пауза между появлениями.

/// Пул полноэкранных текстур скримеров (загружаются в TextureManager).
constexpr const char* SCARE_TAGS[] = {"scare_1", "scare_2", "scare_3",
                                      "scare_4"};
constexpr int SCARE_TAG_COUNT = 4;  ///< Количество текстур скримеров.

constexpr float GRAPE_SPEED_MULTIPLIER =
    2.0f;                                ///< Множитель скорости от Grape.
constexpr Uint64 GRAPE_DURATION = 4000;  ///< Длительность ускорения.

constexpr Uint64 BELL_FREEZE_DURATION = 4000;     ///< Заморозка призраков.
constexpr Uint64 POTION_CONFUSE_DURATION = 8000;  ///< Ослепление призраков.
constexpr Uint64 ENRAGE_DURATIOn =
    4000;  ///< Ускорение призраков (плохой исход).

constexpr int APPLE_SCORE = 1000;  ///< Очки за яблоко.

constexpr Uint64 SCARE_DURATION = 1000;  ///< Длительность скримера.

/// Шанс отравленного яблока: 1 из APPLE_POISON_DENOM.
constexpr int APPLE_POISON_DENOM = 2;

BoostManager::BoostManager(Player& player, GameMap& map)
    : player_(player), map_(map), rng_(std::random_device{}()) {}

void BoostManager::init(const std::string& boost_texture_tag) {
  boost_texture_tag_ = boost_texture_tag;
  active_boost_.reset();
  active_tile_ = {-1, -1};
  collect_spawn_tiles();
  // Первый бустер - через интервал после старта.
  next_spawn_time_ = SDL_GetTicks() + SPAWN_INTERVAL;
}

void BoostManager::update() {
  const Uint64 current_time = SDL_GetTicks();

  // Спаун по интервалу только когда поле свободно.
  if (!active_boost_ && current_time >= next_spawn_time_) {
    spawn_random_boost();
  }

  if (!active_boost_) {
    return;
  }

  // Истечение времени жизни. Следующий спаун - через интервал.
  if (current_time >= boost_despawn_time_) {
    active_boost_.reset();
    next_spawn_time_ = current_time + SPAWN_INTERVAL;
    return;
  }

  // Подбор - клетка центра игрока совпала с клеткой бустера.
  const int tile_size = map_.get_tile_size();
  const int player_col =
      static_cast<int>(player_.get_x() + player_.get_w() / 2) / tile_size;
  const int player_row =
      static_cast<int>(player_.get_y() + player_.get_h() / 2) / tile_size;

  if (player_col == active_tile_.col && player_row == active_tile_.row) {
    const BoostType type = active_boost_->get_type();
    active_boost_.reset();
    next_spawn_time_ = current_time + SPAWN_INTERVAL;
    apply_effect(type);
  }
}

void BoostManager::render(SDL_Renderer* renderer) {
  if (active_boost_) {
    active_boost_->render(renderer);
  }
}

void BoostManager::apply_effect(BoostType type) {
  switch (type) {
    case BoostType::Grape: {
      player_.apply_speed_boost(GRAPE_SPEED_MULTIPLIER, GRAPE_DURATION);
      std::uniform_int_distribution<int> poison1(1, APPLE_POISON_DENOM);
      if (poison1(rng_) == 1 && scare_) {
        std::uniform_int_distribution<int> scare_dist(0, SCARE_TAG_COUNT - 1);
        scare_->request_scare(SCARE_TAGS[scare_dist(rng_)], SCARE_DURATION);
      }
      break;
    }

    case BoostType::Bell: {
      if (ghosts_) ghosts_->freeze(BELL_FREEZE_DURATION);
      std::uniform_int_distribution<int> poison2(1, APPLE_POISON_DENOM);
      if (poison2(rng_) == 1 && scare_) {
        std::uniform_int_distribution<int> scare_dist(0, SCARE_TAG_COUNT - 1);
        scare_->request_scare(SCARE_TAGS[scare_dist(rng_)], SCARE_DURATION);
      }
      break;
    }

    case BoostType::Potion: {
      if (ghosts_) ghosts_->confuse(POTION_CONFUSE_DURATION);
      std::uniform_int_distribution<int> poison3(1, APPLE_POISON_DENOM);
      if (poison3(rng_) == 1 && scare_) {
        std::uniform_int_distribution<int> scare_dist(0, SCARE_TAG_COUNT - 1);
        scare_->request_scare(SCARE_TAGS[scare_dist(rng_)], SCARE_DURATION);
      }
      break;
    }

    case BoostType::Key: {
      map_.open_ghost_door();
      std::uniform_int_distribution<int> poison4(1, APPLE_POISON_DENOM);
      if (poison4(rng_) == 1 && scare_) {
        std::uniform_int_distribution<int> scare_dist(0, SCARE_TAG_COUNT - 1);
        scare_->request_scare(SCARE_TAGS[scare_dist(rng_)], SCARE_DURATION);
      }
      break;
    }

    case BoostType::GummyBear: {
      player_.give_shield();
      std::uniform_int_distribution<int> poison5(1, APPLE_POISON_DENOM);
      if (poison5(rng_) == 1 && scare_) {
        std::uniform_int_distribution<int> scare_dist(0, SCARE_TAG_COUNT - 1);
        scare_->request_scare(SCARE_TAGS[scare_dist(rng_)], SCARE_DURATION);
      }
      break;
    }

    case BoostType::Wand: {
      if (ghosts_) ghosts_->banish_all();
      std::uniform_int_distribution<int> poison6(1, APPLE_POISON_DENOM);
      if (poison6(rng_) == 1 && scare_) {
        std::uniform_int_distribution<int> scare_dist(0, SCARE_TAG_COUNT - 1);
        scare_->request_scare(SCARE_TAGS[scare_dist(rng_)], SCARE_DURATION);
      }
      break;
    }

    case BoostType::SurpriseBox: {
      const int weights[] = {20, 15, 15, 10, 15, 10, 8, 150};
      std::discrete_distribution<int> dist(std::begin(weights),
                                           std::end(weights));
      const int roll = dist(rng_);
      switch (roll) {
        case 6:
          if (ghosts_) ghosts_->enrage(ENRAGE_DURATIOn);
          break;
        case 7:
          if (scare_) {
            std::uniform_int_distribution<int> scare_dist(0,
                                                          SCARE_TAG_COUNT - 1);
            scare_->request_scare(SCARE_TAGS[scare_dist(rng_)], SCARE_DURATION);
          }
          break;
        default:
          apply_effect(static_cast<BoostType>(roll));
          break;
      }
      break;
    }

    case BoostType::Apple: {
      player_.add_score(APPLE_SCORE);
      std::uniform_int_distribution<int> poison(1, APPLE_POISON_DENOM);
      if (poison(rng_) == 1 && scare_) {
        std::uniform_int_distribution<int> scare_dist(0, SCARE_TAG_COUNT - 1);
        scare_->request_scare(SCARE_TAGS[scare_dist(rng_)], SCARE_DURATION);
      }
      break;
    }

    default:
      break;
  }
}

void BoostManager::spawn_random_boost() {
  if (spawn_tiles_.empty()) {
    return;
  }

  std::uniform_int_distribution<int> tile_dist(
      0, static_cast<int>(spawn_tiles_.size()) - 1);
  active_tile_ = spawn_tiles_[tile_dist(rng_)];

  std::uniform_int_distribution<int> type_dist(
      0, static_cast<int>(BoostType::Count) - 1);
  const BoostType type = static_cast<BoostType>(type_dist(rng_));

  const int tile = map_.get_tile_size();
  const float x = map_.tile_to_pixel(active_tile_.col);
  const float y = map_.tile_to_pixel(active_tile_.row);

  active_boost_.emplace(type, x, y, static_cast<float>(tile),
                        boost_texture_tag_);
  boost_despawn_time_ = SDL_GetTicks() + BOOST_LIFETIME;
}

void BoostManager::collect_spawn_tiles() {
  spawn_tiles_.clear();

  const int cols = map_.get_cols();
  const int rows = map_.get_rows();
  const GridPos start = map_.get_pacman_spawn();

  std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
  std::queue<GridPos> q;
  visited[start.row][start.col] = true;
  q.push(start);

  const int dcol[4] = {0, 0, -1, 1};
  const int drow[4] = {-1, 1, 0, 0};

  while (!q.empty()) {
    const GridPos current = q.front();
    q.pop();

    // Достижимая пустая клетка, но не сам старт игрока.
    if (map_.get_tile(current.col, current.row) == TileType::Empty &&
        !(current.col == start.col && current.row == start.row)) {
      spawn_tiles_.push_back(current);
    }

    for (int i = 0; i < 4; ++i) {
      const GridPos nb = {current.col + dcol[i], current.row + drow[i]};

      // Отсекаем соседа за границами сетки
      if (nb.col < 0 || nb.col >= cols || nb.row < 0 || nb.row >= rows) {
        continue;
      }

      // Отсекаем соседа, если уже посещён или непроходим
      if (visited[nb.row][nb.col] ||
          !map_.is_walkable_by_player(nb.col, nb.row)) {
        continue;
      }
      // Подходящего соседа помечаем посещённым и кладём в очередь
      visited[nb.row][nb.col] = true;
      q.push(nb);
    }
  }
}
