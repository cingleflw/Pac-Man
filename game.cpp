/**
 * @file game.cpp
 * @brief Реализация методов класса Game.
 */

#include "game.hpp"

#include <iostream>

#include "input_handler.hpp"
#include "texture_manager.hpp"

constexpr Uint64 SCARE_COOLDOWN =
    4000;  ///< Минимум между скримерами в миллисекундах.
constexpr Uint64 FLASH_DURATION =
    160;  ///< Длительность вспышки в начале скримера в миллисекундах.
constexpr int SHAKE_PX = 8;  ///< Амплитуда тряски экрана.

// SDL требует, чтобы main имел именно такую сигнатуру.
int main(int argc, char** argv) {
  Game g;

  if (g.init("TEST WINDOW", 896, 992,
             SDL_WINDOW_RESIZABLE | SDL_WINDOW_MINIMIZED)) {
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

  // Готовим бусты и подключаем реализацию интерфейсов для скримеров и эффектов
  // призраков.
  boost_manager_.init("boosts");
  boost_manager_.set_scare_trigger(this);
  // boost_manager_.set_ghost_controller(this);

  return true;
}

void Game::render() {
  // Цвет заливки - серый.
  SDL_SetRenderDrawColor(renderer_, 42, 42, 42, 255);
  SDL_RenderClear(renderer_);  // Очищаем всё, что было отрисовано ранее.

  map_.render(renderer_);
  boost_manager_.render(renderer_);
  player_.render(renderer_);

  if (state_ == GameState::Scare) {
    render_scare();
  }

  SDL_RenderPresent(renderer_);  // Выводим на экран текущее состояние.
}

void Game::render_scare() {
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
        state_ = GameState::Playing;
      }
      break;
  }
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
          if (event.key.scancode == SDL_SCANCODE_F) {
            // Циклически переключаем интенсивность скримеров.
            scare_setting_ = static_cast<ScareSetting>(
                (static_cast<int>(scare_setting_) + 1) % 2);
            break;
          }
        }

        // Завершающие экраны.
        if (state_ == GameState::Win || state_ == GameState::Dead) {
          stop_game();
          // break;
        }

        // Движение принимаем только в состоянии активной игры.
        if (state_ == GameState::Playing) {
          InputHandler::Instance(player_, map_).handle(event);
        }
        break;
    }
  }
}

void Game::clean() {
  // Порядок, обратный созданию.
  std::cout << "exit" << std::endl;
  player_.clean();
  TextureManager::instance().clean();  // Текстуры до renderer'а.
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}
