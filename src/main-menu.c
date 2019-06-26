#include "main-menu.h"
#include "scene.h"
#include "utils.h"
#include "graphic.h"
#include "input.h"
#include "game.h"
#include "widget.h"

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
static Id background = VOID_ID;

static Id titleWidget;

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
} levelSelector;


void 
centerMainMenu() 
{
  Graphic_CenterSpriteOnScreenWidth(mainMenuTitle);
  Graphic_CenterSpriteOnScreenWithOffset(newButton, 0, -20);
  Graphic_CenterSpriteOnScreenWithOffset(loadButton, 0, 0);
  Graphic_CenterSpriteOnScreenWithOffset(quitButton, 0, 20);
  // Graphic_ResizeSpriteToScreen(background);
}

void
openLevelSelector()
{
  levelSelector.opened = true;

  SDL_Rect backgroundDest;
  backgroundDest.x = 0;
  backgroundDest.y = 0;
  Graphic_QueryWindowSize(&backgroundDest.w, &backgroundDest.h);
  // Graphic_QuerySpriteDest(background, &backgroundDest);

  backgroundDest.x += backgroundDest.w * 0.1;
  backgroundDest.y += backgroundDest.h * 0.1;
  backgroundDest.w = backgroundDest.w * 0.8;
  backgroundDest.h = backgroundDest.h * 0.8;
  Graphic_SetSpriteDest(levelSelector.background, backgroundDest);
  
  SDL_Rect leftBorderDest;
  leftBorderDest.x = backgroundDest.x;
  leftBorderDest.y = backgroundDest.y;
  leftBorderDest.w = 2;
  leftBorderDest.h = backgroundDest.h;
  Graphic_SetSpriteDest(levelSelector.leftBorder, leftBorderDest);

  SDL_Rect bottomBorderDest;
  bottomBorderDest.x = backgroundDest.x;
  bottomBorderDest.y = backgroundDest.y + backgroundDest.h - 2;
  bottomBorderDest.w = backgroundDest.w;
  bottomBorderDest.h = 2;
  Graphic_SetSpriteDest(levelSelector.bottomBorder, bottomBorderDest);

  SDL_Rect rightBorderDest;
  rightBorderDest.x = backgroundDest.x + backgroundDest.w - 2;
  rightBorderDest.y = backgroundDest.y;
  rightBorderDest.w = 2;
  rightBorderDest.h = backgroundDest.h;
  Graphic_SetSpriteDest(levelSelector.rightBorder, rightBorderDest);

  SDL_Rect topBarDest;
  topBarDest.x = backgroundDest.x;
  topBarDest.y = backgroundDest.y;
  topBarDest.w = backgroundDest.w;
  topBarDest.h = 10;
  Graphic_SetSpriteDest(levelSelector.topBar, topBarDest);

  SDL_Rect topTextZone;
  topTextZone.x = backgroundDest.x;
  topTextZone.y = backgroundDest.y + 20;
  topTextZone.w = backgroundDest.w;
  topTextZone.h = 40;
  Graphic_CenterSpriteInRect(levelSelector.topText, topTextZone);

  SDL_Rect firstLevelButtonDest;
  firstLevelButtonDest.x = backgroundDest.x + backgroundDest.w * 0.05;
  firstLevelButtonDest.y = backgroundDest.y + 60;
  firstLevelButtonDest.w = backgroundDest.w * 0.2;
  firstLevelButtonDest.h = backgroundDest.w * 0.2;
  Graphic_CenterSpriteInRectButKeepRatio(levelSelector.firstLevelButton, firstLevelButtonDest);

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

  Graphic_SetSpriteDest(levelSelector.selectedLevelButton.leftBorder, leftBorderDest);
  Graphic_SetSpriteDest(levelSelector.selectedLevelButton.bottomBorder, bottomBorderDest);
  Graphic_SetSpriteDest(levelSelector.selectedLevelButton.rightBorder, rightBorderDest);
  Graphic_SetSpriteDest(levelSelector.selectedLevelButton.topBorder, topBarDest);

  SDL_Rect okButton;
  okButton.x = backgroundDest.x + backgroundDest.w - 80;
  okButton.y = backgroundDest.y + backgroundDest.h - 30;
  okButton.w = 40;
  okButton.h = 25;
  Graphic_SetSpriteDest(levelSelector.okButton, okButton);

  SDL_Rect backButton;
  backButton.x = backgroundDest.x + backgroundDest.w - 45;
  backButton.y = backgroundDest.y + backgroundDest.h - 30;
  backButton.w = 40;
  backButton.h = 25;
  Graphic_SetSpriteDest(levelSelector.backButton, backButton);

  Graphic_SetSpriteToActive(levelSelector.background);
  Graphic_SetSpriteToActive(levelSelector.leftBorder);
  Graphic_SetSpriteToActive(levelSelector.bottomBorder);
  Graphic_SetSpriteToActive(levelSelector.rightBorder);
  Graphic_SetSpriteToActive(levelSelector.topBar);
  Graphic_SetSpriteToActive(levelSelector.topText);
  Graphic_SetSpriteToActive(levelSelector.firstLevelButton);
  Graphic_SetSpriteToActive(levelSelector.okButton);
  Graphic_SetSpriteToActive(levelSelector.backButton);
  Graphic_SetSpriteToActive(levelSelector.selectedLevelButton.leftBorder);
  Graphic_SetSpriteToActive(levelSelector.selectedLevelButton.bottomBorder);
  Graphic_SetSpriteToActive(levelSelector.selectedLevelButton.rightBorder);
  Graphic_SetSpriteToActive(levelSelector.selectedLevelButton.topBorder);
}


void
closeLevelSelector()
{
  levelSelector.opened = false;
  Graphic_SetSpriteToInactive(levelSelector.background);
  Graphic_SetSpriteToInactive(levelSelector.leftBorder);
  Graphic_SetSpriteToInactive(levelSelector.bottomBorder);
  Graphic_SetSpriteToInactive(levelSelector.rightBorder);
  Graphic_SetSpriteToInactive(levelSelector.topBar);
  Graphic_SetSpriteToInactive(levelSelector.topText);
  Graphic_SetSpriteToInactive(levelSelector.firstLevelButton);
  Graphic_SetSpriteToInactive(levelSelector.okButton);
  Graphic_SetSpriteToInactive(levelSelector.backButton);
  Graphic_SetSpriteToInactive(levelSelector.selectedLevelButton.leftBorder);
  Graphic_SetSpriteToInactive(levelSelector.selectedLevelButton.bottomBorder);
  Graphic_SetSpriteToInactive(levelSelector.selectedLevelButton.rightBorder);
  Graphic_SetSpriteToInactive(levelSelector.selectedLevelButton.topBorder);
  Graphic_SetSpriteToInactive(levelSelector.hoveredButton.leftBorder);
  Graphic_SetSpriteToInactive(levelSelector.hoveredButton.bottomBorder);
  Graphic_SetSpriteToInactive(levelSelector.hoveredButton.rightBorder);
  Graphic_SetSpriteToInactive(levelSelector.hoveredButton.topBorder);
}

void
setBorderAroundHoveredButton()
{
  SDL_Rect rect;
  if (levelSelector.hoveredButton.id == levelSelector.okButton) 
  {
    Graphic_QuerySpriteDest(levelSelector.okButton, &rect);
  } 
  else 
  {
    Graphic_QuerySpriteDest(levelSelector.backButton, &rect);
  }

  SDL_Rect leftBorderDest;
  leftBorderDest.x = rect.x;
  leftBorderDest.y = rect.y;
  leftBorderDest.w = 2;
  leftBorderDest.h = rect.h;

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

  Graphic_SetSpriteDest(levelSelector.hoveredButton.leftBorder, leftBorderDest);
  Graphic_SetSpriteDest(levelSelector.hoveredButton.bottomBorder, bottomBorderDest);
  Graphic_SetSpriteDest(levelSelector.hoveredButton.rightBorder, rightBorderDest);
  Graphic_SetSpriteDest(levelSelector.hoveredButton.topBorder, topBorderDest);

  Graphic_SetSpriteToActive(levelSelector.hoveredButton.leftBorder);
  Graphic_SetSpriteToActive(levelSelector.hoveredButton.bottomBorder);
  Graphic_SetSpriteToActive(levelSelector.hoveredButton.rightBorder);
  Graphic_SetSpriteToActive(levelSelector.hoveredButton.topBorder);
}

static void startGame()
{
  Graphic_Clear();
  Game_Enter();
}

static void 
update() 
{
  if (Input_IsQuitPressed()) {
    Scene_Quit();
  }

  if (levelSelector.opened) {
    SDL_Rect okButtonRect, backButtonRect;
    Graphic_QuerySpriteDest(levelSelector.okButton, &okButtonRect);
    Graphic_QuerySpriteDest(levelSelector.backButton, &backButtonRect);
    if (Input_IsKeyReleased(SDLK_ESCAPE)) {
      closeLevelSelector();
    } else if (Input_IsZoneClicked(okButtonRect, LeftMouseButton) ||
               Input_IsKeyReleased(SDLK_RETURN)) {
      closeLevelSelector();
      startGame();
      return;
    } else if (Input_IsZoneClicked(backButtonRect, LeftMouseButton)) {
      closeLevelSelector();
    }

    if (Input_IsMouseOverZone(okButtonRect)) {
      if (levelSelector.hoveredButton.id != levelSelector.okButton) {
        levelSelector.hoveredButton.id = levelSelector.okButton;
        setBorderAroundHoveredButton();
      }
    } else if (Input_IsMouseOverZone(backButtonRect)) {
      if (levelSelector.hoveredButton.id != levelSelector.backButton) {
        levelSelector.hoveredButton.id = levelSelector.backButton;
        setBorderAroundHoveredButton();
      }
    } 
    else if (levelSelector.hoveredButton.id != VOID_ID) {
      levelSelector.hoveredButton.id = VOID_ID;
      Graphic_SetSpriteToInactive(levelSelector.hoveredButton.leftBorder);
      Graphic_SetSpriteToInactive(levelSelector.hoveredButton.bottomBorder);
      Graphic_SetSpriteToInactive(levelSelector.hoveredButton.rightBorder);
      Graphic_SetSpriteToInactive(levelSelector.hoveredButton.topBorder);
    }
  } else { 
    SDL_Rect newButtonRect, loadButtonRect, quitButtonRect;
    Graphic_QuerySpriteDest(newButton, &newButtonRect);
    Graphic_QuerySpriteDest(loadButton, &loadButtonRect);
    Graphic_QuerySpriteDest(quitButton, &quitButtonRect);

    if (Input_IsKeyReleased(SDLK_RETURN) || 
        Input_IsKeyReleased(SDLK_RETURN2)) {
      switch((int) selectedButton) 
      {
        case 0: openLevelSelector(); break;
        case 1: break;
        case 2: Scene_Quit(); break;
      }
    } else if (Input_IsZoneClicked(newButtonRect, LeftMouseButton)) {
      openLevelSelector();
    } else if (Input_IsZoneClicked(loadButtonRect, LeftMouseButton)) {
      // Load
    } else if (Input_IsZoneClicked(quitButtonRect, LeftMouseButton)) {
      Scene_Quit();
      return;
    } else {
      int prev = selectedButton;
      if (Input_IsKeyPressed(SDLK_DOWN)) {
        prev = selectedButton;
        selectedButton += SELECTION_SPEED;
        if (selectedButton >= 3) {
          selectedButton = 0;
        }
      } else if (Input_IsKeyPressed(SDLK_UP)) {
        selectedButton -= SELECTION_SPEED;
        if (selectedButton < 0) {
          selectedButton = 2.99;
        }
      } else if (Input_IsMouseOverZone(newButtonRect)) {
        selectedButton = 0;
      } else if (Input_IsMouseOverZone(loadButtonRect)) {
        selectedButton = 1;
      } else if (Input_IsMouseOverZone(quitButtonRect)) {
        selectedButton = 2;
      } 
      if (selectedButton != prev) {
        switch((int) selectedButton) {
          case 0:
            Graphic_SetText(newButton, ">New", 0, 0, textColor);
            Graphic_SetText(loadButton, "Load", 0, 0, textColor);
            Graphic_SetText(quitButton, "Quit", 0, 0, textColor);
            break;
          case 1:
            Graphic_SetText(newButton, "New", 0, 0, textColor);
            Graphic_SetText(loadButton, ">Load", 0, 0, textColor);
            Graphic_SetText(quitButton, "Quit", 0, 0, textColor);
            break;
          case 2:
            Graphic_SetText(newButton, "New", 0, 0, textColor);
            Graphic_SetText(loadButton, "Load", 0, 0, textColor);
            Graphic_SetText(quitButton, ">Quit", 0, 0, textColor);
            break;
        }
      }
    }
  }
  centerMainMenu();
  Game_UpdateSimulation();
}

void 
MainMenu_Enter()
{
  Graphic_InitCamera();
  levelSelector.opened = false; 
  selectedButton = 0;

  titleWidget = Widget_Create(VOID_ID);
  Widget_SetAligments(
      titleWidget, 
      Widget_HorizontalAlignCenter, 
      Widget_VerticalAlignCenter
  );
  Widget_SetPosition(titleWidget, 0, 0, 100, 100, UnitInPercentFlags_Width);
  Widget_SetText(titleWidget, "Lemon88");


  backgroundTextureId = Graphic_LoadTexture("background.png");
  okButtonTextureId = Graphic_LoadTexture("ok.png");
  backButtonTextureId = Graphic_LoadTexture("back.png");
  lightScreenSolidTextureId = Graphic_CreateSolidTexture(0xEEFFAA);
  greenSolidTextureId = Graphic_CreateSolidTexture(0x225500);

  SDL_Rect backgroundDest = {0};
  // background = Graphic_CreateFullTextureSprite(backgroundTextureId, backgroundDest);
  // Graphic_ResizeSpriteToScreen(background);

  // Graphic_CenterSpriteOnScreen(background);
  mainMenuTitle = Graphic_CreateText("Lemonade 5000", 0, 40, textColor);
  newButton = Graphic_CreateText(">New", 0, 0, textColor);
  loadButton = Graphic_CreateText("Load", 0, 0, textColor);
  quitButton = Graphic_CreateText("Quit", 0, 0, textColor);

  levelSelector.background = Graphic_CreateInactiveSprite(
    lightScreenSolidTextureId
  ); 
  levelSelector.leftBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  ); 
  levelSelector.bottomBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.rightBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.topBar = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.topText = Graphic_CreateInactiveText(
    "Select Level", 
    greenTextColor
  );
  levelSelector.firstLevelButton = Graphic_CreateInactiveSprite(
    backgroundTextureId
  );
  levelSelector.selectedLevelButton.id = levelSelector.firstLevelButton;
  levelSelector.selectedLevelButton.leftBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.selectedLevelButton.bottomBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.selectedLevelButton.rightBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.selectedLevelButton.topBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.okButton = Graphic_CreateInactiveSprite(okButtonTextureId);
  levelSelector.backButton = Graphic_CreateInactiveSprite(backButtonTextureId);

  levelSelector.hoveredButton.leftBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.hoveredButton.bottomBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.hoveredButton.rightBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );
  levelSelector.hoveredButton.topBorder = Graphic_CreateInactiveSprite(
    greenSolidTextureId
  );

  Game_StartSimulation();

  Scene_SetUpdateTo(update);
  centerMainMenu();
}
