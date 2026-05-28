/**
 * @file input_handler.cpp
 * @brief Реализация методов класса InputHandler.
 */

#include "input_handler.hpp"

InputHandler* InputHandler::instance_ = 0;
InputHandler::~InputHandler() {}

void InputHandler::handle(SDL_Event event){
    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
        switch(event.key.scancode){
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                player_.set_desired_direction(Direction::Up);
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                player_.set_desired_direction(Direction::Down);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                player_.set_desired_direction(Direction::Left);
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                player_.set_desired_direction(Direction::Right);
                break;
            default:
                break;
        }
        break;
    default:
        break;
    }
}