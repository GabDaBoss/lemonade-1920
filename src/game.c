#include "game.h"
#include "scene.h"
#include "input.h"
#include "graphic.h"
#include "main-menu.h"

static bool 
update(void)
{
  if (input_is_quit_pressed())
  {
    graphic_clear();
    MainMenu_Enter();
  }

  return true;
}

void 
Game_Enter(void)
{
  Scene_SetUpdateTo(update);
}

