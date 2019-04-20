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
  if (!Graphic_Init("Lemonade 5000", 1280, 720, 24)) {
    return(EXIT_FAILURE);
  };
  
  Graphic_InitCamera();
  MainMenu_Enter();
  Scene_GameLoop();

  Graphic_Quit();

  return(EXIT_SUCCESS);
}

