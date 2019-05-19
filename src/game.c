#include <math.h>
#include "game.h"
#include "scene.h"
#include "input.h"
#include "graphic.h"
#include "main-menu.h"
#include "utils.h"

static Id _spriteSheetId = VOID_ID;

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
  GameTile_StopSignFacingEast,
  GameTile_StopSignFacingSouth,
  GameTile_Bush,
  GameTile_LeftHouseCorner,
  GameTile_HouseDoor,
  GameTile_Wall,
  GameTile_RightHouseCorner,
  GameTile_HouseRightWallFirstSection,
  GameTile_HouseRightWallCenterSection,
  GameTile_HouseRightWallThirdSection,
  GameTile_HouseRightWallLastSection,
  GameTile_HouseRoof,
  GameTile_HouseLeftRoof,
  GameTile_HouseTopRoof,
  GameTile_HouseTopLeftRoof
} GameTiles;

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define TILE_WIDTH 32
#define TILE_HEIGHT 16
#define MAX_GAME_OBJECTS 100
#define NORTH_TO_SOUTH_WEST_SIDE_LANE 13
#define SOUTH_TO_NORTH_WEST_SIDE_LANE 14
#define NORTH_TO_SOUTH_EAST_SIDE_LANE 23
#define SOUTH_TO_NORTH_EAST_SIDE_LANE 24
#define EAST_TO_WEST_NORTH_SIDE_LANE 10
#define WEST_TO_EAST_NORTH_SIDE_LANE 11
#define EAST_TO_WEST_SOUTH_SIDE_LANE 20
#define WEST_TO_EAST_SOUTH_SIDE_LANE 21
#define STAND_X 12
#define STAND_Y 12

static GameTiles _groundTiles[MAP_HEIGHT][MAP_WIDTH];
static GameTiles _objectTiles[MAP_HEIGHT][MAP_WIDTH];
static Id _tilesSpriteId[MAP_HEIGHT][MAP_WIDTH];
static Id _tilesObjectSpriteId[MAP_HEIGHT][MAP_WIDTH];

typedef enum {
  NoPath,
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
  double x, y, z, dx, dy, dz;
  GameTiles tile;
  Path path;
} GameObject;

static GameObject _gameObjects[MAX_GAME_OBJECTS];
static int _activeGameObjects;

static double _cameraDx;
static double _cameraDy;

static int _dt = 0;
static bool _pause = false;

static void 
_reorderGameObjects()
{
  double k = 0, l = 0, nk, nl;
  int next = 0;
  GameObject tmp;
  bool dirty = false;
  for (int i = 0; i < _activeGameObjects - 1; i++) {
    k = _gameObjects[i].x * MAP_WIDTH + _gameObjects[i].y;
    l = _gameObjects[i].y * MAP_HEIGHT + _gameObjects[i].x;
    next = i;
    for (int j = i + 1; j < _activeGameObjects; j++) {
      nk = _gameObjects[j].x * MAP_WIDTH + _gameObjects[j].y;
      nl = _gameObjects[j].y * MAP_HEIGHT + _gameObjects[j].x;
      if (nk < k && nl < l) {
        next = j;
        k = nk;
        l = nl;
      }
    }
    if (next != i) {
      tmp = _gameObjects[i];
      _gameObjects[i] = _gameObjects[next];
      _gameObjects[next] = tmp;
      dirty = true;
    }
  }

  if (dirty) {
    for (int i = 0; i < _activeGameObjects - 1; i++) {
      Graphic_SetSpriteToBeAfterAnother(
        _gameObjects[i + 1].sprite, 
        _gameObjects[i].sprite
      );
    }
  }
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
    case GameTile_StopSignFacingEast:
      src.x = TILE_WIDTH;
      src.y = 13 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 4 * TILE_HEIGHT;
      break;
    case GameTile_StopSignFacingSouth:
      src.x = 2 * TILE_WIDTH;
      src.y = 13 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 4 * TILE_HEIGHT;
      break;
    case GameTile_Bush:
      src.x = 0;
      src.y = 17 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 2 * TILE_HEIGHT;
      break;
    case GameTile_LeftHouseCorner:
      src.x = 5 * TILE_WIDTH;
      src.y = 27 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 7 * TILE_HEIGHT;
      break;
    case GameTile_HouseDoor:
      src.x = 6 * TILE_WIDTH;
      src.y = 27 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 7 * TILE_HEIGHT;
      break;
    case GameTile_Wall:
      src.x = 7 * TILE_WIDTH;
      src.y = 27 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 7 * TILE_HEIGHT;
      break;
    case GameTile_RightHouseCorner:
      src.x = 8 * TILE_WIDTH;
      src.y = 27 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 7 * TILE_HEIGHT;
      break;
    case GameTile_HouseRightWallFirstSection:
      src.x = 4 * TILE_WIDTH;
      src.y = 26 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 8 * TILE_HEIGHT;
      break;
    case GameTile_HouseRightWallCenterSection:
      src.x = 3 * TILE_WIDTH;
      src.y = 25 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 9 * TILE_HEIGHT;
      break;
    case GameTile_HouseRightWallThirdSection:
      src.x = 2 * TILE_WIDTH;
      src.y = 27 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 7 * TILE_HEIGHT;
      break;
    case GameTile_HouseRightWallLastSection:
      src.x = 1 * TILE_WIDTH;
      src.y = 28 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 6 * TILE_HEIGHT;
      break;
    case GameTile_HouseRoof:
      src.x = 0 * TILE_WIDTH;
      src.y = 28 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_HouseLeftRoof:
      src.x = 0 * TILE_WIDTH;
      src.y = 31 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 3 * TILE_HEIGHT;
      break;
    case GameTile_HouseTopRoof:
      src.x = 0 * TILE_WIDTH;
      src.y = 24 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 2 * TILE_HEIGHT;
      break;
    case GameTile_HouseTopLeftRoof:
      src.x = 0 * TILE_WIDTH;
      src.y = 26 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 2 * TILE_HEIGHT;
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
_getObjectSpriteDest(SDL_Rect src, int x, int y, int z)
{
  SDL_Rect dest;
  dest.x = x * (TILE_WIDTH / 2) - y * (TILE_WIDTH / 2);
  dest.y = x * TILE_HEIGHT / 2 + y * TILE_HEIGHT / 2
    - (src.h - TILE_HEIGHT) - z;
  dest.w = src.w;
  dest.h = src.h;
  return dest;
}

static void
_animateGameObjects()
{
  for (int i = 0; i < _activeGameObjects; i++) {
    switch (_gameObjects[i].tile) {
      case GameTile_WalkingCharacterSouth1: 
        _gameObjects[i].tile = GameTile_WalkingCharacterSouth2;
        break;
      case GameTile_WalkingCharacterSouth2: 
        _gameObjects[i].tile = GameTile_WalkingCharacterSouth1;
        break;
      case GameTile_WalkingCharacterNorth1: 
        _gameObjects[i].tile = GameTile_WalkingCharacterNorth2;
        break;
      case GameTile_WalkingCharacterNorth2: 
        _gameObjects[i].tile = GameTile_WalkingCharacterNorth1;
        break;
      case GameTile_WalkingCharacterEast1: 
        _gameObjects[i].tile = GameTile_WalkingCharacterEast2;
        break;
      case GameTile_WalkingCharacterEast2: 
        _gameObjects[i].tile = GameTile_WalkingCharacterEast1;
        break;
      case GameTile_WalkingCharacterWest1: 
        _gameObjects[i].tile = GameTile_WalkingCharacterWest2;
        break;
      case GameTile_WalkingCharacterWest2: 
        _gameObjects[i].tile = GameTile_WalkingCharacterWest1;
        break;
    }
    SDL_Rect src = _getTileSrc(_gameObjects[i].tile);
    Graphic_SetSpriteSrcRect(_gameObjects[i].sprite, src);
  }
}

static void
_animateGameObject(int i, GameTiles frame1, GameTiles frame2)
{
  if (_gameObjects[i].tile != frame1 && _gameObjects[i].tile != frame2)
  {
    _gameObjects[i].tile = frame1;
  }
}

static void
_moveGameObjects()
{
  for (int i = 0; i < _activeGameObjects; i++) {
    switch(_gameObjects[i].path) {
      case SouthToNorthOnWestSide:
      case SouthToNorthOnEastSide:
      case NorthToSouthOnWestSide:
      case NorthToSouthOnEastSide:
        break;
      case EastOnSouthSideToNorthOnWestSide:
      case EastOnNorthSideToNorthOnWestSide:
        if ((_gameObjects[i].x) <= SOUTH_TO_NORTH_WEST_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterNorth1, 
            GameTile_WalkingCharacterNorth2
          );
          _gameObjects[i].dy = -0.05;
          _gameObjects[i].dx = 0;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterWest1, 
            GameTile_WalkingCharacterWest2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = -0.05;
        }
        break;
      case EastOnSouthSideToNorthOnEastSide:
      case EastOnNorthSideToNorthOnEastSide:
        if ((_gameObjects[i].x) <= SOUTH_TO_NORTH_EAST_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterNorth1, 
            GameTile_WalkingCharacterNorth2
          );
          _gameObjects[i].dy = -0.05;
          _gameObjects[i].dx = 0;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterWest1, 
            GameTile_WalkingCharacterWest2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = -0.05;
        }
        break;
      case EastOnNorthSideToSouthOnWestSide:
      case EastOnSouthSideToSouthOnWestSide:
        if ((_gameObjects[i].x) <= NORTH_TO_SOUTH_WEST_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterSouth1, 
            GameTile_WalkingCharacterSouth2
          );
          _gameObjects[i].dy = 0.05;
          _gameObjects[i].dx = 0;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterWest1, 
            GameTile_WalkingCharacterWest2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = -0.05;
        }
        break;
      case EastOnSouthSideToSouthOnEastSide:
      case EastOnNorthSideToSouthOnEastSide:
        if (_gameObjects[i].x <= NORTH_TO_SOUTH_EAST_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterSouth1, 
            GameTile_WalkingCharacterSouth2
          );
          _gameObjects[i].dy = 0.05;
          _gameObjects[i].dx = 0;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterWest1, 
            GameTile_WalkingCharacterWest2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = -0.05;
        }
        break;
      case SouthOnEastSideToEastOnSouthSide:
      case SouthOnWestSideToEastOnSouthSide:
        if ((_gameObjects[i].y) <= WEST_TO_EAST_SOUTH_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterEast1, 
            GameTile_WalkingCharacterEast2
          );
          _gameObjects[i].dx = 0.05;
          _gameObjects[i].dy = 0;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterNorth1, 
            GameTile_WalkingCharacterNorth2
          );
          _gameObjects[i].dx = 0;
          _gameObjects[i].dy = -0.05;
        }
        break;
      case SouthOnWestSideToEastOnNorthSide:
      case SouthOnEastSideToEastOnNorthSide:
        if ((_gameObjects[i].y) <= WEST_TO_EAST_NORTH_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterEast1, 
            GameTile_WalkingCharacterEast2
          );
          _gameObjects[i].dx = 0.05;
          _gameObjects[i].dy = 0;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterNorth1, 
            GameTile_WalkingCharacterNorth2
          );
          _gameObjects[i].dx = 0;
          _gameObjects[i].dy = -0.05;
        }
        break;
      case NorthOnWestSideToEastOnSouthSide:
      case NorthOnEastSideToEastOnSouthSide:
        if ((_gameObjects[i].y) >= WEST_TO_EAST_SOUTH_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterEast1, 
            GameTile_WalkingCharacterEast2
          );
          _gameObjects[i].dx = 0.05;
          _gameObjects[i].dy = 0;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterSouth1, 
            GameTile_WalkingCharacterSouth2
          );
          _gameObjects[i].dx = 0;
          _gameObjects[i].dy = 0.05;
        }
        break;
      case NorthOnWestSideToEastOnNorthSide:
      case NorthOnEastSideToEastOnNorthSide:
        if ((_gameObjects[i].y) >= WEST_TO_EAST_NORTH_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterEast1, 
            GameTile_WalkingCharacterEast2
          );
          _gameObjects[i].dx = 0.05;
          _gameObjects[i].dy = 0;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterSouth1, 
            GameTile_WalkingCharacterSouth2
          );
          _gameObjects[i].dx = 0;
          _gameObjects[i].dy = 0.05;
        }
        break;
    }
    double x = _gameObjects[i].x + _gameObjects[i].dx;
    double y = _gameObjects[i].y + _gameObjects[i].dy;

    switch(_gameObjects[i].path) {
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


    double dx = x - _gameObjects[i].x;
    double dy = y - _gameObjects[i].y;

    _gameObjects[i].x = x;
    _gameObjects[i].y = y;

    /*
    if (y > 0 && x > 0 && y < MAP_HEIGHT - 1 && x < MAP_WIDTH - 1) {
      Graphic_SetSpriteToBeAfterAnother(
        _gameObjects[i].sprite, 
        _tilesSpriteId[(int) (y + (dy > 0 ? 1 : dy < 0 ? -1: 0))][(int) (x + dx)]
      );
    }
    */

    Graphic_TranslateSpriteFloat(
      _gameObjects[i].sprite, 
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
    return;
  } else if (Input_IsKeyReleased(SDLK_EQUALS) && zoom < 5) {
    Graphic_ZoomSprites((zoom + 1.0) / zoom);
  } else if (Input_IsKeyReleased(SDLK_MINUS) && zoom > 1) {
    Graphic_ZoomSprites((zoom - 1.0) / zoom);
  } else if (Input_IsKeyReleased(SDLK_SPACE)) {
    _pause = !_pause;
  }

  if (!_pause) {
    _moveGameObjects();
    if (_dt == 15) {
      _dt -= 15;
      _animateGameObjects();
    } else {
      _dt++;
    }
    _reorderGameObjects();
  }
  _handleCamera();
}

static void
_createSpriteForTile(int x, int y)
{
  SDL_Rect src = _getTileSrc(_groundTiles[y][x]);
  SDL_Rect dest = _getTileDest(src, x, y);
  
  _tilesSpriteId[y][x] = Graphic_CreateTilesetSprite(
      _spriteSheetId, 
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
  SDL_Rect dest = _getObjectSpriteDest(src, x, y, 0);

  _tilesObjectSpriteId[y][x] = Graphic_CreateTilesetSprite(
      _spriteSheetId, 
      src, 
      dest
  );
}

static void
_createGameObject(
    GameTiles tile, 
    double x, 
    double y, 
    double z,
    double dx, 
    double dy, 
    double dz,
    int i)
{
  SDL_Rect src = _getTileSrc(tile);
  SDL_Rect dest = _getObjectSpriteDest(src, x, y, z);
  _gameObjects[i].tile = tile;
  _gameObjects[i].x = x;
  _gameObjects[i].y = y;
  _gameObjects[i].z = z;
  _gameObjects[i].dx = dx;
  _gameObjects[i].dy = dy;
  _gameObjects[i].dz = dz;
  _gameObjects[i].sprite = Graphic_CreateTilesetSprite(
      _spriteSheetId, 
      src, 
      dest
  );
  _gameObjects[i].path = NoPath;
}

static void
_createCustomer(Path path, int i)
{
  _gameObjects[i].path = path;
  _gameObjects[i].dy = 0;
  _gameObjects[i].dx = 0;
  _gameObjects[i].z = 0;
  _gameObjects[i].dz = 0;
  switch(path) {
    case SouthToNorthOnWestSide:
    case SouthOnWestSideToEastOnSouthSide:
    case SouthOnWestSideToEastOnNorthSide:
    case SouthToNorthOnEastSide:
    case SouthOnEastSideToEastOnSouthSide:
    case SouthOnEastSideToEastOnNorthSide:
      _gameObjects[i].tile = GameTile_WalkingCharacterNorth1;
      _gameObjects[i].dy = -0.05;
      _gameObjects[i].y = MAP_HEIGHT;
      break;
    case NorthToSouthOnWestSide:
    case NorthToSouthOnEastSide:
    case NorthOnWestSideToEastOnSouthSide:
    case NorthOnEastSideToEastOnSouthSide:
    case NorthOnWestSideToEastOnNorthSide:
    case NorthOnEastSideToEastOnNorthSide:
      _gameObjects[i].tile = GameTile_WalkingCharacterSouth1;
      _gameObjects[i].dy = 0.05;
      _gameObjects[i].y = 0;
      break;
    case EastOnSouthSideToSouthOnWestSide:
    case EastOnSouthSideToSouthOnEastSide:
    case EastOnSouthSideToNorthOnWestSide:
    case EastOnSouthSideToNorthOnEastSide:
    case EastOnNorthSideToSouthOnWestSide:
    case EastOnNorthSideToSouthOnEastSide:
    case EastOnNorthSideToNorthOnWestSide:
    case EastOnNorthSideToNorthOnEastSide:
      _gameObjects[i].tile = GameTile_WalkingCharacterWest1;
      _gameObjects[i].dx = -0.05;
      _gameObjects[i].x = MAP_WIDTH;
      break;
  }

  switch(path) {
    case SouthToNorthOnWestSide:
    case SouthOnWestSideToEastOnSouthSide:
    case SouthOnWestSideToEastOnNorthSide:
      _gameObjects[i].x = SOUTH_TO_NORTH_WEST_SIDE_LANE;
      break;
    case SouthToNorthOnEastSide:
    case SouthOnEastSideToEastOnSouthSide:
    case SouthOnEastSideToEastOnNorthSide:
      _gameObjects[i].x = SOUTH_TO_NORTH_EAST_SIDE_LANE;
      break;
    case NorthToSouthOnWestSide:
    case NorthOnWestSideToEastOnSouthSide:
    case NorthOnWestSideToEastOnNorthSide:
      _gameObjects[i].x = NORTH_TO_SOUTH_WEST_SIDE_LANE;
      break;
    case NorthOnEastSideToEastOnSouthSide:
    case NorthToSouthOnEastSide:
    case NorthOnEastSideToEastOnNorthSide:
      _gameObjects[i].x = NORTH_TO_SOUTH_EAST_SIDE_LANE;
      break;
    case EastOnSouthSideToSouthOnWestSide:
    case EastOnSouthSideToSouthOnEastSide:
    case EastOnSouthSideToNorthOnWestSide:
    case EastOnSouthSideToNorthOnEastSide:
      _gameObjects[i].y = EAST_TO_WEST_SOUTH_SIDE_LANE;
      break;
    case EastOnNorthSideToSouthOnWestSide:
    case EastOnNorthSideToSouthOnEastSide:
    case EastOnNorthSideToNorthOnWestSide:
    case EastOnNorthSideToNorthOnEastSide:
      _gameObjects[i].y = EAST_TO_WEST_NORTH_SIDE_LANE;
      break;
  }
}

static void
_createCustomerSprites()
{
  for (int i = 0; i < _activeGameObjects; i++) {
    SDL_Rect src, dest;
    src = _getTileSrc(_gameObjects[i].tile);
    dest = _getObjectSpriteDest(src, _gameObjects[i].x, _gameObjects[i].y, _gameObjects[i].z);

    _gameObjects[i].sprite = Graphic_CreateTilesetSprite(
      _spriteSheetId, 
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
      mapSprites[y][x].textureId = _spriteSheetId;
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

static void
_createHouse(int x, int y)
{
  _createGameObject(GameTile_LeftHouseCorner, x, y, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Wall, x + 1, y, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseDoor, x + 2, y, 0, 0, 0, 0,  _activeGameObjects++);
  _createGameObject(GameTile_Wall, x + 3, y, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Wall, x + 4, y, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Wall, x + 5, y, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Wall, x + 6, y, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_RightHouseCorner, x + 7, y, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRightWallFirstSection, x + 7, y - 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRightWallCenterSection, x + 7, y - 2, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRightWallThirdSection, x + 7, y - 3, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRightWallLastSection, x + 7, y - 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseTopRoof, x + 6, y - 2, 7 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseTopRoof, x + 5, y - 2, 7 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseTopRoof, x + 4, y - 2, 7 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseTopRoof, x + 3, y - 2, 7 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseTopRoof, x + 2, y - 2, 7 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseTopRoof, x + 1, y - 2, 7 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseTopLeftRoof, x, y - 2, 7 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseLeftRoof, x, y - 1, 5.5 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRoof, x + 6, y - 1, 5.5 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRoof, x + 5, y - 1, 5.5 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRoof, x + 4, y - 1, 5.5 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRoof, x + 3, y - 1, 5.5 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRoof, x + 2, y - 1, 5.5 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_HouseRoof, x + 1, y - 1, 5.5 * TILE_HEIGHT, 0, 0, 0, _activeGameObjects++);
}

void 
Game_Enter(void)
{
  _cameraDx = 0;
  _cameraDy = 0;
  _activeGameObjects = 0;
  Scene_SetUpdateTo(_update);
  Graphic_InitCamera();

  _spriteSheetId = Graphic_LoadTexture("sprite-sheet2.bmp");

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  for (int y = 0; y < MAP_HEIGHT; y++) 
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      _objectTiles[y][x] = GameTile_Empty;
    }
  }

  _objectTiles[STAND_Y][STAND_X] = GameTile_Stand;


  _createMapSprite();
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      _createSpriteForTileObject(x, y);
    }
  }

  _createCustomer(NorthToSouthOnWestSide, _activeGameObjects++);
  _createCustomer(SouthToNorthOnWestSide, _activeGameObjects++);
  _createCustomer(NorthToSouthOnEastSide, _activeGameObjects++);
  _createCustomer(SouthToNorthOnEastSide, _activeGameObjects++);
  _createCustomer(EastOnNorthSideToNorthOnWestSide, _activeGameObjects++);
  _createCustomer(EastOnNorthSideToSouthOnWestSide, _activeGameObjects++);
  _createCustomer(EastOnNorthSideToNorthOnEastSide, _activeGameObjects++);
  _createCustomer(EastOnNorthSideToSouthOnEastSide, _activeGameObjects++);
  _createCustomer(EastOnSouthSideToNorthOnWestSide, _activeGameObjects++);
  _createCustomer(EastOnSouthSideToSouthOnWestSide, _activeGameObjects++);
  _createCustomer(EastOnSouthSideToNorthOnEastSide, _activeGameObjects++);
  _createCustomer(EastOnSouthSideToSouthOnEastSide, _activeGameObjects++);
  _createCustomer(SouthOnEastSideToEastOnNorthSide, _activeGameObjects++);
  _createCustomer(SouthOnEastSideToEastOnSouthSide, _activeGameObjects++);
  _createCustomer(SouthOnWestSideToEastOnNorthSide, _activeGameObjects++);
  _createCustomer(SouthOnWestSideToEastOnSouthSide, _activeGameObjects++);
  _createCustomer(NorthOnEastSideToEastOnNorthSide, _activeGameObjects++);
  _createCustomer(NorthOnEastSideToEastOnSouthSide, _activeGameObjects++);
  _createCustomer(NorthOnWestSideToEastOnNorthSide, _activeGameObjects++);
  _createCustomer(NorthOnWestSideToEastOnSouthSide, _activeGameObjects++);
  _createCustomerSprites();
  _createGameObject(
      GameTile_StopSignFacingEast, 
      SOUTH_TO_NORTH_EAST_SIDE_LANE + 1,
      EAST_TO_WEST_NORTH_SIDE_LANE - 1,
      0,
      0,
      0,
      0,
      _activeGameObjects++
  );
  _createGameObject(
      GameTile_StopSignFacingSouth, 
      SOUTH_TO_NORTH_EAST_SIDE_LANE + 1,
      WEST_TO_EAST_SOUTH_SIDE_LANE + 1,
      0,
      0,
      0,
      0,
      _activeGameObjects++
  );
  _createGameObject(
      GameTile_Bush,
      SOUTH_TO_NORTH_EAST_SIDE_LANE + 1,
      EAST_TO_WEST_NORTH_SIDE_LANE - 2,
      0,
      0,
      0,
      0,
      _activeGameObjects++
  );

  _createHouse(
      SOUTH_TO_NORTH_EAST_SIDE_LANE + 4,
      EAST_TO_WEST_NORTH_SIDE_LANE - 6
  );
  _pause = false;
  Graphic_CenterCamera();
  _dt = 0;
}

