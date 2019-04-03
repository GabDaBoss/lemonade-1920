#include "game.h"
#include "scene.h"
#include "input.h"
#include "graphic.h"
#include "main-menu.h"

static void 
update(void)
{
  if (input_is_quit_pressed())
  {
    graphic_clear();
    MainMenu_Enter();
  }
}

void 
Game_Enter(void)
{
  Scene_SetUpdateTo(update);
}

