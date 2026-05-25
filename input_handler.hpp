/**
* @file input_handler.hpp
* @brief Singleton для обработки пользовательского ввода.
*/

#pragma once

#include <SDL3/SDL.h>

#include "game_map.hpp"
#include "player.hpp"

/**
* @brief Обработчик ввода.
* 
* Осуществляет обработку событий SDL, взаимодействуя с
* экземплярами Player и GameMap.
* 
* @note Singleton: копирование и перемещение запрещены - экземпляр
* единственный.
*/

class InputHandler {
public:
    /**
    * @brief Выдаёт ссылку на единственный экземпляр InputHandler.
    *
    * При первом вызове создаёт статический локальный объект.
    * 
    * @param p Ссылка на экземпляр Player.
    * @param m Ссылка на экземпляр GameMap.
    *
    * @return Ссылка на экземпляр InputHandler.
    */
    static InputHandler* Instance(Player& p, GameMap& m){
        if (instance_ == 0){
            instance_ = new InputHandler(p, m);
        }
        return instance_;
    }

    /**
    * @brief Обрабатывает события SDL.
    * 
    * @param event Рассматриваемое событие.
    */
    void handle(SDL_Event event);

private:
    /// @brief Приватный конструктор - экземпляр создаётся только через
    /// Instance(Player& p, GameMap& m).
    InputHandler(Player& p, GameMap& m) : player_(p), map_(m) {};

    /// @brief Деструктор.
    ~InputHandler();

    /// @brief Статический указатель на экземпляр InputHandler.
    static InputHandler* instance_;

    /// @brief Ссылка на экземпляр Player.
    Player& player_;

    /// @brief Ссылка на экземпляр GameMap.
    GameMap& map_;
};