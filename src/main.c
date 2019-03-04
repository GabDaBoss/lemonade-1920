#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "input.h"
#include "graphic.h"

#define MS_PER_UPDATE 8

static Id mainMenuTitle;
static Id newButton;
static Id loadButton;
static Id quitButton;
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
  struct {
    Id id;
    Id leftBorder;
    Id bottomBorder;
    Id rightBorder;
    Id topBorder;
  } selectedLevelButton;
  struct {
    Id id;
    Id leftBorder;
    Id bottomBorder;
    Id rightBorder;
    Id topBorder;
  } hoveredButton;
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
    VOID_ID,
    VOID_ID,
    VOID_ID,
    VOID_ID,
    VOID_ID,
  },
  {
    VOID_ID,
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
void createLevelSelector();
void closeLevelSelector();
void setBorderAroundHoveredButton();

void clear();

int
main() 
{
  bool running = true;

  if (!graphic_init("Lemonade 5000", 1280, 720, 24)) {
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
  while (running) {

    current = SDL_GetTicks();
    Uint32 elapsed = current - previous;
    previous = current;

    lag += elapsed;

    int runs = 0;
    while (lag >= MS_PER_UPDATE && runs < 5 && running) {
      input_poll_inputs();
      running = !input_is_quit_pressed() &&
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
  newButton = graphic_createText(">New", 0, 0, textColor);
  loadButton = graphic_createText("Load", 0, 0, textColor);
  quitButton = graphic_createText("Quit", 0, 0, textColor);
  createLevelSelector();
}

void 
centerMainMenu() 
{
  graphic_centerSpriteOnScreenWidth(mainMenuTitle);
  graphic_centerSpriteOnScreenWithOffset(newButton, 0, -20);
  graphic_centerSpriteOnScreenWithOffset(loadButton, 0, 0);
  graphic_centerSpriteOnScreenWithOffset(quitButton, 0, 20);
}

bool 
handleMainMenuEvents() 
{
  if (levelSelector.opened) {
    SDL_Rect okButtonRect, backButtonRect;
    printf("bug?\n");
    graphic_querySpriteDest(levelSelector.okButton, &okButtonRect);
    graphic_querySpriteDest(levelSelector.backButton, &backButtonRect);
    printf("no-bug\n");

    if (input_is_key_released(SDLK_ESCAPE)) {
      closeLevelSelector();
    } else if(input_isZoneClicked(okButtonRect, LeftMouseButton)) {
      closeLevelSelector();
    } else if(input_isZoneClicked(backButtonRect, LeftMouseButton)) {
      closeLevelSelector();
    }

    if (input_isMouseOverZone(okButtonRect)) {
      if (levelSelector.hoveredButton.id != levelSelector.okButton) {
        levelSelector.hoveredButton.id = levelSelector.okButton;
        setBorderAroundHoveredButton();
      }
    } else if (input_isMouseOverZone(backButtonRect)) {
      if (levelSelector.hoveredButton.id != levelSelector.backButton) {
        levelSelector.hoveredButton.id = levelSelector.backButton;
        setBorderAroundHoveredButton();
      }
    } else if (levelSelector.hoveredButton.id != VOID_ID) {
      levelSelector.hoveredButton.id = VOID_ID;
      graphic_setSpriteToInvisible(levelSelector.hoveredButton.leftBorder);
      graphic_setSpriteToInvisible(levelSelector.hoveredButton.bottomBorder);
      graphic_setSpriteToInvisible(levelSelector.hoveredButton.rightBorder);
      graphic_setSpriteToInvisible(levelSelector.hoveredButton.topBorder);
    }

  } else {
    SDL_Rect newButtonRect, loadButtonRect, quitButtonRect;
    graphic_querySpriteDest(newButton, &newButtonRect);
    graphic_querySpriteDest(loadButton, &loadButtonRect);
    graphic_querySpriteDest(quitButton, &quitButtonRect);

    if (input_is_key_released(SDLK_RETURN) ||
        input_is_key_released(SDLK_RETURN2)) {
      switch((int) selectedButton) {
        case 0:
          openLevelSelector();
          break;
        case 1:
          break;
        case 2:
          return false;
          break;
      }
    } else if (input_isZoneClicked(newButtonRect, LeftMouseButton)) {
      openLevelSelector();
    } else if (input_isZoneClicked(loadButtonRect, LeftMouseButton)) {
      // Load
    } else if (input_isZoneClicked(quitButtonRect, LeftMouseButton)) {
      return false;
    } else {
      int prev = selectedButton;


      if (input_is_key_pressed(SDLK_DOWN)) {
        prev = selectedButton;
        selectedButton += SELECTION_SPEED;
        if (selectedButton >= 3) {
          selectedButton = 0;
        }
      } else if (input_is_key_pressed(SDLK_UP)) {
        selectedButton -= SELECTION_SPEED;
        if (selectedButton < 0) {
          selectedButton = 2.99;
        }
      } else if (input_isMouseOverZone(newButtonRect)) {
        selectedButton = 0;
      } else if (input_isMouseOverZone(loadButtonRect)) {
        selectedButton = 1;
      } else if (input_isMouseOverZone(quitButtonRect)) {
        selectedButton = 2;
      }

      if(selectedButton == prev) {
        return true;
      }

      switch((int) selectedButton) {
        case 0:
          graphic_setText(newButton, ">New", 0, 0, textColor);
          graphic_setText(loadButton, "Load", 0, 0, textColor);
          graphic_setText(quitButton, "Quit", 0, 0, textColor);
          break;
        case 1:
          graphic_setText(newButton, "New", 0, 0, textColor);
          graphic_setText(loadButton, ">Load", 0, 0, textColor);
          graphic_setText(quitButton, "Quit", 0, 0, textColor);
          break;
        case 2:
          graphic_setText(newButton, "New", 0, 0, textColor);
          graphic_setText(loadButton, "Load", 0, 0, textColor);
          graphic_setText(quitButton, ">Quit", 0, 0, textColor);
          break;
      }
    }
  }

  return true;
}


void
createLevelSelector()
{
    SDL_Rect backgroundDest;
    graphic_queryBackgroundDest(&backgroundDest);

    levelSelector.background = graphic_createInvisibleSprite(lightScreenSolidTextureId); 
    levelSelector.leftBorder = graphic_createInvisibleSprite(greenSolidTextureId); 
    levelSelector.bottomBorder = graphic_createInvisibleSprite(greenSolidTextureId);
    levelSelector.rightBorder = graphic_createInvisibleSprite(greenSolidTextureId);
    levelSelector.topBar = graphic_createInvisibleSprite(greenSolidTextureId);
    levelSelector.topText = graphic_createInvisbleText("Select Level", greenTextColor);
    levelSelector.firstLevelButton = graphic_createInvisibleSprite(backgroundTextureId);
    levelSelector.selectedLevelButton.id = levelSelector.firstLevelButton;
    levelSelector.selectedLevelButton.leftBorder = graphic_createInvisibleSprite(greenSolidTextureId);
    levelSelector.selectedLevelButton.bottomBorder = graphic_createInvisibleSprite(greenSolidTextureId);
    levelSelector.selectedLevelButton.rightBorder = graphic_createInvisibleSprite(greenSolidTextureId);
    levelSelector.selectedLevelButton.topBorder = graphic_createInvisibleSprite(greenSolidTextureId);
    levelSelector.okButton = graphic_createInvisibleSprite(okButtonTextureId);
    levelSelector.backButton = graphic_createInvisibleSprite(backButtonTextureId);

}

void
openLevelSelector()
{
    levelSelector.opened = true;

    SDL_Rect backgroundDest;
    graphic_queryBackgroundDest(&backgroundDest);

    backgroundDest.x += backgroundDest.w * 0.1;
    backgroundDest.y += backgroundDest.h * 0.1;
    backgroundDest.w = backgroundDest.w * 0.8;
    backgroundDest.h = backgroundDest.h * 0.8;
    graphic_setSpriteDest(levelSelector.background, backgroundDest);
    
    SDL_Rect leftBorderDest;
    leftBorderDest.x = backgroundDest.x;
    leftBorderDest.y = backgroundDest.y;
    leftBorderDest.w = 2;
    leftBorderDest.h = backgroundDest.h;
    graphic_setSpriteDest(levelSelector.leftBorder, leftBorderDest);

    SDL_Rect bottomBorderDest;
    bottomBorderDest.x = backgroundDest.x;
    bottomBorderDest.y = backgroundDest.y + backgroundDest.h - 2;
    bottomBorderDest.w = backgroundDest.w;
    bottomBorderDest.h = 2;
    graphic_setSpriteDest(levelSelector.bottomBorder, bottomBorderDest);

    SDL_Rect rightBorderDest;
    rightBorderDest.x = backgroundDest.x + backgroundDest.w - 2;
    rightBorderDest.y = backgroundDest.y;
    rightBorderDest.w = 2;
    rightBorderDest.h = backgroundDest.h;
    graphic_setSpriteDest(levelSelector.rightBorder, rightBorderDest);

    SDL_Rect topBarDest;
    topBarDest.x = backgroundDest.x;
    topBarDest.y = backgroundDest.y;
    topBarDest.w = backgroundDest.w;
    topBarDest.h = 10;
    graphic_setSpriteDest(levelSelector.topBar, topBarDest);

    SDL_Rect topTextZone;
    topTextZone.x = backgroundDest.x;
    topTextZone.y = backgroundDest.y + 20;
    topTextZone.w = backgroundDest.w;
    topTextZone.h = 40;
    graphic_centerSpriteInRect(levelSelector.topText, topTextZone);

    SDL_Rect firstLevelButtonDest;
    firstLevelButtonDest.x = backgroundDest.x + backgroundDest.w * 0.05;
    firstLevelButtonDest.y = backgroundDest.y + 60;
    firstLevelButtonDest.w = backgroundDest.w * 0.2;
    firstLevelButtonDest.h = backgroundDest.w * 0.2;
    graphic_centerSpriteInRectButKeepRatio(levelSelector.firstLevelButton, firstLevelButtonDest);

    leftBorderDest.x = firstLevelButtonDest.x;
    leftBorderDest.y = firstLevelButtonDest.y;
    leftBorderDest.w = 2;
    leftBorderDest.h = firstLevelButtonDest.h;
    bottomBorderDest.x = firstLevelButtonDest.x;
    bottomBorderDest.y = firstLevelButtonDest.y + firstLevelButtonDest.h - 2;
    bottomBorderDest.w = firstLevelButtonDest.w;
    bottomBorderDest.h = 2;
    rightBorderDest.x = firstLevelButtonDest.x + firstLevelButtonDest.w - 2;
    rightBorderDest.y = firstLevelButtonDest.y;
    rightBorderDest.w = 2;
    rightBorderDest.h = firstLevelButtonDest.h;
    topBarDest.x = firstLevelButtonDest.x;
    topBarDest.y = firstLevelButtonDest.y;
    topBarDest.w = firstLevelButtonDest.w;
    topBarDest.h = 2;

    graphic_setSpriteDest(levelSelector.selectedLevelButton.leftBorder, leftBorderDest);
    graphic_setSpriteDest(levelSelector.selectedLevelButton.bottomBorder, bottomBorderDest);
    graphic_setSpriteDest(levelSelector.selectedLevelButton.rightBorder, rightBorderDest);
    graphic_setSpriteDest(levelSelector.selectedLevelButton.topBorder, topBarDest);

    SDL_Rect okButton;
    okButton.x = backgroundDest.x + backgroundDest.w - 80;
    okButton.y = backgroundDest.y + backgroundDest.h - 30;
    okButton.w = 40;
    okButton.h = 25;
    graphic_setSpriteDest(levelSelector.okButton, okButton);

    SDL_Rect backButton;
    backButton.x = backgroundDest.x + backgroundDest.w - 45;
    backButton.y = backgroundDest.y + backgroundDest.h - 30;
    backButton.w = 40;
    backButton.h = 25;
    graphic_setSpriteDest(levelSelector.backButton, backButton);

    graphic_setSpriteToVisible(levelSelector.background);
    graphic_setSpriteToVisible(levelSelector.leftBorder);
    graphic_setSpriteToVisible(levelSelector.bottomBorder);
    graphic_setSpriteToVisible(levelSelector.rightBorder);
    graphic_setSpriteToVisible(levelSelector.topBar);
    graphic_setSpriteToVisible(levelSelector.topText);
    graphic_setSpriteToVisible(levelSelector.firstLevelButton);
    graphic_setSpriteToVisible(levelSelector.okButton);
    graphic_setSpriteToVisible(levelSelector.backButton);
    graphic_setSpriteToVisible(levelSelector.selectedLevelButton.leftBorder);
    graphic_setSpriteToVisible(levelSelector.selectedLevelButton.bottomBorder);
    graphic_setSpriteToVisible(levelSelector.selectedLevelButton.rightBorder);
    graphic_setSpriteToVisible(levelSelector.selectedLevelButton.topBorder);
}


void
closeLevelSelector()
{
  levelSelector.opened = false;
  graphic_setSpriteToInvisible(levelSelector.background);
  graphic_setSpriteToInvisible(levelSelector.leftBorder);
  graphic_setSpriteToInvisible(levelSelector.bottomBorder);
  graphic_setSpriteToInvisible(levelSelector.rightBorder);
  graphic_setSpriteToInvisible(levelSelector.topBar);
  graphic_setSpriteToInvisible(levelSelector.topText);
  graphic_setSpriteToInvisible(levelSelector.firstLevelButton);
  graphic_setSpriteToInvisible(levelSelector.okButton);
  graphic_setSpriteToInvisible(levelSelector.backButton);
  graphic_setSpriteToInvisible(levelSelector.selectedLevelButton.leftBorder);
  graphic_setSpriteToInvisible(levelSelector.selectedLevelButton.bottomBorder);
  graphic_setSpriteToInvisible(levelSelector.selectedLevelButton.rightBorder);
  graphic_setSpriteToInvisible(levelSelector.selectedLevelButton.topBorder);
}

void
clear()
{
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

  if (levelSelector.hoveredButton.id != VOID_ID) {
    graphic_deleteSprite(levelSelector.hoveredButton.leftBorder);
    graphic_deleteSprite(levelSelector.hoveredButton.bottomBorder);
    graphic_deleteSprite(levelSelector.hoveredButton.rightBorder);
    graphic_deleteSprite(levelSelector.hoveredButton.topBorder);
  }

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
  levelSelector.hoveredButton.id = VOID_ID;
  levelSelector.hoveredButton.leftBorder = VOID_ID;
  levelSelector.hoveredButton.bottomBorder = VOID_ID;
  levelSelector.hoveredButton.rightBorder = VOID_ID;
  levelSelector.hoveredButton.topBorder = VOID_ID;
}


void
setBorderAroundHoveredButton()
{
    SDL_Rect rect;
    if (levelSelector.hoveredButton.id == levelSelector.okButton) {
        printf("bug?\n");
        graphic_querySpriteDest(levelSelector.okButton, &rect);
        printf("no-bug\n");
    } else {
        printf("bug?\n");
        graphic_querySpriteDest(levelSelector.backButton, &rect);
        printf("no-bug\n");
    }
    SDL_Rect leftBorderDest;
    leftBorderDest.x = rect.x;
    leftBorderDest.y = rect.y;
    leftBorderDest.w = 2;
    leftBorderDest.h = rect.h;
    graphic_createFullTextureSprite(greenSolidTextureId, leftBorderDest);

    SDL_Rect bottomBorderDest;
    bottomBorderDest.x = rect.x;
    bottomBorderDest.y = rect.y + rect.h - 2;
    bottomBorderDest.w = rect.w;
    bottomBorderDest.h = 2;

    SDL_Rect rightBorderDest;
    rightBorderDest.x = rect.x + rect.w - 2;
    rightBorderDest.y = rect.y;
    rightBorderDest.w = 2;
    rightBorderDest.h = rect.h;
    SDL_Rect topBorderDest;
    topBorderDest.x = rect.x;
    topBorderDest.y = rect.y;
    topBorderDest.w = rect.w;
    topBorderDest.h = 2;

    graphic_setSpriteDest(levelSelector.hoveredButton.leftBorder, leftBorderDest);
    graphic_setSpriteDest(levelSelector.hoveredButton.bottomBorder, bottomBorderDest);
    graphic_setSpriteDest(levelSelector.hoveredButton.rightBorder, rightBorderDest);
    graphic_setSpriteDest(levelSelector.hoveredButton.topBorder, topBorderDest);

    graphic_setSpriteToVisible(levelSelector.selectedLevelButton.leftBorder);
    graphic_setSpriteToVisible(levelSelector.selectedLevelButton.bottomBorder);
    graphic_setSpriteToVisible(levelSelector.selectedLevelButton.rightBorder);
    graphic_setSpriteToVisible(levelSelector.selectedLevelButton.topBorder);
}
