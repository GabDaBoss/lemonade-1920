#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "input.h"
#include "graphic.h"

#define MS_PER_UPDATE 8

static Id mainMenuTitle;
static Id playButton;
static Id quitButton;
static Id highScoresButton;
static double selectedButton;
static const double SELECTION_SPEED = 0.10;
static const SDL_Color textColor = {255, 255, 0, 255};

static Id backgroundTextureId;

static bool levelSelectorOpened;
static Id lightScreenSolidTextureId = VOID_ID;
static Id greenSolidTextureId = VOID_ID;

static Id levelSelectorBackground = VOID_ID;
static Id levelSelectLeftBorder = VOID_ID;
static Id levelSelectBottomBorder = VOID_ID;
static Id levelSelectRightBorder = VOID_ID;
static Id levelSelectTopBar = VOID_ID;

static Id firstLevelButton = VOID_ID;

void createMainMenu();
void centerMainMenu();
void handleMainMenuEvents();
void openLevelSelector();
void closeLevelSelector();

int
main() 
{
  bool running = true;

  if (!graphic_init("Lemonade 5000", 1200, 400, 24)) 
  {
    return(EXIT_FAILURE);
  };

  lightScreenSolidTextureId = graphic_createSolidTexture(0xEEFFAA);
  greenSolidTextureId = graphic_createSolidTexture(0x225500);
  backgroundTextureId = graphic_loadTexture("background.png");
  graphic_setBackgroundTexture(backgroundTextureId);

  createMainMenu();

  Uint32 current = 0, previous = 0, lag = 0;
  while (running) 
  {

    current = SDL_GetTicks();
    Uint32 elapsed = current - previous;
    previous = current;

    lag += elapsed;

    int runs = 0;
    while (lag >= MS_PER_UPDATE && runs < 5 && running) 
    {
      input_poll_inputs();
      running = !input_is_quit_pressed();
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
createMainMenu() 
{
  mainMenuTitle = graphic_createText("Lemonade 5000", 0, 40, textColor);
  playButton = graphic_createText(">New", 0, 0, textColor);
  quitButton = graphic_createText("Load", 0, 0, textColor);
  highScoresButton = graphic_createText("Quit", 0, 0, textColor);
}

void 
centerMainMenu() 
{
  graphic_centerSpriteOnScreenWidth(mainMenuTitle);
  graphic_centerSpriteOnScreenWithOffset(playButton, 0, -20);
  graphic_centerSpriteOnScreenWithOffset(quitButton, 0, 0);
  graphic_centerSpriteOnScreenWithOffset(highScoresButton, 0, 20);
}

void 
handleMainMenuEvents() 
{
  if (levelSelectorOpened) 
  {
    if (input_is_key_released(SDLK_RETURN) || 
        input_is_key_released(SDLK_RETURN2)) 
    {
      closeLevelSelector();
    }
  } 
  else 
  {
    if (input_is_key_released(SDLK_RETURN) ||
        input_is_key_released(SDLK_RETURN2)) 
    {
      switch((int) selectedButton) 
      {
        case 0:
          openLevelSelector();
          break;
      }
    } 
    else 
    {
      int prev = selectedButton;
      if (input_is_key_pressed(SDLK_DOWN)) 
      {
        prev = selectedButton;
        selectedButton += SELECTION_SPEED;
        if (selectedButton >= 3) 
        {
            selectedButton = 0;
        }
      } 
      else if (input_is_key_pressed(SDLK_UP)) 
      {
        selectedButton -= SELECTION_SPEED;
        if (selectedButton < 0) 
        {
            selectedButton = 2.99;
        }
      }

      if(selectedButton == prev) 
      {
        return;
      }

      switch((int) selectedButton) 
      {
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
  }
}

void
openLevelSelector()
{
  levelSelectorOpened = true;
  if (levelSelectorBackground == VOID_ID) {
    SDL_Rect backgroundDest;
    graphic_queryBackgroundDest(&backgroundDest);

    backgroundDest.x += backgroundDest.w * 0.1;
    backgroundDest.y += backgroundDest.h * 0.1;
    backgroundDest.w = backgroundDest.w * 0.8;
    backgroundDest.h = backgroundDest.h * 0.8;
    levelSelectorBackground = 
      graphic_createFullTextureSprite(lightScreenSolidTextureId, 
                                      backgroundDest); 

    SDL_Rect leftBorderDest;
    leftBorderDest.x = backgroundDest.x;
    leftBorderDest.y = backgroundDest.y;
    leftBorderDest.w = 2;
    leftBorderDest.h = backgroundDest.h;
    levelSelectLeftBorder = 
      graphic_createFullTextureSprite(greenSolidTextureId, leftBorderDest);

    SDL_Rect bottomBorderDest;
    bottomBorderDest.x = backgroundDest.x;
    bottomBorderDest.y = backgroundDest.y + backgroundDest.h - 2;
    bottomBorderDest.w = backgroundDest.w;
    bottomBorderDest.h = 2;
    levelSelectBottomBorder = 
      graphic_createFullTextureSprite(greenSolidTextureId, bottomBorderDest);

    SDL_Rect rightBorderDest;
    rightBorderDest.x = backgroundDest.x + backgroundDest.w - 2;
    rightBorderDest.y = backgroundDest.y;
    rightBorderDest.w = 2;
    rightBorderDest.h = backgroundDest.h;
    levelSelectRightBorder = 
      graphic_createFullTextureSprite(greenSolidTextureId, rightBorderDest);

    SDL_Rect topBarDest;
    topBarDest.x = backgroundDest.x;
    topBarDest.y = backgroundDest.y;
    topBarDest.w = backgroundDest.w;
    topBarDest.h = 10;
    levelSelectTopBar = 
      graphic_createFullTextureSprite(greenSolidTextureId, topBarDest);

    SDL_Rect firstLevelButtonDest;
    firstLevelButtonDest.x = backgroundDest.x + backgroundDest.w * 0.05;
    firstLevelButtonDest.y = backgroundDest.y + backgroundDest.h * 0.05;
    firstLevelButtonDest.w = backgroundDest.w * 0.2;
    firstLevelButtonDest.h = backgroundDest.w * 0.2;

    SDL_Rect firstLevelButtonSrc;
    graphic_queryTextureSize(backgroundTextureId, 
                             &firstLevelButtonSrc.w, 
                             &firstLevelButtonSrc.h);

    firstLevelButtonSrc.x = (firstLevelButtonSrc.w - firstLevelButtonSrc.h) /2;
    firstLevelButtonSrc.y = 0;
    firstLevelButtonSrc.w = firstLevelButtonSrc.h;
    firstLevelButton = 
      graphic_createTilesetSprite(backgroundTextureId,
                                  firstLevelButtonSrc, 
                                  firstLevelButtonDest);
  }
}


void
closeLevelSelector()
{
  levelSelectorOpened = false;
  graphic_deleteSprite(levelSelectorBackground);
  graphic_deleteSprite(levelSelectLeftBorder);
  graphic_deleteSprite(levelSelectBottomBorder);
  graphic_deleteSprite(levelSelectRightBorder);
  graphic_deleteSprite(levelSelectTopBar);
  graphic_deleteSprite(firstLevelButton);
  levelSelectorBackground = VOID_ID;
  levelSelectLeftBorder = VOID_ID;
  levelSelectBottomBorder = VOID_ID;
  levelSelectRightBorder = VOID_ID;
  levelSelectTopBar = VOID_ID;
  firstLevelButton = VOID_ID;
}
