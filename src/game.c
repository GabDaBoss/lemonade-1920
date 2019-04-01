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
    printf("bye!\n");
    // return false;
    // graphic_clear();
    MainMenu_Enter();
    // printf("bye!");
  }

  return true;
}

void 
Game_Enter(void)
{
  Scene_SetUpdateTo(update);
}

