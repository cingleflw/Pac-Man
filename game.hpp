/**
 * @file game.hpp
 * @brief Главный игровой класс - управление окном, отрисовщиком и игровым
 * циклом.
 *
 * Содержит класс Game, который инкапсулирует всю логику инициализации SDL,
 * обработки событий, обновления состояния и отрисовки кадров.
 */

#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <string>
#include <vector>

#include "boost_manager.hpp"
#include "game_map.hpp"
#include "ghost.hpp"
#include "ghost_controller.hpp"
#include "player.hpp"
#include "scare_trigger.hpp"

constexpr int START_LIVES = 3;  ///< Жизней в начале игры.

/**
 * @brief Режим игры.
 *
 * Скример - временный режим, перехватывающий отрисовку и ввод.
 * Игровой режим вынесен в автомат состояний отдельно от флага жизни цикла
 * running_.
 */
enum class GameState {
  Playing,  ///< Активная игра.
  Scare,    ///< Идёт скример.
  Dead,     ///< Игрок пойман.
  Win,      ///< Уровень пройден.
  Paused,   ///< Пауза.
  Ready,    ///< Готов к Playing.
};

/**
 * @brief Настройка доступности (скримеров).
 *
 * Включены On или выключены Off.
 */
enum class ScareSetting {
  On,   ///< Скримеры включены.
  Off,  ///< Скримеры отключены.
};

/**
 * @brief Основной класс игры, управляющий её жизненным циклом.
 *
 * Реализует ScareTrigger и GhostController. Бусты запрашивают скример через
 * метод request_scare(), не зная, как тот рисуется. Класс Game отвечает за
 * создание окна SDL, управление отрисовщиком, обработку пользовательского ввода
 * и выполнение игрового цикла. Типичный порядок использования: init() ->
 * start_game() -> цикл (handle_events -> update -> render) -> clean().
 *
 * @note Экземпляр Game владеет окном и отрисовщиком. Метод clean()
 * должен быть вызван перед завершением программы.
 */
class Game : public ScareTrigger, public GhostController {
 public:
  Game() : boost_manager_(player_, map_) {}

  /// Деструктор. @warning Не вызывает clean() автоматически.
  ~Game() {}

  /**
   * @brief Инициализирует SDL - создаёт окно и отрисовщик.
   *
   * Создаёт окно с заданными параметрами и привязывает к нему отрисовщик.
   * При успехе окно готово к отрисовке, при ошибке выводит сообщение в stderr.
   *
   * @param[in] title  Заголовок окна.
   * @param[in] w      Ширина окна в пикселях.
   * @param[in] h      Высота окна в пикселях.
   * @param[in] flags  Флаги окна SDL, комбинируемые через побитовое ИЛИ.
   * @pre SDL слинкован.
   * @post При успехе window_ и renderer_ указывают на окно и отрисовщик.
   * @return true - инициализация прошла успешно, false - произошла ошибка.
   */
  bool init(std::string title, int w, int h, int flags);

  /**
   * @brief Отрисовывает текущий кадр на экране.
   *
   * Очищает буфер отрисовщика и выводит текущее состояние.
   *
   * @pre Отрисовщик должен быть успешно создан через init().
   */
  void render();

  /// @brief Обновляет игровую логику на один тик. Вызывается каждый кадр.
  void update();

  /**
   * @brief Обрабатывает пользовательский ввод.
   *
   * Извлекает события из очереди SDL. Обрабатывает:
   * - SDL_EVENT_QUIT - закрытие окна.
   * - SDL_EVENT_WINDOW_RESIZED - изменение размера окна.
   */
  void handle_events();

  /**
   * @brief Освобождает все ресурсы и завершает работу SDL.
   *
   * Уничтожает отрисовщик, окно и вызывает SDL_Quit().
   *
   * @post window_ и renderer_ становятся невалидными указателями.
   */
  void clean();

  /// @brief Запускает игровой цикл, устанавливая флаг running_.
  void start_game() { running_ = true; }

  /// @brief Останавливает игровой цикл, сбрасывая флаг running_.
  void stop_game() { running_ = false; }

  /**
   * @brief Проверяет, выполняется ли игровой цикл.
   * @return true - игра запущена, false - игра остановлена.
   */
  bool is_running() { return running_; }

  /**
   * @brief Реализация ScareTrigger: запрос скримера от любой системы.
   *
   * Запрос учитывает кулдаун между скримерами, щит игрока (поглощает испуг)
   * и настройку интенсивности. После показа игра возвращается в состояние
   * Playing.
   *
   * @param[in] texture_tag Тег текстуры скримера.
   * @param[in] duration Длительность показа в миллисекундах.
   */
  void request_scare(const std::string& texture_tag, Uint64 duration) override;

  // --- Реализация GhostController: передаём эффекты всем призракам ---

  /// @brief Останавливает всех призраков на duration мс (Bell).
  void freeze(Uint64 duration_ms) override {
    for (Ghost* g : ghosts_) g->freeze(duration_ms);
  }

  /// @brief Ослепляет призраков: случайное движение, но они опасны (Potion).
  void confuse(Uint64 duration_ms) override {
    for (Ghost* g : ghosts_) g->confuse(duration_ms);
  }

  /// @brief Съедобный режим. @note Длительностью испуга в текущей версии
  /// управляет синхронизация с энерджайзером в update_ghost_mode(); ни один
  /// буст этот метод не вызывает: он реализован лишь ради интерфейса.
  void frighten(Uint64 duration_ms) override {
    (void)duration_ms;
    for (Ghost* g : ghosts_) g->frighten();
  }

  /// @brief Мгновенно отправляет всех домой «глазами» (Wand).
  void banish_all() override {
    for (Ghost* g : ghosts_) g->get_eaten();
  }

  /// @brief Временно ускоряет призраков (неудачный исход Surprise box).
  void enrage(Uint64 duration_ms) override {
    for (Ghost* g : ghosts_) g->enrage(duration_ms);
  }

  void lose_life();

 private:
  /// @brief Создаёт (или пересоздаёт) призраков из спаунов карты.
  void spawn_ghosts(int tile_size);

  /// @brief Двигает таймер фаз Scatter/Chase и синхронизирует испуг.
  void update_ghost_mode();

  /// @brief Проверяет столкновение Pac-Man с призраками.
  void check_ghost_collisions();

  /// @brief Рисует полноэкранный скример поверх игры.
  void render_scare();

  /// Флаг: запущена ли игра.
  bool running_ = false;

  /// Текущий режим игры.
  GameState state_ = GameState::Playing;

  /// Указатель на окно.
  SDL_Window* window_ = nullptr;

  /// Указатель на отрисовщик.
  SDL_Renderer* renderer_ = nullptr;

  /// Экземпляр класса GameMap.
  GameMap map_;
  /// Экземпляр класса Player.
  Player player_;

  /// Менеджер бустов.
  BoostManager boost_manager_;

  // Состояние скримера
  ScareSetting scare_setting_ = ScareSetting::On;  ///< Настройка доступности.
  std::string scare_tag_;        ///< Текстура текущего скримера.
  Uint64 scare_start_time_ = 0;  ///< Когда начался скример.
  Uint64 scare_duration_ = 0;    ///< Сколько длится скример.
  Uint64 last_scare_end_time_ =
      0;  ///< Когда закончился прошлый скример (для кулдауна).

  // --- Призраки ---
  Blinky blinky_;  ///< Красный.
  Pinky pinky_;    ///< Розовый.
  Inky inky_;      ///< Голубой.
  Clyde clyde_;    ///< Оранжевый.

  /// Указатели на всех призраков - для единообразного обхода в циклах.
  std::vector<Ghost*> ghosts_;

  // --- Таймер фаз Scatter/Chase ---
  int phase_index_ = 0;            ///< Индекс текущей фазы в расписании.
  Uint64 phase_start_ms_ = 0;      ///< Время начала текущей фазы.
  Uint64 fright_pause_start_ = 0;  ///< Момент «заморозки» таймера на испуг.
  bool was_energized_ = false;  ///< Был ли активен энерджайзер в прошлом кадре.

  int lives_ = START_LIVES;
  Uint64 ready_until_ = 0;
};
