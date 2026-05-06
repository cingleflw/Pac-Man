#include "input_handler.hpp"

const float ACCURACY = 22.63;

InputHandler* InputHandler::instance_ = 0;
InputHandler::~InputHandler() {}

void InputHandler::handle(SDL_Event event){
    int col;
    int second_col;
    int row;
    float speed = player_.get_speed();
    float x = player_.get_x();
    float y = player_.get_y();
    switch(event.type){
        case SDL_EVENT_KEY_DOWN:
        switch(event.key.scancode){
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                col = map_.pixel_to_tile(x + ACCURACY);
                row = map_.pixel_to_tile(y - speed);
                if (map_.is_walkable_by_player(col, row)){
                    player_.set_direction(0, -speed);
                    player_.set_moving({false, false, true, false});
                    map_.eating_dot(col, row);
                }
                else {
                    player_.set_moving({false, false, false, false});
                }
            break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                col = map_.pixel_to_tile(x);
                row = map_.pixel_to_tile(y + speed + ACCURACY);
                if (map_.is_walkable_by_player(col, row)){
                    player_.set_direction(0, +speed);
                    player_.set_moving({false, false, false, true});
                    map_.eating_dot(col, row);
                }
                else {
                    player_.set_moving({false, false, false, false});
                }
            break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                col = map_.pixel_to_tile(x - speed);
                row = map_.pixel_to_tile(y);
                if (map_.is_walkable_by_player(col, row)){
                    player_.set_direction(-speed, 0);
                    player_.set_moving({false, true, false, false});
                 map_.eating_dot(col, row);
                }
                else {
                    player_.set_moving({false, false, false, false});
                }
            break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                col = map_.pixel_to_tile(x + speed + ACCURACY);
                second_col = map_.pixel_to_tile(x);
                row = map_.pixel_to_tile(y);
                if (map_.is_walkable_by_player(col, row)){
                    player_.set_direction(+speed, 0);
                    player_.set_moving({true, false, false, false});
                    map_.eating_dot(col, row);
                }
                else if (map_.is_walkable_by_player(second_col, row) && (x + 7 > 870)) {
                    player_.set_direction(+speed, 0);
                    player_.set_moving({true, false, false, false});
                    map_.eating_dot(col, row);
                }
                else {
                    player_.set_moving({false, false, false, false});
                }
            break;
            default:
                break;
        }
        break;
        case SDL_EVENT_KEY_UP:
            player_.set_moving({false, false, false, false});
            break;
        default:
        break;
    }
}