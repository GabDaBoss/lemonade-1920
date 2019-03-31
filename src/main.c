#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "input.h"
#include "graphic.h"
#include "scene.h"
#include "main-menu.h"

int
main() 
{

  if (!graphic_init("Lemonade 5000", 1280, 720, 24)) {
    return(EXIT_FAILURE);
  };
  
  MainMenu_Enter();
  Scene_GameLoop();

  graphic_quit();

  return(EXIT_SUCCESS);
}

