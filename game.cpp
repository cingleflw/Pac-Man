/**
 * @file game.cpp
 * @brief Реализация методов класса Game.
 */

#include "game.hpp"

#include <iostream>

// SDL требует, чтобы main имел именно такую сигнатуру (int, char**).
int main(int argv, char** argc) {
  Game g;

  if (g.init("TEST WINDOW", 640, 480,
             SDL_WINDOW_RESIZABLE | SDL_WINDOW_MINIMIZED)) {
    g.startGame();
  } else {
    return 1;  // Инициализация не удалась – выход с ошибкой
  }

  // Главный вечный игровой цикл. Выполняется каждый тик (кадр). Завершается,
  // когда running_ станет false.
  while (g.isRunning()) {
    g.handleEvents();  // 1. Слушаем ввод
    g.update();        // 2. Обновляем состояние
    g.render();        // 3. Отрисовываем кадр
  }

  g.clean();
  return 0;
}

bool Game::init(std::string title, int w, int h, int flags) {
  // Создаёт окно. Вернёт nullptr, если окно создать не удалось.
  window_ = SDL_CreateWindow(title.c_str(), w, h, flags);

  if (window_ != 0) {
    std::cout << "window created" << std::endl;

    // NULL вместо имени драйвера – SDL сам выберет подходящий.
    renderer_ = SDL_CreateRenderer(window_, NULL);

    if (renderer_ != 0) {
      std::cout << "renderer created" << std::endl;
      // Цвет заливки - красный.
      SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
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
  SDL_RenderClear(renderer_);    // Очищаем всё, что было отрисовано ранее
  SDL_RenderPresent(renderer_);  // Выводим на экран текущее состояние
}

void Game::update() {
  // Пока пуст - нет игровых объектов для обновления. Сюда будет добавляться
  // логика: перемещение персонажей, проверка столкновений, обновление счёта и
  // т.д.
}

void Game::handleEvents() {
  SDL_Event event;  // Структура для хранения текущего события

  // SDL_PollEvent записывает событие по ссылке. Возвращает true, если в очереди
  // было событие.
  if (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        stopGame();  // Закрытие окна
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
  // Порядок, обратный созданию
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}
