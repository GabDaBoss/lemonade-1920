#include "input.h"

#define FLAGS_PER_WORD (sizeof(long) * 8)
#define FLAGS_NUM SDL_NUM_SCANCODES / FLAGS_PER_WORD
#define MASK ~(1<<30)


static struct {
    bool quit;
    unsigned long key_pressed[FLAGS_NUM]; 
    unsigned long key_released[FLAGS_NUM]; 
    int x;
    int y;
    MouseButton mouseButtonPressed;
    MouseButton mouseButtonReleased;
    MouseButton mouseButtonClicked;
} state;

static inline int
flag_index(SDL_Keycode code)
{
    return (code & MASK) / FLAGS_PER_WORD;
}

void 
input_poll_inputs()
{
    SDL_Event event;
    for(int i = FLAGS_NUM; i-- > 0; ) {
        state.key_released[i] = 0;
    }

    state.mouseButtonReleased = 0;

    while (SDL_PollEvent(&event) != 0) {
        SDL_Keycode code = event.key.keysym.sym;
        int i = flag_index(code);
        int pos = (code & MASK) - (FLAGS_PER_WORD * i);
        switch (event.type) {
            case SDL_QUIT:
                state.quit = true;
                break;
            case SDL_KEYDOWN:
                state.key_pressed[i] |= (long) 1 << pos;
                break;
            case SDL_KEYUP:
                state.key_pressed[i] &= ~(1 << pos);
                state.key_released[i] |= 1 << pos;
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button) {
                  case SDL_BUTTON_LEFT:
                    state.mouseButtonPressed |= LeftMouseButton;
                    break;
                  case SDL_BUTTON_RIGHT:
                    state.mouseButtonPressed |=  RightMouseButton;
                    break;
                  case SDL_BUTTON_MIDDLE:
                    state.mouseButtonPressed |= MiddleMouseButton;
                    break;
                }
                state.x = event.button.x;
                state.y = event.button.y;
                break;
            case SDL_MOUSEBUTTONUP:
                switch (event.button.button) {
                  case SDL_BUTTON_LEFT:
                    state.mouseButtonReleased |= LeftMouseButton;
                    break;
                  case SDL_BUTTON_RIGHT:
                    state.mouseButtonReleased |=  RightMouseButton;
                    break;
                  case SDL_BUTTON_MIDDLE:
                    state.mouseButtonReleased |= MiddleMouseButton;
                    break;
                }
                state.x = event.button.x;
                state.y = event.button.y;
                break;
        }
    }
}

bool 
input_is_key_pressed(SDL_Keycode code)
{
    int i = flag_index(code);
    int pos = (code & MASK) - (FLAGS_PER_WORD * i);
    return state.key_pressed[i] & 1 << pos;
}

bool 
input_is_key_released(SDL_Keycode code)
{
    int i = flag_index(code);
    int pos = (code & MASK) - (FLAGS_PER_WORD * i);
    return state.key_released[i] & 1 << pos;
}

bool
input_is_quit_pressed()
{
    return state.quit;
}

bool input_isZoneClicked(SDL_Rect zone, MouseButton buttons)
{
  if (state.mouseButtonReleased & buttons) {
    if (state.x >= zone.x && 
        state.x <= zone.x + zone.w &&
        state.y >= zone.y &&
        state.y <= zone.y + zone.h) {
      return true;
    }
  }

  return false;
}
