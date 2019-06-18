#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef enum {
  NoMouseButton = 0,
  LeftMouseButton = 1 << 0,
  RightMouseButton = 1 << 1,
  MiddleMouseButton = 1 << 2,
} MouseButton;

void Input_PollInputs();
bool Input_IsKeyPressed(SDL_Keycode code);
bool Input_IsKeyReleased(SDL_Keycode code);
bool Input_IsQuitPressed();
bool Input_IsZoneClicked(SDL_Rect zone, MouseButton buttons);
bool Input_IsMouseOverZone(SDL_Rect zone);
void Input_QueryMouseTranslation(int* dx, int* dy);
void Input_QueryMousePosition(int* x, int* y);

#endif
