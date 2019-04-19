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
  int x, y;
} Customer;

static Customer _customers[MAX_CUSTOMERS];

static double _cameraDx;
static double _cameraDy;
static struct {
  int x, y, w, h, dx, dy;
} _map;

static double _zoom;
static int _tileWidth;
static int _tileHeight;

static int _dt = 0;

static void
_calculateTileWidth()
{
   _tileWidth = DEFAULT_TILE_WIDTH * _zoom;
}

static void 
_calculateTileHeight()
{
  _tileHeight = DEFAULT_TILE_HEIGHT * _zoom;
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

    if (dx > 0 && _map.x >= 0) {
      dx = 0;
    }

    if (dx < 0 && _map.x <= w - _map.w) {
      dx = 0;
    }

    if (dy > 0 && _map.y >= 0) {
      dy = 0;
    }

    if (dy < 0 && _map.y <= h - _map.h) {
      dy = 0;
    }

    if (!dx && !dy) {
      return;
    }

    _map.x += dx * _zoom;
    _map.y += dy * _zoom;
    _map.dx = _map.x + (double) (MAP_HEIGHT - 1) / 2 * (_tileWidth + _zoom * 2);
    _map.dy = _map.y;
    Graphic_TranslateAllSprite(dx * _zoom, dy * _zoom);
  }
}

static SDL_Rect
_getSrcForTile(GameTiles tile)
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
_getTileSpriteDest(SDL_Rect src, int x, int y)
{
  SDL_Rect dest;
  dest.x = x * (_tileWidth / 2 + _zoom) - y * (_tileWidth / 2 + _zoom) + _map.dx;
  dest.y = x * (_tileHeight / 2) + y * (_tileHeight / 2) + _map.dy;
  dest.w = src.w * _zoom;
  dest.h = src.h * _zoom;
  return dest;
}

static SDL_Rect
_getObjectSpriteDest(SDL_Rect src, int x, int y)
{
  SDL_Rect dest;
  dest.x = x * (_tileWidth / 2 + _zoom) - y * (_tileWidth / 2 + _zoom) + _map.dx;
  dest.y = x * _tileHeight / 2 + y * _tileHeight / 2 + _map.dy
    - (src.h - DEFAULT_TILE_HEIGHT) * _zoom;
  dest.w = src.w * _zoom;
  dest.h = src.h * _zoom;
  return dest;
}

static void 
_moveCharacters(int x, int y, int newY)
{
  _objectTiles[y][x] = GameTile_Empty;

  SDL_Rect src = _getSrcForTile(_objectTiles[newY][x]);
  SDL_Rect dest = _getObjectSpriteDest(src, x, newY);

  Graphic_SetSpriteSrcAndDest(
      _tilesObjectSpriteId[y][x], 
      src,
      dest
  );

  Graphic_SetSpriteToBeAfterAnother(
      _tilesObjectSpriteId[y][x], 
      _tilesSpriteId[newY][x]
  );

  _tilesObjectSpriteId[newY][x] = _tilesObjectSpriteId[y][x];
  _tilesObjectSpriteId[y][x] = VOID_ID;
}


static void
_resizeSprites()
{
  _calculateTileWidth();
  _calculateTileHeight();
  Graphic_ZoomSprites(_zoom);
}

static void 
_update(void)
{
  if (input_is_quit_pressed()) {
    Graphic_Clear();
    MainMenu_Enter();
  } else if (input_is_key_released(SDLK_EQUALS) && _zoom < 5) {
    printf("+\n");
    _zoom = 2;
    _resizeSprites();
  } else if (input_is_key_released(SDLK_MINUS) && _zoom > 1) {
    printf("-\n");
    _zoom = 0.5;
    _resizeSprites();
  }

  if (_dt == 10) {
    for (int y = MAP_HEIGHT; y--;) {
      int nextY = (y + 1) % MAP_HEIGHT;
      if (_objectTiles[y][46] == GameTile_WalkingCharacterDown1) {
        _objectTiles[nextY][46] = GameTile_WalkingCharacterDown2;
      } else if (_objectTiles[y][46] == GameTile_WalkingCharacterDown2) {
        _objectTiles[nextY][46] = GameTile_WalkingCharacterDown1;
      } else {
        continue;
      }
      _moveCharacters(46, y, nextY);
    }

    for (int y = 0; y < MAP_HEIGHT; y++) {
      int nextY = y - 1 >= 0 ? y - 1 : MAP_HEIGHT - 1;
      if (_objectTiles[y][47] == GameTile_WalkingCharacterUp1) {
        _objectTiles[nextY][47] = GameTile_WalkingCharacterUp2;
      } else if (_objectTiles[y][47] == GameTile_WalkingCharacterUp2) {
        _objectTiles[nextY][47] = GameTile_WalkingCharacterUp1;
      } else {
        continue;
      }
      _moveCharacters(47, y, nextY);
    }
    _dt -= 10;
  }
  _dt++;
  _handleCamera();
}

static void
_createSpriteForTile(int x, int y)
{
  SDL_Rect src = _getSrcForTile(_groundTiles[y][x]);
  SDL_Rect dest = _getTileSpriteDest(src, x, y);
  
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
  SDL_Rect src = _getSrcForTile(_objectTiles[y][x]);
  SDL_Rect dest = _getObjectSpriteDest(src, x, y);

  _tilesObjectSpriteId[y][x] = Graphic_CreateTilesetSprite(
      spriteSheetId, 
      src, 
      dest
  );
}

void 
Game_Enter(void)
{
  Scene_SetUpdateTo(_update);

  _zoom = 3;
  _calculateTileWidth();
  _calculateTileHeight();

  spriteSheetId = Graphic_LoadTexture("sprite-sheet.bmp");

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  _map.w = (double) (MAP_WIDTH + MAP_HEIGHT) / 2 * (_tileWidth + _zoom * 2);
  _map.h = (double) (MAP_WIDTH + MAP_HEIGHT) / 2 * _tileHeight;
  _map.x = (double) (w - _map.w) / 2;
  _map.y = (double) (h - _map.h) / 2;
  _map.dx = _map.x + (double) (MAP_HEIGHT - 1) / 2 * (_tileWidth + _zoom * 2);
  _map.dy = _map.y;

  for (int y = 0; y < MAP_HEIGHT; y++) 
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      _groundTiles[y][x] = GameTile_Grass;
      _objectTiles[y][x] = GameTile_Empty;
    }
  }

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 48; x < 56; x++)
    {
      _groundTiles[y][x] = GameTile_Road;
    }
  }

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    _groundTiles[y][46] = GameTile_SideWalk;
    _groundTiles[y][47] = GameTile_SideWalk;
    _groundTiles[y][56] = GameTile_SideWalk;
    _groundTiles[y][57] = GameTile_SideWalk;
  }

  _objectTiles[40][45] = GameTile_StandLeft;
  _objectTiles[39][45] = GameTile_StandCenter;
  _objectTiles[38][45] = GameTile_StandRight;

  _objectTiles[0][46] = GameTile_WalkingCharacterDown1;
  _objectTiles[99][47] = GameTile_WalkingCharacterUp1;

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      _createSpriteForTile(x, y);
      _createSpriteForTileObject(x, y);
    }
  }
}

