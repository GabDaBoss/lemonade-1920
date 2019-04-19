#include <stdbool.h>
#include <stdlib.h>

#include "scene.h"
#include "graphic.h"
#include "input.h"

static UpdateFunc update;
static bool running = true;

#define MS_PER_UPDATE 8
#define MS_PER_FRAME 16

void 
Scene_GameLoop()
{
  Uint32 current = 0, previous = 0, updateLag = 0;
  running = true;
  while (running) {
    current = SDL_GetTicks();
    Uint32 elapsed = current - previous;
    previous = current;

    updateLag += elapsed;

    int runs = 0;
    while (updateLag >= MS_PER_UPDATE && runs < 5) {
      input_poll_inputs();
      update();

      runs++;
      updateLag -= MS_PER_UPDATE;
    }

    Graphic_Render();
    SDL_Delay(1);
  }
}

void 
Scene_SetUpdateTo(UpdateFunc func)
{
  update = func;
}

void
Scene_Quit() {
  running = false;
}
