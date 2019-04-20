#include "input.h"

#define FLAGS_PER_WORD (sizeof(long) * 8)
#define FLAGS_NUM SDL_NUM_SCANCODES / FLAGS_PER_WORD
#define MASK ~(1<<30)


static struct {
  bool quit;
  unsigned long keyPressed[FLAGS_NUM]; 
  unsigned long keyReleased[FLAGS_NUM]; 
  int x;
  int y;
  MouseButton mouseButtonPressed;
  MouseButton mouseButtonReleased;
  MouseButton mouseButtonClicked;
} _state;

static int _dx, _dy;

static inline int
flagIndex(SDL_Keycode code)
{
  return (code & MASK) / FLAGS_PER_WORD;
}

void 
Input_PollInputs()
{
  SDL_Event event;
  for(int i = FLAGS_NUM; i-- > 0; ) {
    _state.keyReleased[i] = 0;
  }

  _state.quit = false;

  _state.mouseButtonReleased = 0;
  int prevX = _state.x, prevY = _state.y;
  SDL_GetMouseState(&_state.x, &_state.y);
  _dx = _state.x - prevX;
  _dy = _state.y - prevY;

  while (SDL_PollEvent(&event) != 0) {
    SDL_Keycode code = event.key.keysym.sym;
    int i = flagIndex(code);
    int pos = (code & MASK) - (FLAGS_PER_WORD * i);
    switch (event.type) {
      case SDL_QUIT:
        _state.quit = true;
        break;
      case SDL_KEYDOWN:
        _state.keyPressed[i] |= (long) 1 << pos;
        break;
      case SDL_KEYUP:
        _state.keyPressed[i] &= (long) ~((long)1 << pos);
        _state.keyReleased[i] |= (long) 1 << pos;
        break;
      case SDL_MOUSEBUTTONDOWN:
        switch (event.button.button) {
          case SDL_BUTTON_LEFT:
            _state.mouseButtonPressed |= LeftMouseButton;
            break;
          case SDL_BUTTON_RIGHT:
            _state.mouseButtonPressed |=  RightMouseButton;
            break;
          case SDL_BUTTON_MIDDLE:
            _state.mouseButtonPressed |= MiddleMouseButton;
            break;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        switch (event.button.button) {
          case SDL_BUTTON_LEFT:
            _state.mouseButtonReleased |= LeftMouseButton;
            break;
          case SDL_BUTTON_RIGHT:
            _state.mouseButtonReleased |=  RightMouseButton;
            break;
          case SDL_BUTTON_MIDDLE:
            _state.mouseButtonReleased |= MiddleMouseButton;
            break;
        }
        break;
    }
  }
}

bool 
Input_IsKeyPressed(SDL_Keycode code)
{
  int i = flagIndex(code);
  int pos = (code & MASK) - (FLAGS_PER_WORD * i);
  return _state.keyPressed[i] & 1 << pos;
}

bool 
Input_IsKeyReleased(SDL_Keycode code)
{
  int i = flagIndex(code);
  int pos = (code & MASK) - (FLAGS_PER_WORD * i);
  return _state.keyReleased[i] & (long) 1 << pos;
}

bool
Input_IsQuitPressed()
{
  return _state.quit;
}

bool 
Input_IsZoneClicked(SDL_Rect zone, MouseButton buttons)
{
  if (_state.mouseButtonReleased & buttons) {
    if (_state.x >= zone.x && 
        _state.x <= zone.x + zone.w &&
        _state.y >= zone.y &&
        _state.y <= zone.y + zone.h) {
      return true;
    }
  }

  return false;
}

bool 
Input_IsMouseOverZone(SDL_Rect zone)
{
  return _state.x >= zone.x && 
         _state.x <= zone.x + zone.w &&
         _state.y >= zone.y &&
         _state.y <= zone.y + zone.h;
}

void
Input_QueryMouseTranslation(int* dx, int* dy)
{
  *dx = _dx;
  *dy = _dy;
}

void Input_QueryMousePosition(int* x, int* y)
{
  *x = _state.x;
  *y = _state.y;
}
