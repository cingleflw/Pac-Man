/**
 * @file ghost.hpp
 * @brief Призраки-враги (Blinky, Pinky, Inky, Clyde).
 *
 * Все четыре призрака разделяют один алгоритм движения (жадный выбор
 * соседней клетки, ближайшей к цели) и отличаются только способом
 * вычисления целевой клетки. Поэтому в основе лежит абстрактный класс
 * Ghost с общим update(), а конкретные призраки переопределяют ровно
 * два метода: get_chase_target() и get_scatter_target().
 */

#pragma once

#include <SDL3/SDL.h>

#include <string>

#include "game_map.hpp"  // GridPos, GhostId, и (через player.hpp) Direction, Player
#include "game_object.hpp"

class Player;
class GameMap;

/**
 * @brief Режим поведения призрака.
 *
 * Scatter и Chase переключаются глобально по таймеру (этим управляет Game).
 * Frightened включается при поедании энерджайзера, Eaten - когда Pac-Man
 * съел призрака и тот возвращается домой в виде «глаз».
 */
enum class GhostMode {
  Scatter,     ///< Разбегание по углам карты.
  Chase,       ///< Преследование (индивидуальная цель у каждого призрака).
  Frightened,  ///< Испуг: синий, движется случайно, уязвим.
  Eaten,       ///< Съеден: «глаза» возвращаются в дом призраков.
};

/// @brief Возвращает направление, противоположное заданному.
Direction opposite(Direction d);

/// @brief Возвращает клетку, соседнюю с pos в направлении d (на один тайл).
GridPos step(GridPos pos, Direction d);

/**
 * @brief Абстрактный базовый класс призрака.
 *
 * Владеет общим состоянием (режим, направление, скорость) и общей
 * логикой движения. Конкретное «поведение» призрака целиком заключено
 * в переопределяемых get_chase_target() и get_scatter_target().
 */
class Ghost : public GameObject {
 public:
  /**
   * @brief Конструктор.
   *
   * Начальная позиция (x, y) одновременно считается «домашней» клеткой:
   * именно туда призрак стремится в режиме Eaten.
   *
   * @param x           Начальная координата X (левый верхний угол).
   * @param y           Начальная координата Y (левый верхний угол).
   * @param w           Ширина спрайта.
   * @param h           Высота спрайта.
   * @param texture_tag Тег текстуры в TextureManager.
   * @param tile_size   Размер тайла в пикселях (для перевода пикселей в
   * клетки).
   */
  Ghost(float x, float y, float w, float h, const std::string& texture_tag,
        int tile_size);

  /// @brief Пустой конструктор.
  Ghost() {};

  /**
   * @brief Обновление логики призрака за один кадр.
   *
   * В момент выравнивания по тайлу выбирает новое направление в сторону
   * текущей цели (зависит от режима), затем сдвигается на шаг.
   *
   * @param[in] map    Игровое поле (только для запросов проходимости).
   * @param[in] player Игрок - источник цели в режиме Chase.
   * @param[in] blinky Ссылка на Blinky: нужна Inky для вычисления цели.
   *                   Для самого Blinky передаётся он же (значение не
   *                   используется).
   */
  void update(const GameMap& map, const Player& player, const Ghost& blinky);

  /// @brief Отрисовка с учётом режима (направление / синий испуг / глаза).
  void render(SDL_Renderer* renderer) override;

  /// @brief Очищает состояние экземпляра.
  void clean() override;

  /**
   * @brief Переключает Scatter <-> Chase (вызывается из Game по таймеру).
   *
   * При смене между Scatter и Chase призрак немедленно разворачивается
   * на 180° - это каноничное поведение оригинала. Не трогает режимы
   * Frightened и Eaten.
   *
   * @param[in] mode Новый режим (ожидается Scatter или Chase).
   */
  void set_mode(GhostMode mode);

  /// @brief Переводит призрака в режим Frightened (съеден энерджайзер).
  void frighten();

  /// @brief Переводит призрака в режим Eaten (его съел Pac-Man).
  void get_eaten();

  /// @brief Останавливает призрака на duration_ms мс (буст Bell).
  void freeze(Uint64 duration_ms) {
    frozen_until_ = SDL_GetTicks() + duration_ms;
  }

  /// @brief Ослепляет призрака: блуждает случайно duration_ms мс, но остаётся
  /// ОПАСНЫМ (буст Potion). Режим и уязвимость не меняются — это не Frightened.
  void confuse(Uint64 duration_ms) {
    confused_until_ = SDL_GetTicks() + duration_ms;
  }

  /// @brief Временно ускоряет призрака на duration_ms мс (Surprise box).
  void enrage(Uint64 duration_ms) {
    enraged_until_ = SDL_GetTicks() + duration_ms;
  }

  /// @brief Текущий режим призрака.
  GhostMode get_mode() const { return mode_; }

  /// @brief true, если призрака сейчас можно съесть.
  bool is_frightened() const { return mode_ == GhostMode::Frightened; }

  /// @brief Текущее направление движения.
  Direction get_direction() const { return direction_; }

  /**
   * @brief Клетка, в которой призрак находится сейчас.
   *
   * Публичный, потому что Inky опирается на позицию Blinky.
   */
  GridPos current_tile() const { return tile_of(*this); }

 protected:
  /**
   * @brief Цель в режиме Chase - «характер» призрака.
   *
   * Единственный метод, который полностью определяет индивидуальное
   * поведение. Переопределяется в каждом подклассе.
   *
   * @param[in] player Игрок.
   * @param[in] blinky Blinky (нужен только Inky).
   * @return Целевая клетка.
   */
  virtual GridPos get_chase_target(const Player& player,
                                   const Ghost& blinky) const = 0;

  /// @brief Угол карты, в который призрак уходит в режиме Scatter.
  virtual GridPos get_scatter_target() const = 0;

  /// @brief Переводит пиксельную позицию любого объекта в координаты клетки.
  GridPos tile_of(const GameObject& obj) const {
    return {static_cast<int>(obj.get_x()) / tile_size_,
            static_cast<int>(obj.get_y()) / tile_size_};
  }

  GhostMode mode_ = GhostMode::Scatter;    ///< Текущий режим.
  Direction direction_ = Direction::Left;  ///< Текущее направление.
  float speed_ = 0.2f;                     ///< Скорость в пикселях за кадр.
  int tile_size_ = 0;                      ///< Размер тайла в пикселях.

  GridPos home_tile_ = {};  ///< Цель в режиме Eaten (дом).

  Uint64 frozen_until_ = 0;    ///< До этого момента (мс) призрак заморожен.
  Uint64 confused_until_ = 0;  ///< До этого момента (мс) призрак ослеплён.
  Uint64 enraged_until_ = 0;   ///< До этого момента (мс) призрак ускорен.

 private:
  /**
   * @brief Жадный выбор направления к цели.
   *
   * Перебирает соседей в порядке приоритета Up, Left, Down, Right
   * (этот порядок разрешает ничьи по дистанции), отбрасывает стены и
   * разворот назад, выбирает ближайшего к target по квадрату дистанции.
   */
  Direction choose_direction(const GameMap& map, GridPos target) const;

  /// @brief Случайное допустимое направление (режим Frightened).
  Direction choose_random_direction(const GameMap& map) const;

  /// @brief true, если призрак стоит ровно на границе тайла.
  bool is_tile_aligned() const;

  /// @brief Сдвигает призрака на speed_ пикселей в текущем направлении.
  void move_one_step();

  /// @brief Телепортация через туннель (нормализация координат по краям).
  void handle_tunnel(const GameMap& map);
};

// ---------------------------------------------------------------------------
//  Конкретные призраки. Логика целей - короткая, поэтому вынесена прямо сюда.
//  Координаты Scatter-углов ПОДГОНИ под размеры своей карты.
// ---------------------------------------------------------------------------

/// @brief Сдвиг цели Pinky вперёд игрока, в тайлах.
const int PINKY_LOOKAHEAD = 4;
/// @brief Сдвиг опорной точки Inky вперёд игрока, в тайлах.
const int INKY_LOOKAHEAD = 2;
/// @brief Квадрат радиуса «трусости» Clyde (8 тайлов): 8 * 8 = 64.
const int CLYDE_FEAR_RADIUS_SQUARED = 64;

/**
 * @brief Сдвигает клетку на n тайлов в направлении d.
 *
 * Воспроизводит «overflow bug» оригинала: при движении вверх к сдвигу
 * вверх добавляется такой же сдвиг влево. Это каноничная деталь.
 */
inline GridPos lookahead(GridPos from, Direction d, int n) {
  for (int i = 0; i < n; ++i) from = step(from, d);
  if (d == Direction::Up) {
    for (int i = 0; i < n; ++i) from = step(from, Direction::Left);
  }
  return from;
}

/**
 * @brief Blinky (красный) - «преследователь».
 *
 * Цель - текущая клетка Pac-Man. Гонится по пятам.
 */
class Blinky : public Ghost {
 public:
  using Ghost::Ghost;

 protected:
  GridPos get_chase_target(const Player& player,
                           const Ghost& /*blinky*/) const override {
    return tile_of(player);
  }
  GridPos get_scatter_target() const override { return {25, 0}; }  // верх-право
};

/**
 * @brief Pinky (розовый) - «засадчик».
 *
 * Цель - на PINKY_LOOKAHEAD тайлов впереди Pac-Man по его направлению.
 */
class Pinky : public Ghost {
 public:
  using Ghost::Ghost;

 protected:
  GridPos get_chase_target(const Player& player,
                           const Ghost& /*blinky*/) const override {
    return lookahead(tile_of(player), player.get_direction(), PINKY_LOOKAHEAD);
  }
  GridPos get_scatter_target() const override { return {2, 0}; }  // верх-лево
};

/**
 * @brief Inky (голубой) - «непредсказуемый».
 *
 * Берёт точку в INKY_LOOKAHEAD тайлах впереди Pac-Man, проводит из
 * позиции Blinky в неё вектор и удваивает его: target = 2 * pivot - blinky.
 */
class Inky : public Ghost {
 public:
  using Ghost::Ghost;

 protected:
  GridPos get_chase_target(const Player& player,
                           const Ghost& blinky) const override {
    GridPos pivot =
        lookahead(tile_of(player), player.get_direction(), INKY_LOOKAHEAD);
    GridPos b = blinky.current_tile();
    return {2 * pivot.col - b.col, 2 * pivot.row - b.row};
  }
  GridPos get_scatter_target() const override { return {27, 35}; }  // низ-право
};

/**
 * @brief Clyde (оранжевый) - «трусишка».
 *
 * Далеко от Pac-Man (> 8 тайлов) ведёт себя как Blinky; вблизи -
 * убегает в свой scatter-угол.
 */
class Clyde : public Ghost {
 public:
  using Ghost::Ghost;

 protected:
  GridPos get_chase_target(const Player& player,
                           const Ghost& /*blinky*/) const override {
    GridPos me = current_tile();
    GridPos p = tile_of(player);
    int dx = p.col - me.col;
    int dy = p.row - me.row;
    if (dx * dx + dy * dy > CLYDE_FEAR_RADIUS_SQUARED) {
      return p;  // далеко - преследую
    }
    return get_scatter_target();  // близко - отступаю
  }
  GridPos get_scatter_target() const override { return {0, 35}; }  // низ-лево
};
