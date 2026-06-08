/**
 * @file game.cpp
 * @brief Реализация методов класса Game.
 */

#include "game.hpp"

#include <iostream>

#include "input_handler.hpp"
#include "texture_manager.hpp"

constexpr int HUD_HEIGHT = 64;  ///< Высота верхней панели в пикселях.
constexpr Uint64 SCARE_COOLDOWN =
    4000;  ///< Минимум между скримерами в миллисекундах.

/// @brief Одна фаза глобального таймера призраков.
struct Phase {
  GhostMode mode;
  Uint64 duration_ms;
};

/// @brief Расписание Scatter/Chase. Последняя фаза - Chase навсегда.
const Phase kSchedule[] = {
    {GhostMode::Scatter, 7000}, {GhostMode::Chase, 20000},
    {GhostMode::Scatter, 7000}, {GhostMode::Chase, 20000},
    {GhostMode::Scatter, 5000}, {GhostMode::Chase, 20000},
    {GhostMode::Scatter, 5000}, {GhostMode::Chase, 0},
};
const int kScheduleLen =
    static_cast<int>(sizeof(kSchedule) / sizeof(kSchedule[0]));

// SDL требует, чтобы main имел именно такую сигнатуру.
int main(int argc, char** argv) {
  Game g;

  if (g.init("PAC-MAN", 896, 992 + HUD_HEIGHT, SDL_WINDOW_RESIZABLE)) {
    g.start_game();
  } else {
    return 1;  // Инициализация не удалась - выход с ошибкой.
  }

  // Главный вечный игровой цикл. Выполняется каждый тик (кадр). Завершается,
  // когда running_ станет false.
  while (g.is_running()) {
    g.handle_events();  // Слушаем ввод.
    g.update();         // Обновляем состояние.
    g.render();         // Отрисовываем кадр.
  }

  g.clean();
  return 0;
}

bool Game::init(std::string title, int w, int h, int flags) {
  // Создаёт окно. Вернёт nullptr, если окно создать не удалось.
  window_ = SDL_CreateWindow(title.c_str(), w, h, flags);

  if (window_ == 0) {
    std::cerr << "window error" << std::endl;
    return false;
  }
  std::cout << "window created" << std::endl;

  // NULL вместо имени драйвера - SDL сам выберет подходящий.
  renderer_ = SDL_CreateRenderer(window_, NULL);

  if (renderer_ == 0) {
    std::cerr << "renderer error" << std::endl;
    return false;
  }
  std::cout << "renderer created" << std::endl;

  if (TextureManager::instance().load("assets/levels/tileset_1.png",
                                      "tileset_1", renderer_)) {
    std::cout << "tileset texture created" << std::endl;
  } else {
    std::cerr << "texture error" << std::endl;
    return false;
  }

  if (!map_.load_from_file("assets/levels/level_1.txt", "tileset_1")) {
    return false;
  }
  auto spawn = map_.get_pacman_spawn();

  if (TextureManager::instance().load("assets/character.png", "main_char",
                                      renderer_)) {
    std::cout << "main texture created" << std::endl;
  } else {
    std::cerr << "texture error" << std::endl;
    return false;
  }
  float spawn_x = map_.tile_to_pixel(spawn.col);
  float spawn_y = map_.tile_to_pixel(spawn.row);
  player_ = Player(spawn_x, spawn_y, 32, 32, "main_char");

  // Подгрузка текстур бустов и скримеров.
  if (!TextureManager::instance().load("assets/boosts.png", "boosts",
                                       renderer_)) {
    std::cerr << "boosts texture warning" << std::endl;
  }
  for (int i = 1; i <= 4; ++i) {
    const std::string path = "assets/scare_" + std::to_string(i) + ".png";
    const std::string tag = "scare_" + std::to_string(i);
    if (!TextureManager::instance().load(path, tag, renderer_)) {
      std::cerr << tag << " texture warning" << std::endl;
    }
  }

  if (!TextureManager::instance().load("assets/life.png", "life", renderer_)) {
    std::cerr << "life texture warning" << std::endl;
  }

  if (!TextureManager::instance().load("assets/digits.png", "digits",
                                       renderer_))
    std::cerr << "digits texture warning" << std::endl;
  if (!TextureManager::instance().load("assets/ready.png", "ready_text",
                                       renderer_))
    std::cerr << "ready_text warning" << std::endl;
  if (!TextureManager::instance().load("assets/gameover.png", "gameover_text",
                                       renderer_))
    std::cerr << "gameover_text warning" << std::endl;
  if (!TextureManager::instance().load("assets/win.png", "win_text", renderer_))
    std::cerr << "win_text warning" << std::endl;

  if (!TextureManager::instance().load("assets/scare_label.png", "scare_label",
                                       renderer_))
    std::cerr << "scare_label texture warning" << std::endl;
  if (!TextureManager::instance().load("assets/check_on.png", "check_on",
                                       renderer_))
    std::cerr << "check_on texture warning" << std::endl;
  if (!TextureManager::instance().load("assets/check_off.png", "check_off",
                                       renderer_))
    std::cerr << "check_off texture warning" << std::endl;

  // Готовим бусты и подключаем реализацию интерфейсов для скримеров и эффектов
  // призраков.
  boost_manager_.init("boosts");
  boost_manager_.set_scare_trigger(this);
  boost_manager_.set_ghost_controller(this);

  // Текстуры призраков. Их отсутствие НЕ фатально: игра запустится,
  // призраки просто не нарисуются, пока не подложишь ассеты.
  auto load_ghost = [&](const char* path, const char* tag) {
    if (!TextureManager::instance().load(path, tag, renderer_)) {
      std::cerr << "ghost texture missing: " << tag << std::endl;
    }
  };
  load_ghost("assets/blinky.png", "blinky");
  load_ghost("assets/pinky.png", "pinky");
  load_ghost("assets/inky.png", "inky");
  load_ghost("assets/clyde.png", "clyde");
  load_ghost("assets/ghost_frightened.png", "ghost_frightened");
  load_ghost("assets/ghost_eyes.png", "ghost_eyes");

  // Создаём призраков и заводим фазовый таймер.
  spawn_ghosts(map_.get_tile_size());
  ghosts_ = {&blinky_, &pinky_, &inky_, &clyde_};
  phase_start_ms_ = SDL_GetTicks();
  for (Ghost* g : ghosts_) g->set_mode(kSchedule[0].mode);

  return true;
}

void Game::spawn_ghosts(int tile_size) {
  // Каждому id соответствует свой подкласс и свой тег текстуры.
  for (const GhostSpawn& s : map_.get_ghost_spawns()) {
    float gx = map_.tile_to_pixel(s.position.col);
    float gy = map_.tile_to_pixel(s.position.row);
    float size = static_cast<float>(tile_size);
    switch (s.id) {
      case GhostId::Blinky:
        blinky_ = Blinky(gx, gy, size, size, "blinky", tile_size);
        break;
      case GhostId::Pinky:
        pinky_ = Pinky(gx, gy, size, size, "pinky", tile_size);
        break;
      case GhostId::Inky:
        inky_ = Inky(gx, gy, size, size, "inky", tile_size);
        break;
      case GhostId::Clyde:
        clyde_ = Clyde(gx, gy, size, size, "clyde", tile_size);
        break;
    }
  }
}

void Game::render() {
  // Цвет заливки - серый.
  SDL_SetRenderDrawColor(renderer_, 42, 42, 42, 255);
  SDL_RenderClear(renderer_);  // Очищаем всё, что было отрисовано ранее.

  // Лабиринт и сущности рисуются в окне, сдвинутом вниз на размер HUD полосы.
  // Отриовщики карты, игрока, призраков ничего об этом не знают.
  const int maze_w = map_.get_cols() * map_.get_tile_size();
  const int maze_h = map_.get_rows() * map_.get_tile_size();

  // Прямоуголник maze_w на maze_h от (0; HUD_HEIGHT).
  SDL_Rect maze_vp = {0, HUD_HEIGHT, maze_w, maze_h};
  SDL_SetRenderViewport(renderer_, &maze_vp);

  map_.render(renderer_);
  boost_manager_.render(renderer_);
  player_.render(renderer_);

  for (Ghost* g : ghosts_) g->render(renderer_);

  if (state_ == GameState::Ready) {
    draw_centered("ready_text", maze_w, maze_h);
  }
  if (state_ == GameState::Win) {
    draw_centered("win_text", maze_w, maze_h);
  }
  if (state_ == GameState::Dead) {
    draw_centered("gameover_text", maze_w, maze_h);
  }

  SDL_SetRenderViewport(renderer_, nullptr);  // Обратно на всё окно.
  render_hud();                               // Рисуется уже без сдвига.

  if (state_ == GameState::Scare) {
    render_scare();
  }

  SDL_RenderPresent(renderer_);  // Выводим на экран текущее состояние.
}

void Game::render_scare() {
  constexpr Uint64 FLASH_DURATION =
      160;  ///< Длительность вспышки в начале скримера в миллисекундах.
  constexpr int SHAKE_PX = 8;  ///< Амплитуда тряски экрана.

  int window_w = 0, window_h = 0;
  SDL_GetWindowSize(window_, &window_w, &window_h);
  const float w = static_cast<float>(window_w);
  const float h = static_cast<float>(window_h);

  const Uint64 time_since_start = SDL_GetTicks() - scare_start_time_;

  // Вспышка в первые миллисекунды.
  if (time_since_start < FLASH_DURATION) {
    SDL_FRect full = {0, 0, w, h};
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer_, &full);
    return;
  }

  // Тряска.
  const float x =
      static_cast<float>((SDL_GetTicks() % (2 * SHAKE_PX)) - SHAKE_PX);
  const float y =
      static_cast<float>((SDL_GetTicks() / 3 % (2 * SHAKE_PX)) - SHAKE_PX);

  SDL_FRect full = {0, 0, w, h};
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_RenderFillRect(renderer_, &full);

  TextureManager::instance().draw(scare_tag_, x, y, w, h, renderer_);
}

void Game::update() {
  switch (state_) {
    case GameState::Playing:
      player_.update(map_);
      boost_manager_.update();

      update_ghost_mode();

      // Inky нужна позиция Blinky, поэтому передаём его всем явным
      // аргументом.
      for (Ghost* g : ghosts_) g->update(map_, player_, blinky_);

      check_ghost_collisions();

      if (map_.is_cleared()) {
        state_ = GameState::Win;
      }
      break;

    case GameState::Scare:
      if (SDL_GetTicks() - scare_start_time_ >= scare_duration_) {
        last_scare_end_time_ = SDL_GetTicks();
        state_ = GameState::Playing;
      }
      break;

    case GameState::Win:
    case GameState::Dead:
    case GameState::Paused:
      // Ждём действия игрока в handle_events().
      break;

    case GameState::Ready:
      if (SDL_GetTicks() >= ready_until_) {
        phase_start_ms_ = SDL_GetTicks();  // не теряем ~2с первой фазы
        state_ = GameState::Playing;
      }
      break;
  }
}

void Game::update_ghost_mode() {
  const Uint64 now = SDL_GetTicks();
  const bool energized = player_.is_energized();

  // Фронт включения энерджайзера: пугаем всех и замораживаем фазовый таймер.
  if (energized && !was_energized_) {
    for (Ghost* g : ghosts_) g->frighten();
    fright_pause_start_ = now;
  }
  // Фронт окончания: сдвигаем старт фазы на длительность испуга и
  // возвращаем призраков в текущую фазу расписания.
  else if (!energized && was_energized_) {
    phase_start_ms_ += (now - fright_pause_start_);
    for (Ghost* g : ghosts_) g->set_mode(kSchedule[phase_index_].mode);
  }
  was_energized_ = energized;

  // Во время испуга расписание Scatter/Chase «на паузе».
  if (energized) return;

  // Переход к следующей фазе по таймеру (последняя фаза длится вечно).
  if (phase_index_ + 1 < kScheduleLen &&
      now - phase_start_ms_ >= kSchedule[phase_index_].duration_ms) {
    phase_index_++;
    phase_start_ms_ = now;
    for (Ghost* g : ghosts_) g->set_mode(kSchedule[phase_index_].mode);
  }
}

constexpr int GHOST_EAT_SCORE = 200;

void Game::check_ghost_collisions() {
  const float px = player_.get_x(), py = player_.get_y();
  const float pw = player_.get_w(), ph = player_.get_h();
  constexpr float PAD = 8.0f;  // чтобы не ловить касание уголками
  for (Ghost* g : ghosts_) {
    if (g->get_mode() == GhostMode::Eaten) continue;
    const bool hit =
        px < g->get_x() + g->get_w() - PAD && px + pw - PAD > g->get_x() &&
        py < g->get_y() + g->get_h() - PAD && py + ph - PAD > g->get_y();
    if (!hit) continue;

    if (g->is_frightened()) {
      player_.add_score(GHOST_EAT_SCORE);
      g->get_eaten();
    } else if (player_.consume_shield()) {
      g->get_eaten();
    } else {
      lose_life();
      return;
    }
  }
}

void Game::respawn_entities() {
  GridPos player_spawn = map_.get_pacman_spawn();
  player_.reset_to_spawn(map_.tile_to_pixel(player_spawn.col),
                         map_.tile_to_pixel(player_spawn.row));
  spawn_ghosts(map_.get_tile_size());
  for (Ghost* g : ghosts_) {
    g->set_mode(kSchedule[0].mode);
  }
  phase_index_ = 0;
  phase_start_ms_ = SDL_GetTicks();
  was_energized_ = false;
}

void Game::start_new_game() {
  map_.load_from_file("assets/levels/level_1.txt", "tileset_1");
  boost_manager_.init("boosts");
  lives_ = START_LIVES;
  player_.add_score(-player_.get_score());
  respawn_entities();
  state_ = GameState::Ready;
  ready_until_ = SDL_GetTicks() + READY_MS;
}

void Game::request_scare(const std::string& texture_tag, Uint64 duration) {
  const Uint64 now = SDL_GetTicks();

  if (state_ != GameState::Playing) {
    return;  // Чтобы не наложить поверх баннеров Win/Dead/Ready.
  }
  if (scare_setting_ == ScareSetting::Off) {
    return;
  }
  if (now - last_scare_end_time_ < SCARE_COOLDOWN) {
    return;  // Кулдаун.
  }

  scare_tag_ = texture_tag;
  scare_start_time_ = now;
  scare_duration_ = duration;
  state_ = GameState::Scare;
}

void Game::handle_events() {
  SDL_Event event;  // Структура для хранения текущего события.

  // SDL_PollEvent записывает событие по ссылке. Возвращает true, если в очереди
  // было событие.
  if (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        stop_game();  // Закрытие окна.
        break;

      case SDL_EVENT_WINDOW_RESIZED:
        std::cout << "resized" << std::endl;
        // Срабатывает в момент завершения ресайза окна. Можно пересчитать
        // размеры игровых элементов.
        break;

      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
        // Глобальные клавиши доступны в любом режиме.
        if (event.type == SDL_EVENT_KEY_DOWN) {
          if (event.key.scancode == SDL_SCANCODE_P) {
            if (state_ == GameState::Playing) {
              state_ = GameState::Paused;
            } else if (state_ == GameState::Paused) {
              state_ = GameState::Playing;
            }
            break;
          }
          if (event.key.scancode == SDL_SCANCODE_F) {
            // Циклически переключаем настройку скримеров.
            scare_setting_ = static_cast<ScareSetting>(
                (static_cast<int>(scare_setting_) + 1) % 2);
            break;
          }
        }

        // Завершающие экраны.
        if (state_ == GameState::Win || state_ == GameState::Dead) {
          if (event.key.scancode == SDL_SCANCODE_RETURN) {
            start_new_game();
          } else if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
            stop_game();
          }
          break;
        }

        // Движение принимаем только в состоянии активной игры.
        if (state_ == GameState::Playing) {
          InputHandler::Instance(player_, map_).handle(event);
        }
        break;
    }
  }
}

void Game::lose_life() {
  if (--lives_ <= 0) {
    state_ = GameState::Dead;
    return;
  }
  respawn_entities();
  state_ = GameState::Ready;
  ready_until_ = SDL_GetTicks() + READY_MS;
}

void Game::render_number(int value, float x, float y, int w, int h) {
  const std::string s = std::to_string(value);
  for (size_t i = 0; i < s.size(); i++) {
    TextureManager::instance().draw_frame("digits", x + i * w, y, w, h, 1,
                                          s[i] - '0', renderer_);
  }
}

void Game::render_hud() {
  constexpr int HUD_ICON_SIZE = 32;  ///< Сторона иконки (цифры/жизни).
  constexpr int LIFE_ICON = 36;      ///< Расстояние между иконками жизней.

  const float y = (HUD_HEIGHT - HUD_ICON_SIZE) / 2.0;  // Центр.

  render_number(player_.get_score(), 16, y, HUD_ICON_SIZE, HUD_ICON_SIZE);

  const int window_w = map_.get_cols() * map_.get_tile_size();

  // Индикатор настройки скримеров по центру шапки.
  constexpr int LABEL_W = 128, LABEL_H = 32;
  constexpr int MARK = 32;  // Сторона галочки/крестика.
  constexpr int GAP = 8;    // Расстояние между надписью и значком.

  const float scare_group_w = LABEL_W + GAP + MARK;
  const float scare_x = (window_w - scare_group_w) / 2.0;
  const char* mark = "";

  if (scare_setting_ == ScareSetting::Off) {
    mark = "check_off";
  } else {
    mark = "check_on";
  }

  TextureManager::instance().draw("scare_label", scare_x, y, LABEL_W, LABEL_H,
                                  renderer_);
  TextureManager::instance().draw(mark, scare_x + LABEL_W + GAP, y, MARK, MARK,
                                  renderer_);

  // Жизни справа.
  for (int i = 0; i < lives_; i++) {
    const float life_x = window_w - (i + 1) * LIFE_ICON - 8;
    TextureManager::instance().draw("life", life_x, y, HUD_ICON_SIZE,
                                    HUD_ICON_SIZE, renderer_);
  }
}

void Game::draw_centered(const std::string& tag, int w, int h) {
  constexpr float banner_w = 512, banner_h = 128;
  TextureManager::instance().draw(tag, (w - banner_w) / 2.0f,
                                  (h - banner_h) / 2.0f, banner_w, banner_h,
                                  renderer_);
}

void Game::clean() {
  // Порядок, обратный созданию.
  std::cout << "exit" << std::endl;
  for (Ghost* g : ghosts_) g->clean();
  player_.clean();
  TextureManager::instance().clean();  // Текстуры до renderer'а.
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}
