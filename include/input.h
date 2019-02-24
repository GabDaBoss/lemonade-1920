#ifndef GUI_H
#define GUI_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef enum {
  NoMouseButton = 0,
  LeftMouseButton = 1 << 0,
  RightMouseButton = 1 << 1,
  MiddleMouseButton = 1 << 2,
} MouseButton;

void input_poll_inputs();
bool input_is_key_pressed(SDL_Keycode code);
bool input_is_key_released(SDL_Keycode code);
bool input_is_quit_pressed();
bool input_isZoneClicked(SDL_Rect zone, MouseButton buttons);
bool input_isMouseOverZone(SDL_Rect zone);

#endif
