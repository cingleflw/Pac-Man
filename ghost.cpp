/**
 * @file ghost.cpp
 * @brief Реализация общей логики призраков (класс Ghost).
 *
 * Подклассы Blinky/Pinky/Inky/Clyde определены целиком в ghost.hpp -
 * их логика короткая. Здесь живёт общий «двигатель»: выбор направления,
 * движение, переключение режимов и отрисовка.
 */

#include "ghost.hpp"

#include <climits>
#include <cstdlib>
#include <iostream>

#include "player.hpp"
#include "texture_manager.hpp"

namespace {
/// Множитель скорости при enrage. Критично держать speed_ * mult < 1.0 px/кадр:
/// иначе призрак «перепрыгнет» 1-пиксельную полосу выравнивания, и
/// is_tile_aligned() перестанет ловить границу тайла — призрак промчится
/// сквозь перекрёсток, не приняв решения о повороте.
constexpr float ENRAGE_SPEED_MULT = 1.6f;  // 0.3 * 1.6 = 0.48 < 1.0 — безопасно
}  // namespace

// ---------------------------------------------------------------------------
//  Свободные функции-помощники.
// ---------------------------------------------------------------------------

Direction opposite(Direction d) {
  switch (d) {
    case Direction::Up:
      return Direction::Down;
    case Direction::Down:
      return Direction::Up;
    case Direction::Left:
      return Direction::Right;
    case Direction::Right:
      return Direction::Left;
    default:
      return Direction::None;
  }
}

GridPos step(GridPos pos, Direction d) {
  switch (d) {
    case Direction::Up:
      return {pos.col, pos.row - 1};
    case Direction::Down:
      return {pos.col, pos.row + 1};
    case Direction::Left:
      return {pos.col - 1, pos.row};
    case Direction::Right:
      return {pos.col + 1, pos.row};
    default:
      return pos;
  }
}

// ---------------------------------------------------------------------------
//  Конструктор.
// ---------------------------------------------------------------------------

Ghost::Ghost(float x, float y, float w, float h, const std::string& texture_tag,
             int tile_size)
    : GameObject(x, y, w, h, texture_tag), tile_size_(tile_size) {
  // Стартовая клетка = дом (цель в режиме Eaten).
  home_tile_ = {static_cast<int>(x) / tile_size_,
                static_cast<int>(y) / tile_size_};
  current_row_ = 1;
  current_frame_ = 0;
}

// ---------------------------------------------------------------------------
//  Главный цикл обновления.
// ---------------------------------------------------------------------------

void Ghost::update(const GameMap& map, const Player& player,
                   const Ghost& blinky) {
  const Uint64 now = SDL_GetTicks();

  // Заморозка (Bell): стоим на месте, решения не принимаем.
  if (now < frozen_until_) {
    return;
  }
  const bool confused = now < confused_until_;

  if (is_tile_aligned()) {
    if (mode_ == GhostMode::Eaten && current_tile().col == home_tile_.col &&
        current_tile().row == home_tile_.row) {
      mode_ = GhostMode::Chase;
    }

    Direction new_dir;
    // Случайное блуждание: при испуге — всегда; при ослеплении — во всех
    // режимах, КРОМЕ Eaten («глаза» обязаны добраться домой, иначе зависнут).
    if (mode_ == GhostMode::Frightened ||
        (confused && mode_ != GhostMode::Eaten)) {
      new_dir = choose_random_direction(map);
    } else {
      GridPos target;
      switch (mode_) {
        case GhostMode::Chase:
          target = get_chase_target(player, blinky);
          break;
        case GhostMode::Eaten:
          target = home_tile_;
          break;
        case GhostMode::Scatter:
        default:
          target = get_scatter_target();
          break;
      }
      new_dir = choose_direction(map, target);
    }

    if (new_dir != Direction::None && new_dir != direction_) {
      GridPos t = current_tile();
      x_ = static_cast<float>(t.col * tile_size_);
      y_ = static_cast<float>(t.row * tile_size_);
      direction_ = new_dir;
    }
  }

  move_one_step();
  handle_tunnel(map);
}

// ---------------------------------------------------------------------------
//  Выбор направления.
// ---------------------------------------------------------------------------

Direction Ghost::choose_direction(const GameMap& map, GridPos target) const {
  // Порядок перебора задаёт приоритет при равной дистанции: Up > Left > Down >
  // Right. Вместе со строгим сравнением «<» это сохраняет первого победителя.
  const Direction order[4] = {Direction::Up, Direction::Left, Direction::Down,
                              Direction::Right};
  const GridPos me = current_tile();
  const Direction reverse = opposite(direction_);

  Direction best = Direction::None;
  long long best_dist = LLONG_MAX;

  for (Direction d : order) {
    if (d == reverse) continue;  // разворачиваться назад нельзя
    GridPos next = step(me, d);
    // Проходимость проверяем по нормализованным координатам, чтобы туннель
    // на краю карты считался проходимым.
    GridPos norm = map.normalize_position(next.col, next.row);
    if (!map.is_walkable_by_ghost(norm.col, norm.row)) continue;

    // Дистанцию считаем по «сырым» координатам - призрак не «знает» про
    // срезку через туннель, как и в оригинале. Квадрат дистанции, без корня.
    long long dx = next.col - target.col;
    long long dy = next.row - target.row;
    long long dist = dx * dx + dy * dy;
    if (dist < best_dist) {
      best_dist = dist;
      best = d;
    }
  }

  // Тупик (валиден только разворот) - разрешаем развернуться.
  if (best == Direction::None) best = reverse;
  return best;
}

Direction Ghost::choose_random_direction(const GameMap& map) const {
  const Direction order[4] = {Direction::Up, Direction::Left, Direction::Down,
                              Direction::Right};
  const GridPos me = current_tile();
  const Direction reverse = opposite(direction_);

  Direction candidates[4];
  int count = 0;
  for (Direction d : order) {
    if (d == reverse) continue;
    GridPos next = step(me, d);
    GridPos norm = map.normalize_position(next.col, next.row);
    if (!map.is_walkable_by_ghost(norm.col, norm.row)) continue;
    candidates[count++] = d;
  }

  if (count == 0) return reverse;  // тупик
  return candidates[std::rand() % count];
}

// ---------------------------------------------------------------------------
//  Движение.
// ---------------------------------------------------------------------------

bool Ghost::is_tile_aligned() const {
  return (static_cast<int>(x_) % tile_size_ == 0) &&
         (static_cast<int>(y_) % tile_size_ == 0);
}

void Ghost::move_one_step() {
  const float speed =
      (SDL_GetTicks() < enraged_until_) ? speed_ * ENRAGE_SPEED_MULT : speed_;
  switch (direction_) {
    case Direction::Left:
      x_ -= speed;
      break;
    case Direction::Right:
      x_ += speed;
      break;
    case Direction::Up:
      y_ -= speed;
      break;
    case Direction::Down:
      y_ += speed;
      break;
    default:
      break;
  }
}

void Ghost::handle_tunnel(const GameMap& map) {
  // Горизонтальный туннель: вышел за левый край - появился у правого, и
  // наоборот.
  const float width = static_cast<float>(map.get_cols() * tile_size_);
  if (x_ < 0) {
    x_ += width;
  } else if (x_ >= width) {
    x_ -= width;
  }
}

// ---------------------------------------------------------------------------
//  Переключение режимов.
// ---------------------------------------------------------------------------

void Ghost::set_mode(GhostMode mode) {
  if (mode_ == GhostMode::Eaten) return;  // было: Frightened || Eaten
  if (mode != mode_) {
    direction_ = opposite(direction_);
    mode_ = mode;
  }
}

void Ghost::frighten() {
  if (mode_ == GhostMode::Eaten) return;  // «глаза» не пугаются
  direction_ = opposite(direction_);      // разворот при входе в испуг
  mode_ = GhostMode::Frightened;
}

void Ghost::get_eaten() { mode_ = GhostMode::Eaten; }

// ---------------------------------------------------------------------------
//  Отрисовка.
//  ВАЖНО: номера строк и теги текстур ниже - под спрайт-лист с раскладкой
//  как у Player (Right=1, Left=2, Up=3, Down=4). Подгони под свои ассеты.
// ---------------------------------------------------------------------------

void Ghost::render(SDL_Renderer* renderer) {
  // Простая двухкадровая анимация «шевеления».
  current_frame_ = static_cast<int>((SDL_GetTicks() / 150) % 2);

  // Строка спрайта по направлению взгляда.
  int row = 1;
  switch (direction_) {
    case Direction::Right:
      row = 1;
      break;
    case Direction::Left:
      row = 2;
      break;
    case Direction::Up:
      row = 3;
      break;
    case Direction::Down:
      row = 4;
      break;
    default:
      row = 1;
      break;
  }

  // По умолчанию - собственный цвет призрака (texture_tag_).
  std::string tag = texture_tag_;

  if (mode_ == GhostMode::Frightened) {
    tag = "ghost_frightened";  // общий синий спрайт - загрузи под этим тегом
    row = 1;
  } else if (mode_ == GhostMode::Eaten) {
    tag = "ghost_eyes";  // общие «глаза» - загрузи под этим тегом
  }

  TextureManager::instance().draw_frame(tag, x_, y_, w_, h_, row,
                                        current_frame_, renderer);
}

void Ghost::clean() {
  GameObject::clean();
  std::cout << "Ghost cleaned" << std::endl;
}
