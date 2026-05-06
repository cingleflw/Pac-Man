#pragma once

#include <SDL3/SDL.h>

#include "game_map.hpp"
#include "player.hpp"

class InputHandler {
public:
    static InputHandler* Instance(Player& p, GameMap& m){
        if (instance_ == 0){
            instance_ = new InputHandler(p, m);
        }
        return instance_;
    }

    void handle(SDL_Event event);
private:
    InputHandler(Player& p, GameMap& m) : player_(p), map_(m) {};
    ~InputHandler();
    static InputHandler* instance_;
    Player& player_;
    GameMap& map_;
};