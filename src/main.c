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
static const SDL_Color greenTextColor = { 0x22, 0x55, 0, 0xFF };

static Id backgroundTextureId;
static Id okButtonTextureId;
static Id backButtonTextureId;

static Id lightScreenSolidTextureId = VOID_ID;
static Id greenSolidTextureId = VOID_ID;

static struct {
  bool opened;
  Id background;
  Id leftBorder;
  Id bottomBorder;
  Id rightBorder;
  Id topBar;
  Id topText;
  Id firstLevelButton;
  Id okButton;
  Id backButton;
  struct  {
    int value;
    Id leftBorder;
    Id bottomBorder;
    Id rightBorder;
    Id topBorder;
  } selectedLevelButton;
} levelSelector = { 
  false,
  VOID_ID,
  VOID_ID,
  VOID_ID,
  VOID_ID,
  VOID_ID,
  VOID_ID,
  VOID_ID,
  VOID_ID,
  VOID_ID,
  {
    0,
    VOID_ID,
    VOID_ID,
    VOID_ID,
    VOID_ID,
  },
};


void createMainMenu();
void centerMainMenu();
bool handleMainMenuEvents();
void openLevelSelector();
void closeLevelSelector();
void setBorderAroundSelectedButton();

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
  okButtonTextureId = graphic_loadTexture("ok.png");
  backButtonTextureId = graphic_loadTexture("back.png");
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
      running = handleMainMenuEvents();
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

bool 
handleMainMenuEvents() 
{
  if (levelSelector.opened) 
  {
    if (input_is_key_released(SDLK_ESCAPE)) 
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
        case 1:
          break;
        case 2:
          return false;
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
        return true;
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

  return true;
}

void
openLevelSelector()
{
  levelSelector.opened = true;
  if (levelSelector.background == VOID_ID) {
    SDL_Rect backgroundDest;
    graphic_queryBackgroundDest(&backgroundDest);

    backgroundDest.x += backgroundDest.w * 0.1;
    backgroundDest.y += backgroundDest.h * 0.1;
    backgroundDest.w = backgroundDest.w * 0.8;
    backgroundDest.h = backgroundDest.h * 0.8;
    levelSelector.background = 
      graphic_createFullTextureSprite(lightScreenSolidTextureId, 
                                      backgroundDest); 

    SDL_Rect leftBorderDest;
    leftBorderDest.x = backgroundDest.x;
    leftBorderDest.y = backgroundDest.y;
    leftBorderDest.w = 2;
    leftBorderDest.h = backgroundDest.h;
    levelSelector.leftBorder = 
      graphic_createFullTextureSprite(greenSolidTextureId, leftBorderDest);

    SDL_Rect bottomBorderDest;
    bottomBorderDest.x = backgroundDest.x;
    bottomBorderDest.y = backgroundDest.y + backgroundDest.h - 2;
    bottomBorderDest.w = backgroundDest.w;
    bottomBorderDest.h = 2;
    levelSelector.bottomBorder = 
      graphic_createFullTextureSprite(greenSolidTextureId, bottomBorderDest);

    SDL_Rect rightBorderDest;
    rightBorderDest.x = backgroundDest.x + backgroundDest.w - 2;
    rightBorderDest.y = backgroundDest.y;
    rightBorderDest.w = 2;
    rightBorderDest.h = backgroundDest.h;
    levelSelector.rightBorder = 
      graphic_createFullTextureSprite(greenSolidTextureId, rightBorderDest);

    SDL_Rect topBarDest;
    topBarDest.x = backgroundDest.x;
    topBarDest.y = backgroundDest.y;
    topBarDest.w = backgroundDest.w;
    topBarDest.h = 10;
    levelSelector.topBar = 
      graphic_createFullTextureSprite(greenSolidTextureId, topBarDest);

    SDL_Rect topTextZone;
    topTextZone.x = backgroundDest.x;
    topTextZone.y = backgroundDest.y + 20;
    topTextZone.w = backgroundDest.w;
    topTextZone.h = 40;
    levelSelector.topText =
      graphic_createTextCentered("Select Level", topTextZone, greenTextColor);

    SDL_Rect firstLevelButtonDest;
    firstLevelButtonDest.x = backgroundDest.x + backgroundDest.w * 0.05;
    firstLevelButtonDest.y = backgroundDest.y + 60;
    firstLevelButtonDest.w = backgroundDest.w * 0.2;
    firstLevelButtonDest.h = backgroundDest.w * 0.2;

    SDL_Rect firstLevelButtonSrc;
    graphic_queryTextureSize(backgroundTextureId, 
                             &firstLevelButtonSrc.w, 
                             &firstLevelButtonSrc.h);

    firstLevelButtonSrc.x = (firstLevelButtonSrc.w - firstLevelButtonSrc.h) /2;
    firstLevelButtonSrc.y = 0;
    firstLevelButtonSrc.w = firstLevelButtonSrc.h;
    levelSelector.firstLevelButton = 
      graphic_createTilesetSprite(backgroundTextureId,
                                  firstLevelButtonSrc, 
                                  firstLevelButtonDest);

    levelSelector.selectedLevelButton.value = 0;
    setBorderAroundSelectedButton();

    SDL_Rect okButton;
    okButton.x = backgroundDest.x + backgroundDest.w - 80;
    okButton.y = backgroundDest.y + backgroundDest.h - 30;
    okButton.w = 40;
    okButton.h = 25;
    levelSelector.okButton =
      graphic_createFullTextureSprite(okButtonTextureId, okButton);

    SDL_Rect backButton;
    backButton.x = backgroundDest.x + backgroundDest.w - 45;
    backButton.y = backgroundDest.y + backgroundDest.h - 30;
    backButton.w = 40;
    backButton.h = 25;
    levelSelector.backButton =
      graphic_createFullTextureSprite(backButtonTextureId, backButton);
  }
}


void
closeLevelSelector()
{
  levelSelector.opened = false;
  graphic_deleteSprite(levelSelector.background);
  graphic_deleteSprite(levelSelector.leftBorder);
  graphic_deleteSprite(levelSelector.bottomBorder);
  graphic_deleteSprite(levelSelector.rightBorder);
  graphic_deleteSprite(levelSelector.topBar);
  graphic_deleteSprite(levelSelector.firstLevelButton);
  graphic_deleteSprite(levelSelector.selectedLevelButton.leftBorder);
  graphic_deleteSprite(levelSelector.selectedLevelButton.bottomBorder);
  graphic_deleteSprite(levelSelector.selectedLevelButton.rightBorder);
  graphic_deleteSprite(levelSelector.selectedLevelButton.topBorder);
  graphic_deleteText(levelSelector.topText);
  graphic_deleteSprite(levelSelector.okButton);
  graphic_deleteSprite(levelSelector.backButton);
  levelSelector.background = VOID_ID;
  levelSelector.leftBorder = VOID_ID;
  levelSelector.bottomBorder = VOID_ID;
  levelSelector.rightBorder = VOID_ID;
  levelSelector.topBar = VOID_ID;
  levelSelector.topText = VOID_ID;
  levelSelector.okButton = VOID_ID;
  levelSelector.backButton = VOID_ID;
  levelSelector.firstLevelButton = VOID_ID;
  levelSelector.selectedLevelButton.leftBorder = VOID_ID;
  levelSelector.selectedLevelButton.bottomBorder = VOID_ID;
  levelSelector.selectedLevelButton.rightBorder = VOID_ID;
  levelSelector.selectedLevelButton.topBorder = VOID_ID;
}

void
setBorderAroundSelectedButton()
{
  SDL_Rect selectedButtonDest;
  switch (levelSelector.selectedLevelButton.value)
  {
    case 0:
      graphic_querySpriteDest(levelSelector.firstLevelButton, &selectedButtonDest);
      break;
  }

  SDL_Rect leftBorderDest;
  leftBorderDest.x = selectedButtonDest.x;
  leftBorderDest.y = selectedButtonDest.y;
  leftBorderDest.w = 2;
  leftBorderDest.h = selectedButtonDest.h;
  levelSelector.selectedLevelButton.leftBorder = 
    graphic_createFullTextureSprite(greenSolidTextureId, leftBorderDest);

  SDL_Rect bottomBorderDest;
  bottomBorderDest.x = selectedButtonDest.x;
  bottomBorderDest.y = selectedButtonDest.y + selectedButtonDest.h - 2;
  bottomBorderDest.w = selectedButtonDest.w;
  bottomBorderDest.h = 2;
  levelSelector.selectedLevelButton.bottomBorder = 
    graphic_createFullTextureSprite(greenSolidTextureId, bottomBorderDest);

  SDL_Rect rightBorderDest;
  rightBorderDest.x = selectedButtonDest.x + selectedButtonDest.w - 2;
  rightBorderDest.y = selectedButtonDest.y;
  rightBorderDest.w = 2;
  rightBorderDest.h = selectedButtonDest.h;
  levelSelector.selectedLevelButton.rightBorder = 
    graphic_createFullTextureSprite(greenSolidTextureId, rightBorderDest);

  SDL_Rect topBarDest;
  topBarDest.x = selectedButtonDest.x;
  topBarDest.y = selectedButtonDest.y;
  topBarDest.w = selectedButtonDest.w;
  topBarDest.h = 2;
  levelSelector.selectedLevelButton.topBorder = 
    graphic_createFullTextureSprite(greenSolidTextureId, topBarDest);
}
