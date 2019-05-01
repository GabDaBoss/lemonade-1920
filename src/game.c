#include <math.h>
#include "game.h"
#include "scene.h"
#include "input.h"
#include "graphic.h"
#include "main-menu.h"
#include "utils.h"

static Id spriteSheetId = VOID_ID;

typedef enum {
  GameTile_Empty,
  GameTile_Grass,
  GameTile_SideWalk,
  GameTile_Road,
  GameTile_StandLeft,
  GameTile_StandCenter,
  GameTile_StandRight,
  GameTile_StandingCharacterDown,
  GameTile_StandingCharacterRight,
  GameTile_StandingCharacterUp,
  GameTile_StandingCharacterLeft,
  GameTile_WalkingCharacterDown1,
  GameTile_WalkingCharacterRight1,
  GameTile_WalkingCharacterUp1,
  GameTile_WalkingCharacterLeft1,
  GameTile_WalkingCharacterDown2,
  GameTile_WalkingCharacterRight2,
  GameTile_WalkingCharacterUp2,
  GameTile_WalkingCharacterLeft2,
} GameTiles;

#define MAP_WIDTH 100
#define MAP_HEIGHT 100
#define DEFAULT_TILE_WIDTH 14
#define DEFAULT_TILE_HEIGHT 8
#define MAX_CUSTOMERS 10

static GameTiles _groundTiles[MAP_HEIGHT][MAP_WIDTH];
static GameTiles _objectTiles[MAP_HEIGHT][MAP_WIDTH];
static Id _tilesSpriteId[MAP_HEIGHT][MAP_WIDTH];
static Id _tilesObjectSpriteId[MAP_HEIGHT][MAP_WIDTH];

typedef struct {
  Id sprite;
  double x, y, dx, dy;
  GameTiles tile;
} Customer;

static Customer _customers[MAX_CUSTOMERS];
static int _activeCustomers;

static double _cameraDx;
static double _cameraDy;

static int _tileWidth;
static int _tileHeight;

static int _dt = 0;
static bool _pause = false;

static void
_calculateTileWidth()
{
   _tileWidth = DEFAULT_TILE_WIDTH;
}

static void 
_calculateTileHeight()
{
  _tileHeight = DEFAULT_TILE_HEIGHT;
}

static void 
_handleCamera() {
  int x, y, w, h;
  Input_QueryMousePosition(&x, &y);
  Graphic_QueryWindowSize(&w, &h);
  if (x < w * .20 || x > w * .80 || y < h * .20 || y > h * .80) 
  {
    _cameraDx += (double) (x - w / 2) / (w / 2);
    _cameraDy += (double) (y - h / 2) / (h / 2);
    int dx = 0, dy = 0;
    if (_cameraDx >= 1) {
      dx = -1;
      _cameraDx--;
    } else if (_cameraDx <= -1) {
      dx = 1;
      _cameraDx++;
    }

    if (_cameraDy >= 1) {
      dy = -1;
      _cameraDy--;
    } else if (_cameraDy <= -1) {
      dy = 1;
      _cameraDy++;
    }

    if (!dx && !dy) {
      return;
    }

    Graphic_MoveCamera(dx, dy);
  }
}

static SDL_Rect
_getTileSrc(GameTiles tile)
{
  SDL_Rect src;
  switch (tile) {
    case GameTile_Empty:
      src.x = src.y = src.w = src.h = 0;
      break;
    case GameTile_Grass:
      src.x = 0;
      src.y = 0;
      src.w = 14;
      src.h = 8;
      break;
    case GameTile_SideWalk:
      src.x = 14;
      src.y = 0;
      src.w = 14;
      src.h = 8;
      break;
    case GameTile_Road:
      src.x = 28;
      src.y = 0;
      src.w = 14;
      src.h = 8;
      break;
    case GameTile_StandLeft:
      src.x = 0;
      src.y = 8;
      src.w = 14;
      src.h = 47;
      break;
    case GameTile_StandCenter:
      src.x = 14;
      src.y = 8;
      src.w = 14;
      src.h = 47;
      break;
    case GameTile_StandRight:
      src.x = 28;
      src.y = 8;
      src.w = 14;
      src.h = 47;
      break;
    case GameTile_StandingCharacterDown:
      src.x = 0;
      src.y = 55;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_StandingCharacterRight:
      src.x = 14;
      src.y = 55;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_StandingCharacterUp:
      src.x = 28;
      src.y = 55;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_StandingCharacterLeft:
      src.x = 42;
      src.y = 55;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_WalkingCharacterDown1:
      src.x = 0;
      src.y = 95;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_WalkingCharacterRight1:
      src.x = 14;
      src.y = 95;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_WalkingCharacterUp1:
      src.x = 28;
      src.y = 95;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_WalkingCharacterLeft1:
      src.x = 42;
      src.y = 95;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_WalkingCharacterDown2:
      src.x = 0;
      src.y = 135;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_WalkingCharacterRight2:
      src.x = 14;
      src.y = 135;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_WalkingCharacterUp2:
      src.x = 28; 
      src.y = 135;
      src.w = 14;
      src.h = 40;
      break;
    case GameTile_WalkingCharacterLeft2:
      src.x = 42;
      src.y = 135;
      src.w = 14;
      src.h = 40;
      break;
  }
  return src;
}

static SDL_Rect 
_getTileDest(SDL_Rect src, int x, int y)
{
  SDL_Rect dest;
  dest.x = x * (_tileWidth / 2 + 1) - y * (_tileWidth / 2 + 1);
  dest.y = x * (_tileHeight / 2) + y * (_tileHeight / 2);
  dest.w = src.w;
  dest.h = src.h;
  return dest;
}

static SDL_Rect
_getObjectSpriteDest(SDL_Rect src, int x, int y)
{
  SDL_Rect dest;
  dest.x = x * (_tileWidth / 2 + 1) - y * (_tileWidth / 2 + 1);
  dest.y = x * _tileHeight / 2 + y * _tileHeight / 2
    - (src.h - DEFAULT_TILE_HEIGHT);
  dest.w = src.w;
  dest.h = src.h;
  return dest;
}

static void
_animateCustomers()
{
  for (int i = 0; i < _activeCustomers; i++) {
    switch (_customers[i].tile) {
      case GameTile_WalkingCharacterDown1: 
        _customers[i].tile = GameTile_WalkingCharacterDown2;
        break;
      case GameTile_WalkingCharacterDown2: 
        _customers[i].tile = GameTile_WalkingCharacterDown1;
        break;
    }
    SDL_Rect src = _getTileSrc(_customers[i].tile);
    Graphic_SetSpriteSrcRect(_customers[i].sprite, src);
  }
}

static void
_moveCustomers()
{
  for (int i = 0; i < _activeCustomers; i++) {
    double x = _customers[i].x + _customers[i].dx;
    double y = _customers[i].y + _customers[i].dy;

    if (x >= MAP_WIDTH) {
      x = -1;
    }

    if (y >= MAP_HEIGHT) {
      y = -1;
    }

    double dx = x - _customers[i].x;
    double dy = y - _customers[i].y;

    _customers[i].x = x;
    _customers[i].y = y;

    /*
    if (y > 0 && x > 0 && y < MAP_HEIGHT - 1 && x < MAP_WIDTH - 1) {
      Graphic_SetSpriteToBeAfterAnother(
        _customers[i].sprite, 
        _tilesSpriteId[(int) (y + (dy > 0 ? 1 : dy < 0 ? -1: 0))][(int) (x + dx)]
      );
    }
    */

    Graphic_TranslateSpriteFloat(
      _customers[i].sprite, 
      -dy * DEFAULT_TILE_HEIGHT,
      dy * DEFAULT_TILE_HEIGHT / 2
    );
  }
}

static void 
_update(void)
{
  double zoom = Graphic_GetCameraZoom();
  if (Input_IsQuitPressed()) {
    Graphic_Clear();
    MainMenu_Enter();
  } else if (Input_IsKeyReleased(SDLK_EQUALS) && zoom < 5) {
    Graphic_ZoomSprites((zoom + 1.0) / zoom);
  } else if (Input_IsKeyReleased(SDLK_MINUS) && zoom > 1) {
    Graphic_ZoomSprites((zoom - 1.0) / zoom);
  } else if (Input_IsKeyReleased(SDLK_SPACE)) {
    _pause = !_pause;
  }

  if (!_pause) {
    _moveCustomers();
    if (_dt == 15) {
      _dt -= 15;
      _animateCustomers();
    } else {
      _dt++;
    }
  }
  _handleCamera();
}

static void
_createSpriteForTile(int x, int y)
{
  SDL_Rect src = _getTileSrc(_groundTiles[y][x]);
  SDL_Rect dest = _getTileDest(src, x, y);
  
  _tilesSpriteId[y][x] = Graphic_CreateTilesetSprite(
      spriteSheetId, 
      src, 
      dest
  );
}

static void
_createSpriteForTileObject(int x, int y)
{
  if (_objectTiles[y][x] == GameTile_Empty) {
    return;
  }
  SDL_Rect src = _getTileSrc(_objectTiles[y][x]);
  SDL_Rect dest = _getObjectSpriteDest(src, x, y);

  _tilesObjectSpriteId[y][x] = Graphic_CreateTilesetSprite(
      spriteSheetId, 
      src, 
      dest
  );
}

static void
_createCustomerSprites()
{
  for (int i = 0; i < _activeCustomers; i++) {
    SDL_Rect src, dest;
    src = _getTileSrc(_customers[i].tile);
    dest = _getObjectSpriteDest(src, _customers[i].x, _customers[i].y);

    _customers[i].sprite = Graphic_CreateTilesetSprite(
      spriteSheetId, 
      src, 
      dest
    );
  }
}

static void
_createMapSprite()
{
  Sprite mapSprites[MAP_HEIGHT][MAP_WIDTH] = {0};
  for (int y = 0; y < MAP_HEIGHT; y++) 
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      mapSprites[y][x].src = _getTileSrc(GameTile_Grass);
      mapSprites[y][x].dest = _getTileDest(mapSprites[y][x].src, x, y);
      mapSprites[y][x].textureId = spriteSheetId;
    }
  }

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 48; x < 56; x++)
    {
      mapSprites[y][x].src = _getTileSrc(GameTile_Road);
      mapSprites[y][x].dest = _getTileDest(mapSprites[y][x].src, x, y);
    }
  }

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    mapSprites[y][46].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[y][46].dest = _getTileDest(mapSprites[y][46].src, 46, y);
    mapSprites[y][47].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[y][47].dest = _getTileDest(mapSprites[y][47].src, 47, y);
    mapSprites[y][56].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[y][56].dest = _getTileDest(mapSprites[y][56].src, 56, y);
    mapSprites[y][57].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[y][57].dest = _getTileDest(mapSprites[y][57].src, 57, y);
  }

  Graphic_CreateSpriteFromSprites((Sprite*) mapSprites, (Sprite*) mapSprites[MAP_WIDTH]);
}

void 
Game_Enter(void)
{
  Scene_SetUpdateTo(_update);
  Graphic_InitCamera();

  _calculateTileWidth();
  _calculateTileHeight();

  spriteSheetId = Graphic_LoadTexture("sprite-sheet.bmp");

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  for (int y = 0; y < MAP_HEIGHT; y++) 
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      _objectTiles[y][x] = GameTile_Empty;
    }
  }

  _objectTiles[46][45] = GameTile_StandLeft;
  _objectTiles[45][45] = GameTile_StandCenter;
  _objectTiles[44][45] = GameTile_StandRight;


  _createMapSprite();
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      //_createSpriteForTile(x, y);
      _createSpriteForTileObject(x, y);
    }
  }

  _activeCustomers = 1;
  _customers[0].x = 46;
  _customers[0].y = -1;
  _customers[0].dy = 0.05;
  _customers[0].dx = 0;
  _customers[0].tile = GameTile_WalkingCharacterDown1;
  _createCustomerSprites();
  _pause = false;
  Graphic_CenterCamera();
  _dt = 0;
}

