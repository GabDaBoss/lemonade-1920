#ifndef GUI_H
#define GUI_H

#include <stdbool.h>
#include <SDL2/SDL.h>

void input_poll_inputs();
bool input_is_key_pressed(SDL_Keycode code);
bool input_is_key_released(SDL_Keycode code);
bool input_is_quit_pressed();
bool input_is_zone_clicked(SDL_Rect zone);

#endif
