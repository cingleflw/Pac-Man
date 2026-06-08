/**
 * @file game_map.hpp
 * @brief Игровое поле - тайловая карта, загружаемая из текстового
 * файла.
 *
 * Содержит перечисления для типов тайлов, структуры для координат и стартовых
 * позиций, класс GameMap, который владеет данными уровня и
 * предоставляет интерфейс для его загрузки, отрисовки и запросов.
 */

#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

#include "player.hpp"

/// @brief класс игрока (Pac-man).
class Player;

/**
 * @brief Типы тайлов игрового поля.
 *
 * Числовые значения соответствуют символам в файле карты и
 * индексам кадров в первой строке тайлсета.
 */

enum class TileType : int {
  Empty = 0,      // Пустой коридор.
  Wall = 1,       // Стена.
  Dot = 2,        // Обычная точка.
  Energizer = 3,  // Большая точка.
  GhostDoor = 4,  // Дверь дома призраков.
};

/// @brief Координаты ячейки на карты.
struct GridPos {
  int col;  // Столбец
  int row;  // Строка
};

/// @brief Идентификатор призрака.
enum class GhostId {
  Blinky,  // Красный.
  Pinky,   // Розовый.
  Inky,    // Голубой.
  Clyde,   // Оранжевый.
};

/// @brief Стартовая позиция одного призрака.
struct GhostSpawn {
  GhostId id;        // Какой призрак.
  GridPos position;  // В какой ячейке.
};

/**
 * @brief Угол тайла. Используется при рендере стен через
 * автотайлинг - каждый тайл рисуется как четыре угловых спрайта,
 * выбираемых в зависимости от соседей.
 */
enum class Corner {
  TopLeft,
  TopRight,
  BottomLeft,
  BottomRight,
};

/**
 * @brief Игровое поле, загружаемое из текстового файла.
 *
 * Класс владеет сеткой тайлов уровня, информацией о стартовых
 * позициях персонажей и тегом тайлсета для отрисовки. Предоставляет
 * интерфейс для запросов (что в ячейке, можно ли пройти, сколько точек
 * осталось).
 */
class GameMap {
 public:
  /**
   * @brief Загружает карту из текстового файла.
   *
   * Файл состоит из строк одинаковой длины, где каждый символ -
   * это цифра от 0 до 9, соответствующая значению TileType и маркеру
   * спауна. Подсчитывает число точек и
   * запоминает стартовые позиции персонажей.
   *
   * @param[in] path         Путь к файлу карты.
   * @param[in] tileset_tag  Тег текстуры спрайтлиста из TextureManager.
   * Используется в render().
   * @param[in] tile_size    Пикселей на тайл. По умолчанию 32.
   * @pre Текстура с тегом tileset_tag должна быть загружена в TextureManager.
   * @post При успехе поля data_, cols_, rows_, dots_remaining_,
   *       pacman_spawn_, ghost_spawns_, tileset_tag_, tile_size_ заполнены.
   * @note Маркеры спауна 5 для игрока, 6 для Blinky, 7 для Pinky, 8 для Inky, 9
   * для Clyde не попадают в data_. В точке маркера остаётся подходящий базовый
   * тайл, а координаты сохраняются в spawn-поля класса.
   * @return true - карта успешно загружена, false - ошибка открытия или формата
   * файла.
   */
  bool load_from_file(const std::string& path, const std::string& tileset_tag,
                      int tile_size = 32);

  /**
   * @brief Отрисовывает всё игровое поле.
   *
   * @param[in] renderer Отрисовщик, в который производится отрисовка.
   * @pre load_from_file() должен быть успешно выполнен.
   */
  void render(SDL_Renderer* renderer);

  /**
   * @brief Возвращает тип тайла по координатам сетки.
   *
   * При запросе за пределами карты возвращает Wall.
   *
   * @param[in] col Индекс столбца.
   * @param[in] row Индекс строки.
   * @return Тип тайла в ячейке или Wall, если координаты вне карты.
   */
  TileType get_tile(int col, int row) const;

  /**
   * @brief Устанавливает тип тайла и обновляет счётчик точек.
   *
   * Увеличивает или уменьшает счётчик dots_remaining_. Запись за пределами
   * карты игнорируется.
   *
   * @param[in] col  Индекс столбца.
   * @param[in] row  Индекс строки.
   * @param[in] type Новый тип тайла.
   */
  void set_tile(int col, int row, TileType type);

  /**
   * @brief Проверяет может ли пройти в ячейку игрок.
   *
   * Нельзя пройти через Wall и GhostDoor.
   *
   * @param[in] col Индекс столбца.
   * @param[in] row Индекс строки.
   * @return true - ячейка проходима, false - нет.
   */
  bool is_walkable_by_player(int col, int row) const;

  /**
   * @brief Проверяет может ли пройти в ячейку призрак.
   *
   * Нельзя пройти через Wall.
   *
   * @param[in] col Индекс столбца.
   * @param[in] row Индекс строки.
   * @return true - ячейка проходима, false - нет.
   */
  bool is_walkable_by_ghost(int col, int row) const;

  /// @brief Возвращает число столбцов карты.
  int get_cols() const { return cols_; }

  /// @brief Возвращает число строк карты.
  int get_rows() const { return rows_; }

  /// @brief Возвращает размер одного тайла в пикселях.
  int get_tile_size() const { return tile_size_; }

  /**
   * @brief Преобразует координату в пикселях в индекс тайла.
   *
   * @param[in] x Пиксельная координата.
   * @return Индекс соответствующей ячейки сетки.
   */
  int pixel_to_tile(float x) const { return x / tile_size_; }

  /**
   * @brief Преобразует индекс тайла в координату в пикселях его левого верхнего
   * угла.
   *
   * @param[in] col Индекс тайла.
   * @return Координата левого верхнего угла ячейки в пикселях.
   */
  float tile_to_pixel(int col) const { return col * tile_size_; }

  /**
   * @brief Преобразует индекс тайла в координату его центра в пикселях.
   *
   * @param[in] col Индекс тайла.
   * @return Координата центра ячейки в пикселях.
   */
  float tile_to_center_pixel(int col) const { return (col + 0.5) * tile_size_; }

  /// @brief Возвращает число оставшихся на карте точек (Dot и Energizer).
  int get_dots_remaining() const { return dots_remaining_; }

  /**
   * @brief Проверяет, собраны ли все точки на карте.
   * @return true - уровень пройден, false - точки ещё остались.
   */
  bool is_cleared() const { return dots_remaining_ == 0; }

  /// @brief Возвращает стартовую позицию игрока.
  GridPos get_pacman_spawn() const { return pacman_spawn_; }

  /// @brief Возвращает список стартовых позиций всех призраков.
  const std::vector<GhostSpawn>& get_ghost_spawns() const {
    return ghost_spawns_;
  }

  /**
   * @brief Нормализует координаты с учётом туннеля.
   *
   * Если координата выходит за край карты - возвращает индекс ячейки
   * на противоположном крае.
   *
   * @param[in] col Индекс столбца.
   * @param[in] row Индекс строки.
   * @return Нормализованные координаты.
   */
  GridPos normalize_position(int col, int row) const;

  /**
   * @brief Обрабатывает логику поедание точек.
   *
   * При поедание любой точки текущая ячейка заменяется на пустую.
   * Если съедена большая то, у Pac-man'a включается режим
   * энерджайзера на некоторое время.
   *
   * @param[in] col Индекс столбца.
   * @param[in] row Индекс строки.
   * @param player  Ссылка на экземпляр Player.
   */
  void eating_dot(int col, int row, Player& player);

  /// @brief Открывает дверь дома призраков для игрока (буст Key).
  void open_ghost_door();

 private:
  std::vector<std::vector<TileType>> data_;    ///< Двумерная сетка тайлов.
  int cols_ = 0;                               ///< Столбцы.
  int rows_ = 0;                               ///< Строки.
  int tile_size_ = 0;                          ///< Пикселей на тайл.
  int dots_remaining_ = 0;                     ///< Осталось точек.
  GridPos pacman_spawn_ = {};                  ///< Коордиаты спауна игрока.
  std::vector<GhostSpawn> ghost_spawns_ = {};  ///< Координаты спауна призраков.
  std::string tileset_tag_;                    ///< Тег текстуры тайлсета.
  bool is_door_open_ = false;                  ///< Открыта ли дверь игроку.

  /// @brief Выбирает номер спрайта для одного угла тайла стены.
  int compute_corner_frame(int col, int row, Corner corner) const;

  /// @brief Возвращает режим поворота текстуры, чтобы левый верхний спрайт стал
  /// спрайтом угла corner.
  SDL_FlipMode corner_flip(Corner corner) const;

  /// @brief Записывает в dx и dy смещение углового спрайта
  /// от левого верхнего угла тайла.
  void corner_offset(Corner corner, float& dx, float& dy) const;

  /// @brief Отрисовывает один угол тайла стены
  void draw_wall_corner(int col, int row, Corner corner,
                        SDL_Renderer* renderer) const;
};