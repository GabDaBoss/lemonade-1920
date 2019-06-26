#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "input.h"
#include "graphic.h"
#include "scene.h"
#include "main-menu.h"
#include "widget.h"

int
main() 
{
  if (!Graphic_Init("Lemonade 5000", 1280, 720, 24)) {
    return(EXIT_FAILURE);
  };
  
  Graphic_InitCamera();
  Widget_Init();

  MainMenu_Enter();
  Scene_GameLoop();

  Graphic_Quit();

  return(EXIT_SUCCESS);
}

