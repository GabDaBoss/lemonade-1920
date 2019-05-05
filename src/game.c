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
  GameTile_CrosswalkNorthSouth1,
  GameTile_CrosswalkNorthSouth2,
  GameTile_CrosswalkEastWest1,
  GameTile_CrosswalkEastWest2,
  GameTile_Road,
  GameTile_Stand,
  GameTile_StandingCharacterSouth,
  GameTile_StandingCharacterEast,
  GameTile_StandingCharacterNorth,
  GameTile_StandingCharacterWest,
  GameTile_WalkingCharacterSouth1,
  GameTile_WalkingCharacterEast1,
  GameTile_WalkingCharacterNorth1,
  GameTile_WalkingCharacterWest1,
  GameTile_WalkingCharacterSouth2,
  GameTile_WalkingCharacterEast2,
  GameTile_WalkingCharacterNorth2,
  GameTile_WalkingCharacterWest2,
} GameTiles;

#define MAP_WIDTH 100
#define MAP_HEIGHT 100
#define TILE_WIDTH 32
#define TILE_HEIGHT 16
#define MAX_CUSTOMERS 10
#define NORTH_TO_SOUTH_WEST_SIDE_LANE 46
#define SOUTH_TO_NORTH_WEST_SIDE_LANE 47
#define NORTH_TO_SOUTH_EAST_SIDE_LANE 56
#define SOUTH_TO_NORTH_EAST_SIDE_LANE 57
#define EAST_TO_WEST_NORTH_SIDE_LANE 30
#define WEST_TO_EAST_NORTH_SIDE_LANE 31
#define EAST_TO_WEST_SOUTH_SIDE_LANE 40
#define WEST_TO_EAST_SOUTH_SIDE_LANE 41

static GameTiles _groundTiles[MAP_HEIGHT][MAP_WIDTH];
static GameTiles _objectTiles[MAP_HEIGHT][MAP_WIDTH];
static Id _tilesSpriteId[MAP_HEIGHT][MAP_WIDTH];
static Id _tilesObjectSpriteId[MAP_HEIGHT][MAP_WIDTH];

typedef enum {
  SouthToNorthOnWestSide,
  NorthToSouthOnWestSide,
  SouthToNorthOnEastSide,
  NorthToSouthOnEastSide,
  SouthOnWestSideToEastOnSouthSide,
  EastOnSouthSideToSouthOnWestSide,
  SouthOnEastSideToEastOnSouthSide,
  EastOnSouthSideToSouthOnEastSide,
  NorthOnWestSideToEastOnSouthSide,
  EastOnSouthSideToNorthOnWestSide,
  NorthOnEastSideToEastOnSouthSide,
  EastOnSouthSideToNorthOnEastSide,
  SouthOnWestSideToEastOnNorthSide,
  EastOnNorthSideToSouthOnWestSide,
  SouthOnEastSideToEastOnNorthSide,
  EastOnNorthSideToSouthOnEastSide,
  NorthOnWestSideToEastOnNorthSide,
  EastOnNorthSideToNorthOnWestSide,
  NorthOnEastSideToEastOnNorthSide,
  EastOnNorthSideToNorthOnEastSide,
} Path;

typedef struct {
  Id sprite;
  double x, y, dx, dy;
  GameTiles tile;
  Path path;
} Customer;

static Customer _customers[MAX_CUSTOMERS];
static int _activeCustomers;

static double _cameraDx;
static double _cameraDy;

static int _dt = 0;
static bool _pause = false;

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
      src.y = 12 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = TILE_HEIGHT;
      break;
    case GameTile_SideWalk:
      src.x = TILE_WIDTH;
      src.y = 12 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = TILE_HEIGHT;
      break;
    case GameTile_Road:
      src.x = 2 * TILE_WIDTH;
      src.y = 12 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = TILE_HEIGHT;
      break;
    case GameTile_CrosswalkNorthSouth1:
      src.x = 3 * TILE_WIDTH;
      src.y = 12 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = TILE_HEIGHT;
      break;
    case GameTile_CrosswalkNorthSouth2:
      src.x = 4 * TILE_WIDTH;
      src.y = 12 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = TILE_HEIGHT;
      break;
    case GameTile_CrosswalkEastWest1:
      src.x = 5 * TILE_WIDTH;
      src.y = 12 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = TILE_HEIGHT;
      break;
    case GameTile_CrosswalkEastWest2:
      src.x = 6 * TILE_WIDTH;
      src.y = 12 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = TILE_HEIGHT;
      break;
    case GameTile_Stand:
      src.x = 0;
      src.y = 13 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 4 * TILE_HEIGHT;
      break;
    case GameTile_StandingCharacterSouth:
      src.x = 0;
      src.y = 9 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_StandingCharacterEast:
      src.x = 0;
      src.y = 6 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_StandingCharacterNorth:
      src.x = 0;
      src.y = 3 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_StandingCharacterWest:
      src.x = 0;
      src.y = 0;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_WalkingCharacterSouth1:
      src.x = TILE_WIDTH;
      src.y = 9 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_WalkingCharacterEast1:
      src.x = TILE_WIDTH;
      src.y = 6 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_WalkingCharacterNorth1:
      src.x = TILE_WIDTH;
      src.y = 3 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_WalkingCharacterWest1:
      src.x = TILE_WIDTH;
      src.y = 0;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_WalkingCharacterSouth2:
      src.x = 2 * TILE_WIDTH;
      src.y = 9 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_WalkingCharacterEast2:
      src.x = 2 * TILE_WIDTH;
      src.y = 6 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_WalkingCharacterNorth2:
      src.x = 2 * TILE_WIDTH;
      src.y = 3 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_WalkingCharacterWest2:
      src.x = 2 * TILE_WIDTH;
      src.y = 0;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
  }
  return src;
}

static SDL_Rect 
_getTileDest(SDL_Rect src, int x, int y)
{
  SDL_Rect dest;
  dest.x = x * (TILE_WIDTH / 2) - y * (TILE_WIDTH / 2);
  dest.y = x * (TILE_HEIGHT / 2) + y * (TILE_HEIGHT / 2);
  dest.w = src.w;
  dest.h = src.h;
  return dest;
}

static SDL_Rect
_getObjectSpriteDest(SDL_Rect src, int x, int y)
{
  SDL_Rect dest;
  dest.x = x * (TILE_WIDTH / 2) - y * (TILE_WIDTH / 2);
  dest.y = x * TILE_HEIGHT / 2 + y * TILE_HEIGHT / 2
    - (src.h - TILE_HEIGHT);
  dest.w = src.w;
  dest.h = src.h;
  return dest;
}

static void
_animateCustomers()
{
  for (int i = 0; i < _activeCustomers; i++) {
    switch (_customers[i].tile) {
      case GameTile_WalkingCharacterSouth1: 
        _customers[i].tile = GameTile_WalkingCharacterSouth2;
        break;
      case GameTile_WalkingCharacterSouth2: 
        _customers[i].tile = GameTile_WalkingCharacterSouth1;
        break;
      case GameTile_WalkingCharacterNorth1: 
        _customers[i].tile = GameTile_WalkingCharacterNorth2;
        break;
      case GameTile_WalkingCharacterNorth2: 
        _customers[i].tile = GameTile_WalkingCharacterNorth1;
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
    switch(_customers[i].path) {
      case SouthToNorthOnWestSide:
      case SouthToNorthOnEastSide:
      case NorthToSouthOnWestSide:
      case NorthToSouthOnEastSide:
        break;
      case EastOnNorthSideToNorthOnWestSide:
        if ((_customers[i].x) <= SOUTH_TO_NORTH_WEST_SIDE_LANE) {
          _customers[i].dy = -0.05;
          _customers[i].dx = 0;
        } else {
          _customers[i].dy = 0;
          _customers[i].dx = -0.05;
        }
      case EastOnNorthSideToNorthOnEastSide:
      case EastOnSouthSideToNorthOnWestSide:
      case EastOnSouthSideToNorthOnEastSide:
      case EastOnSouthSideToSouthOnWestSide:
      case EastOnSouthSideToSouthOnEastSide:
      case EastOnNorthSideToSouthOnWestSide:
      case EastOnNorthSideToSouthOnEastSide:
      case SouthOnWestSideToEastOnSouthSide:
      case SouthOnWestSideToEastOnNorthSide:
      case SouthOnEastSideToEastOnSouthSide:
      case SouthOnEastSideToEastOnNorthSide:
      case NorthOnWestSideToEastOnSouthSide:
      case NorthOnWestSideToEastOnNorthSide:
      case NorthOnEastSideToEastOnSouthSide:
      case NorthOnEastSideToEastOnNorthSide:
        break;
    }
    double x = _customers[i].x + _customers[i].dx;
    double y = _customers[i].y + _customers[i].dy;

    switch(_customers[i].path) {
      case SouthToNorthOnWestSide:
      case SouthToNorthOnEastSide:
        if (y < 0) {
          y = MAP_HEIGHT;
        }
        break;
      case EastOnNorthSideToNorthOnWestSide:
      case EastOnNorthSideToNorthOnEastSide:
        if (y < 0) {
          x = MAP_WIDTH;
          y = EAST_TO_WEST_NORTH_SIDE_LANE;
        }
        break;
      case EastOnSouthSideToNorthOnWestSide:
      case EastOnSouthSideToNorthOnEastSide:
        if (y < 0) {
          x = MAP_WIDTH;
          y = EAST_TO_WEST_SOUTH_SIDE_LANE;
        }
        break;
      case NorthToSouthOnWestSide:
      case NorthToSouthOnEastSide:
        if (y >= MAP_HEIGHT) {
          y = 0;
        }
        break;
      case EastOnSouthSideToSouthOnWestSide:
      case EastOnSouthSideToSouthOnEastSide:
        if (y >= MAP_HEIGHT) {
          x = MAP_WIDTH;
          y = EAST_TO_WEST_SOUTH_SIDE_LANE;
        }
        break;
      case EastOnNorthSideToSouthOnWestSide:
      case EastOnNorthSideToSouthOnEastSide:
        if (y >= MAP_HEIGHT) {
          x = MAP_WIDTH;
          y = EAST_TO_WEST_NORTH_SIDE_LANE;
        }
        break;
      case SouthOnWestSideToEastOnSouthSide:
      case SouthOnWestSideToEastOnNorthSide:
        if (x >= MAP_WIDTH) {
          x = SOUTH_TO_NORTH_WEST_SIDE_LANE;
          y = MAP_HEIGHT;
        }
        break;
      case SouthOnEastSideToEastOnSouthSide:
      case SouthOnEastSideToEastOnNorthSide:
        if (x >= MAP_WIDTH) {
          x = SOUTH_TO_NORTH_EAST_SIDE_LANE;
          y = MAP_HEIGHT;
        }
        break;
      case NorthOnWestSideToEastOnSouthSide:
      case NorthOnWestSideToEastOnNorthSide:
        if (x >= MAP_WIDTH) {
          x = NORTH_TO_SOUTH_WEST_SIDE_LANE;
          y = 0;
        }
        break;
      case NorthOnEastSideToEastOnSouthSide:
      case NorthOnEastSideToEastOnNorthSide:
        if (x >= MAP_WIDTH) {
          x = NORTH_TO_SOUTH_EAST_SIDE_LANE;
          y = 0;
        }
        break;
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
      -dy * TILE_HEIGHT + dx * TILE_HEIGHT,
      dy * TILE_HEIGHT / 2 + dx * TILE_HEIGHT / 2
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
_createCustomer(Path path, int i)
{
  _customers[i].path = path;
  _customers[i].dy = 0;
  _customers[i].dx = 0;
  switch(path) {
    case SouthToNorthOnWestSide:
    case SouthOnWestSideToEastOnSouthSide:
    case SouthOnWestSideToEastOnNorthSide:
    case SouthToNorthOnEastSide:
    case SouthOnEastSideToEastOnSouthSide:
    case SouthOnEastSideToEastOnNorthSide:
      _customers[i].tile = GameTile_WalkingCharacterNorth1;
      _customers[i].dy = -0.05;
      _customers[i].y = MAP_HEIGHT;
      break;
    case NorthToSouthOnWestSide:
    case NorthToSouthOnEastSide:
    case NorthOnWestSideToEastOnSouthSide:
    case NorthOnEastSideToEastOnSouthSide:
    case NorthOnWestSideToEastOnNorthSide:
    case NorthOnEastSideToEastOnNorthSide:
      _customers[i].tile = GameTile_WalkingCharacterSouth1;
      _customers[i].dy = 0.05;
      _customers[i].y = 0;
      break;
    case EastOnSouthSideToSouthOnWestSide:
    case EastOnSouthSideToSouthOnEastSide:
    case EastOnSouthSideToNorthOnWestSide:
    case EastOnSouthSideToNorthOnEastSide:
    case EastOnNorthSideToSouthOnWestSide:
    case EastOnNorthSideToSouthOnEastSide:
    case EastOnNorthSideToNorthOnWestSide:
    case EastOnNorthSideToNorthOnEastSide:
      _customers[i].tile = GameTile_WalkingCharacterWest1;
      _customers[i].dx = -0.05;
      _customers[i].x = MAP_WIDTH;
      break;
  }

  switch(path) {
    case SouthToNorthOnWestSide:
    case SouthOnWestSideToEastOnSouthSide:
    case SouthOnWestSideToEastOnNorthSide:
      _customers[i].x = SOUTH_TO_NORTH_WEST_SIDE_LANE;
      break;
    case SouthToNorthOnEastSide:
    case SouthOnEastSideToEastOnSouthSide:
    case SouthOnEastSideToEastOnNorthSide:
      _customers[i].x = SOUTH_TO_NORTH_EAST_SIDE_LANE;
      break;
    case NorthToSouthOnWestSide:
    case NorthOnWestSideToEastOnSouthSide:
    case NorthOnWestSideToEastOnNorthSide:
      _customers[i].x = NORTH_TO_SOUTH_WEST_SIDE_LANE;
      break;
    case NorthOnEastSideToEastOnSouthSide:
    case NorthToSouthOnEastSide:
    case NorthOnEastSideToEastOnNorthSide:
      _customers[i].x = NORTH_TO_SOUTH_EAST_SIDE_LANE;
      break;
    case EastOnSouthSideToSouthOnWestSide:
    case EastOnSouthSideToSouthOnEastSide:
    case EastOnSouthSideToNorthOnWestSide:
    case EastOnSouthSideToNorthOnEastSide:
      _customers[i].y = EAST_TO_WEST_SOUTH_SIDE_LANE;
      break;
    case EastOnNorthSideToSouthOnWestSide:
    case EastOnNorthSideToSouthOnEastSide:
    case EastOnNorthSideToNorthOnWestSide:
    case EastOnNorthSideToNorthOnEastSide:
      _customers[i].y = EAST_TO_WEST_NORTH_SIDE_LANE;
      break;
  }
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
      mapSprites[y][x].textureId = spriteSheetId;
    }
  }

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = SOUTH_TO_NORTH_WEST_SIDE_LANE + 1; x < NORTH_TO_SOUTH_EAST_SIDE_LANE; x++)
    {
      mapSprites[y][x].src = _getTileSrc(GameTile_Road);
    }
  }

  for (int y = 0; y < MAP_HEIGHT; y++) {
    mapSprites[y][NORTH_TO_SOUTH_WEST_SIDE_LANE].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[y][SOUTH_TO_NORTH_WEST_SIDE_LANE].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[y][NORTH_TO_SOUTH_EAST_SIDE_LANE].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[y][SOUTH_TO_NORTH_EAST_SIDE_LANE].src = _getTileSrc(GameTile_SideWalk);
  }

  for (int y = WEST_TO_EAST_NORTH_SIDE_LANE + 1; y < EAST_TO_WEST_SOUTH_SIDE_LANE; y++) {
    mapSprites[y][NORTH_TO_SOUTH_EAST_SIDE_LANE].src = _getTileSrc(GameTile_CrosswalkNorthSouth1);
    mapSprites[y][SOUTH_TO_NORTH_EAST_SIDE_LANE].src = _getTileSrc(GameTile_CrosswalkNorthSouth2);
  }

  for (int x = SOUTH_TO_NORTH_WEST_SIDE_LANE + 1; x < NORTH_TO_SOUTH_EAST_SIDE_LANE; x++)
  {
    mapSprites[EAST_TO_WEST_NORTH_SIDE_LANE][x].src = _getTileSrc(GameTile_CrosswalkEastWest2);
    mapSprites[WEST_TO_EAST_NORTH_SIDE_LANE][x].src = _getTileSrc(GameTile_CrosswalkEastWest1);
    mapSprites[EAST_TO_WEST_SOUTH_SIDE_LANE][x].src = _getTileSrc(GameTile_CrosswalkEastWest2);
    mapSprites[WEST_TO_EAST_SOUTH_SIDE_LANE][x].src = _getTileSrc(GameTile_CrosswalkEastWest1);
  }

  for (int x = SOUTH_TO_NORTH_EAST_SIDE_LANE + 1; x < MAP_WIDTH; x++) {
    for (int y = WEST_TO_EAST_NORTH_SIDE_LANE + 1; y < EAST_TO_WEST_SOUTH_SIDE_LANE; y++) {
      mapSprites[y][x].src = _getTileSrc(GameTile_Road);
    }
    mapSprites[EAST_TO_WEST_NORTH_SIDE_LANE][x].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[WEST_TO_EAST_NORTH_SIDE_LANE][x].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[EAST_TO_WEST_SOUTH_SIDE_LANE][x].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[WEST_TO_EAST_SOUTH_SIDE_LANE][x].src = _getTileSrc(GameTile_SideWalk);
  }

  for (int x = 0; x < MAP_WIDTH; x++) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
      mapSprites[y][x].dest = _getTileDest(mapSprites[y][x].src, x, y);
    }
  }

  Graphic_CreateSpriteFromSprites(
      (Sprite*) mapSprites, 
      (Sprite*) mapSprites[MAP_WIDTH]);
}

void 
Game_Enter(void)
{
  Scene_SetUpdateTo(_update);
  Graphic_InitCamera();

  spriteSheetId = Graphic_LoadTexture("sprite-sheet2.bmp");

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  for (int y = 0; y < MAP_HEIGHT; y++) 
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      _objectTiles[y][x] = GameTile_Empty;
    }
  }

  _objectTiles[45][45] = GameTile_Stand;


  _createMapSprite();
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      //_createSpriteForTile(x, y);
      _createSpriteForTileObject(x, y);
    }
  }

  _activeCustomers = 0;
  _createCustomer(NorthToSouthOnWestSide, _activeCustomers++);
  _createCustomer(SouthToNorthOnWestSide, _activeCustomers++);
  _createCustomer(NorthToSouthOnEastSide, _activeCustomers++);
  _createCustomer(SouthToNorthOnEastSide, _activeCustomers++);
  _createCustomer(EastOnNorthSideToNorthOnWestSide, _activeCustomers++);
  _createCustomerSprites();
  _pause = false;
  Graphic_CenterCamera();
  _dt = 0;
}

