/**
 * @file game.cpp
 * @brief Реализация методов класса Game.
 */

#include "game.hpp"

#include <iostream>

#include "texture_manager.hpp"

// SDL требует, чтобы main имел именно такую сигнатуру.
int main(int argv, char** argc) {
  Game g;

  if (g.init("TEST WINDOW", 672, 744,
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

  if (window_ != 0) {
    std::cout << "window created" << std::endl;

    // NULL вместо имени драйвера - SDL сам выберет подходящий.
    renderer_ = SDL_CreateRenderer(window_, NULL);

    if (renderer_ != 0) {
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

      if (TextureManager::instance().load("assets/test.png", "main_char",
                                          renderer_)) {
        std::cout << "main texture created" << std::endl;
      } else {
        std::cerr << "texture error" << std::endl;
        return false;
      }
    } else {
      std::cerr << "renderer error" << std::endl;
      return false;
    }
  } else {
    std::cerr << "window error" << std::endl;
    return false;
  }

  return true;
}

void Game::render() {
  // Цвет заливки - красный.
  SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
  SDL_RenderClear(renderer_);  // Очищаем всё, что было отрисовано ранее.

  map_.render(renderer_);

  // Рисуем текстуру персонажа: позиция (100, 100), размер 200*200.
  TextureManager::instance().draw("main_char", 100, 100, 200, 200, renderer_);

  SDL_RenderPresent(renderer_);  // Выводим на экран текущее состояние.
}

void Game::update() {
  // Циклически перебираем кадры от 0 до 5.
  current_frame_ = int((SDL_GetTicks() / 100) % 6);
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
    }
  }
}

void Game::clean() {
  // Порядок, обратный созданию.
  std::cout << "exit" << std::endl;
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}
