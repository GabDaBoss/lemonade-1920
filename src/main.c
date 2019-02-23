#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "input.h"
#include "graphic.h"

#define MS_PER_UPDATE 8

static SpriteTextId mainMenuTitle;
static SpriteTextId playButton;
static SpriteTextId quitButton;
static SpriteTextId highScoresButton;
static double selectedButton;
static const double SELECTION_SPEED = 0.10;
static const SDL_Color textColor = {255, 255, 0, 255};

static TextureId backgroundTextureId;

void createMainMenu();
void centerMainMenu();
void handleMainMenuEvents();

int main() {
  bool running = true;

  if (!graphic_init("Lemonade 5000", 1200, 400, 24)) {
    return(EXIT_FAILURE);
  };

  backgroundTextureId = graphic_loadTexture("background.png");
  graphic_setBackgroundTexture(backgroundTextureId);

  createMainMenu();

  Uint32 current = 0, previous = 0, lag = 0;
  while (running) {
    input_poll_inputs();
    running = !input_is_quit_pressed();

    current = SDL_GetTicks();
    Uint32 elapsed = current - previous;
    previous = current;

    lag += elapsed;

    int runs = 0;
    while (lag >= MS_PER_UPDATE && runs < 5 && running) {
      handleMainMenuEvents();
      centerMainMenu();
      runs++;
      lag -= MS_PER_UPDATE;
    }

    graphic_resizeBackgroundToScreen();
    graphic_render();

    SDL_Delay(1);
  }

  graphic_quit();

  return(EXIT_SUCCESS);
}

void
createMainMenu() {
  mainMenuTitle = graphic_createText("Lemonade 5000", 0, 40, textColor);
  playButton = graphic_createText(">New", 0, 0, textColor);
  quitButton = graphic_createText("Load", 0, 0, textColor);
  highScoresButton = graphic_createText("Quit", 0, 0, textColor);
}

void 
centerMainMenu() {
  graphic_centerSpriteOnScreenWidth(mainMenuTitle);
  graphic_centerSpriteOnScreenWithOffset(playButton, 0, -20);
  graphic_centerSpriteOnScreenWithOffset(quitButton, 0, 0);
  graphic_centerSpriteOnScreenWithOffset(highScoresButton, 0, 20);
}

void 
handleMainMenuEvents() 
{
  int prev = selectedButton;
  if(input_is_key_pressed(SDLK_DOWN)) {
    prev = selectedButton;
    selectedButton += SELECTION_SPEED;
    if(selectedButton >= 3) {
      selectedButton = 0;
    }
  } else if (input_is_key_pressed(SDLK_UP)) {
    selectedButton -= SELECTION_SPEED;
    if (selectedButton < 0) {
      selectedButton = 2.99;
    }
  }

  if(selectedButton == prev) {
    return;
  }
  switch((int) selectedButton) {
    case 0:
      graphic_setText(playButton, ">New", 0, 0, textColor);
      graphic_setText(quitButton, "Load", 0, 0, textColor);
      graphic_setText(highScoresButton, "Quit", 0, 0, textColor);
      break;
    case 1:
      graphic_setText(playButton, "New", 0, 0, textColor);
      graphic_setText(quitButton, ">Load", 0, 0, textColor);
      graphic_setText(highScoresButton, "Quit", 0, 0, textColor);
      break;
    case 2:
      graphic_setText(playButton, "New", 0, 0, textColor);
      graphic_setText(quitButton, "Load", 0, 0, textColor);
      graphic_setText(highScoresButton, ">Quit", 0, 0, textColor);
      break;
  }
}

