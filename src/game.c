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
  GameTile_StopSignFacingWest,
  GameTile_StopSignFacingNorth,
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
  GameTile_HouseTopLeftRoof,
  GameTile_FrontPorchStair,
  GameTile_NorthToSouthEntryWalkway,
  GameTile_EastToWestFence,
  GameTile_SouthToNorthFence,
  GameTile_EastToWestFenceEntrance,
  GameTile_WestToNorthFenceCorner,
  GameTile_NorthToSouthFence,
  GameTile_EastToNorthFenceCorner,
} GameTiles;

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define TILE_WIDTH 32
#define TILE_HEIGHT 16
#define MAX_GAME_OBJECTS 100
#define NORTH_TO_SOUTH_WEST_SIDE_LANE 26
#define SOUTH_TO_NORTH_WEST_SIDE_LANE 27
#define NORTH_TO_SOUTH_EAST_SIDE_LANE 36
#define SOUTH_TO_NORTH_EAST_SIDE_LANE 37
#define EAST_TO_WEST_NORTH_SIDE_LANE 10
#define WEST_TO_EAST_NORTH_SIDE_LANE 11
#define EAST_TO_WEST_SOUTH_SIDE_LANE 20
#define WEST_TO_EAST_SOUTH_SIDE_LANE 21
#define STAND_X 25
#define STAND_Y 25

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
  SouthOnWestSideToWestOnSouthSide,
  WestOnSouthSideToSouthOnWestSide,
  SouthOnEastSideToWestOnSouthSide,
  WestOnSouthSideToSouthOnEastSide,
  NorthOnWestSideToWestOnSouthSide,
  WestOnSouthSideToNorthOnWestSide,
  NorthOnEastSideToWestOnSouthSide,
  WestOnSouthSideToNorthOnEastSide,
  SouthOnWestSideToWestOnNorthSide,
  WestOnNorthSideToSouthOnWestSide,
  SouthOnEastSideToWestOnNorthSide,
  WestOnNorthSideToSouthOnEastSide,
  NorthOnWestSideToWestOnNorthSide,
  WestOnNorthSideToNorthOnWestSide,
  NorthOnEastSideToWestOnNorthSide,
  WestOnNorthSideToNorthOnEastSide,
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
_handleCamera() 
{
  int x, y, w, h;
  Input_QueryMousePosition(&x, &y);
  Graphic_QueryWindowSize(&w, &h);
  if (x < w * .20 || x > w * .80 || y < h * .20 || y > h * .80) {
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
    case GameTile_NorthToSouthEntryWalkway:
      src.x = 7 * TILE_WIDTH;
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
    case GameTile_StopSignFacingWest:
      src.x = 3 * TILE_WIDTH;
      src.y = 13 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 4 * TILE_HEIGHT;
      break;
    case GameTile_StopSignFacingNorth:
      src.x = 4 * TILE_WIDTH;
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
    case GameTile_FrontPorchStair:
      src.x = 1 * TILE_WIDTH;
      src.y = 26 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 2 * TILE_HEIGHT;
      break;
    case GameTile_EastToWestFence:
      src.x = 1 * TILE_WIDTH;
      src.y = 22 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 2 * TILE_HEIGHT;
      break;
    case GameTile_SouthToNorthFence:
      src.x = 1 * TILE_WIDTH;
      src.y = 24 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 2 * TILE_HEIGHT;
      break;
    case GameTile_EastToWestFenceEntrance:
      src.x = 2 * TILE_WIDTH;
      src.y = 22 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 2 * TILE_HEIGHT;
      break;
    case GameTile_WestToNorthFenceCorner:
      src.x = 2 * TILE_WIDTH;
      src.y = 24 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 2 * TILE_HEIGHT;
      break;
    case GameTile_NorthToSouthFence:
      src.x = 0 * TILE_WIDTH;
      src.y = 22 * TILE_HEIGHT;
      src.w = TILE_WIDTH;
      src.h = 2 * TILE_HEIGHT;
      break;
    case GameTile_EastToNorthFenceCorner:
      src.x = 3 * TILE_WIDTH;
      src.y = 23 * TILE_HEIGHT;
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
  if (_gameObjects[i].tile != frame1 && _gameObjects[i].tile != frame2) {
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
      case WestOnSouthSideToNorthOnWestSide:
      case WestOnNorthSideToNorthOnWestSide:
        if (_gameObjects[i].x >= SOUTH_TO_NORTH_WEST_SIDE_LANE) {
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
            GameTile_WalkingCharacterEast1, 
            GameTile_WalkingCharacterEast2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = 0.05;
        }
        break;
      case WestOnSouthSideToNorthOnEastSide:
      case WestOnNorthSideToNorthOnEastSide:
        if (_gameObjects[i].x >= SOUTH_TO_NORTH_EAST_SIDE_LANE) {
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
            GameTile_WalkingCharacterEast1, 
            GameTile_WalkingCharacterEast2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = 0.05;
        }
        break;
      case WestOnNorthSideToSouthOnWestSide:
      case WestOnSouthSideToSouthOnWestSide:
        if (_gameObjects[i].x >= NORTH_TO_SOUTH_WEST_SIDE_LANE) {
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
            GameTile_WalkingCharacterEast1, 
            GameTile_WalkingCharacterEast2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = 0.05;
        }
        break;
      case WestOnSouthSideToSouthOnEastSide:
      case WestOnNorthSideToSouthOnEastSide:
        if (_gameObjects[i].x >= NORTH_TO_SOUTH_EAST_SIDE_LANE) {
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
            GameTile_WalkingCharacterEast1, 
            GameTile_WalkingCharacterEast2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = 0.05;
        }
        break;
      case SouthOnEastSideToWestOnSouthSide:
      case SouthOnWestSideToWestOnSouthSide:
        if ((_gameObjects[i].y) <= WEST_TO_EAST_SOUTH_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterWest1, 
            GameTile_WalkingCharacterWest2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = -0.05;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterNorth1, 
            GameTile_WalkingCharacterNorth2
          );
          _gameObjects[i].dy = -0.05;
          _gameObjects[i].dx = 0;
        }
        break;
      case SouthOnWestSideToWestOnNorthSide:
      case SouthOnEastSideToWestOnNorthSide:
        if ((_gameObjects[i].y) <= WEST_TO_EAST_NORTH_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterWest1, 
            GameTile_WalkingCharacterWest2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = -0.05;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterNorth1, 
            GameTile_WalkingCharacterNorth2
          );
          _gameObjects[i].dy = -0.05;
          _gameObjects[i].dx = 0;
        }
        break;
      case NorthOnWestSideToWestOnSouthSide:
      case NorthOnEastSideToWestOnSouthSide:
        if ((_gameObjects[i].y) >= WEST_TO_EAST_SOUTH_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterWest1, 
            GameTile_WalkingCharacterWest2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = -0.05;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterSouth1, 
            GameTile_WalkingCharacterSouth2
          );
          _gameObjects[i].dy = 0.05;
          _gameObjects[i].dx = 0;
        }
        break;
      case NorthOnWestSideToWestOnNorthSide:
      case NorthOnEastSideToWestOnNorthSide:
        if ((_gameObjects[i].y) >= WEST_TO_EAST_NORTH_SIDE_LANE) {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterWest1, 
            GameTile_WalkingCharacterWest2
          );
          _gameObjects[i].dy = 0;
          _gameObjects[i].dx = -0.05;
        } else {
          _animateGameObject(
            i, 
            GameTile_WalkingCharacterSouth1, 
            GameTile_WalkingCharacterSouth2
          );
          _gameObjects[i].dy = 0.05;
          _gameObjects[i].dx = 0;
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
      case WestOnNorthSideToNorthOnWestSide:
      case WestOnNorthSideToNorthOnEastSide:
        if (y < 0) {
          x = 0;
          y = EAST_TO_WEST_NORTH_SIDE_LANE;
        }
        break;
      case WestOnSouthSideToNorthOnWestSide:
      case WestOnSouthSideToNorthOnEastSide:
        if (y < 0) {
          x = 0;
          y = EAST_TO_WEST_SOUTH_SIDE_LANE;
        }
        break;
      case NorthToSouthOnWestSide:
      case NorthToSouthOnEastSide:
        if (y >= MAP_HEIGHT) {
          y = 0;
        }
        break;
      case WestOnSouthSideToSouthOnWestSide:
      case WestOnSouthSideToSouthOnEastSide:
        if (y >= MAP_HEIGHT) {
          x = 0;
          y = EAST_TO_WEST_SOUTH_SIDE_LANE;
        }
        break;
      case WestOnNorthSideToSouthOnWestSide:
      case WestOnNorthSideToSouthOnEastSide:
        if (y >= MAP_HEIGHT) {
          x = 0;
          y = EAST_TO_WEST_NORTH_SIDE_LANE;
        }
        break;
      case SouthOnWestSideToWestOnSouthSide:
      case SouthOnWestSideToWestOnNorthSide:
        if (x < 0) {
          x = SOUTH_TO_NORTH_WEST_SIDE_LANE;
          y = MAP_HEIGHT;
        }
        break;
      case SouthOnEastSideToWestOnSouthSide:
      case SouthOnEastSideToWestOnNorthSide:
        if (x < 0) {
          x = SOUTH_TO_NORTH_EAST_SIDE_LANE;
          y = MAP_HEIGHT;
        }
        break;
      case NorthOnWestSideToWestOnSouthSide:
      case NorthOnWestSideToWestOnNorthSide:
        if (x < 0) {
          x = NORTH_TO_SOUTH_WEST_SIDE_LANE;
          y = 0;
        }
        break;
      case NorthOnEastSideToWestOnSouthSide:
      case NorthOnEastSideToWestOnNorthSide:
        if (x < 0) {
          x = NORTH_TO_SOUTH_EAST_SIDE_LANE;
          y = 0;
        }
        break;
    }


    double dx = x - _gameObjects[i].x;
    double dy = y - _gameObjects[i].y;

    _gameObjects[i].x = x;
    _gameObjects[i].y = y;

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
    Game_UpdateSimulation();
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
    case SouthOnWestSideToWestOnSouthSide:
    case SouthOnWestSideToWestOnNorthSide:
    case SouthToNorthOnEastSide:
    case SouthOnEastSideToWestOnSouthSide:
    case SouthOnEastSideToWestOnNorthSide:
      _gameObjects[i].tile = GameTile_WalkingCharacterNorth1;
      _gameObjects[i].dy = -0.05;
      _gameObjects[i].y = MAP_HEIGHT;
      break;
    case NorthToSouthOnWestSide:
    case NorthToSouthOnEastSide:
    case NorthOnWestSideToWestOnSouthSide:
    case NorthOnEastSideToWestOnSouthSide:
    case NorthOnWestSideToWestOnNorthSide:
    case NorthOnEastSideToWestOnNorthSide:
      _gameObjects[i].tile = GameTile_WalkingCharacterSouth1;
      _gameObjects[i].dy = 0.05;
      _gameObjects[i].y = 0;
      break;
    case WestOnSouthSideToSouthOnWestSide:
    case WestOnSouthSideToSouthOnEastSide:
    case WestOnSouthSideToNorthOnWestSide:
    case WestOnSouthSideToNorthOnEastSide:
    case WestOnNorthSideToSouthOnWestSide:
    case WestOnNorthSideToSouthOnEastSide:
    case WestOnNorthSideToNorthOnWestSide:
    case WestOnNorthSideToNorthOnEastSide:
      _gameObjects[i].tile = GameTile_WalkingCharacterEast1;
      _gameObjects[i].dx = 0.05;
      _gameObjects[i].x = 0;
      break;
  }

  switch(path) {
    case SouthToNorthOnWestSide:
    case SouthOnWestSideToWestOnSouthSide:
    case SouthOnWestSideToWestOnNorthSide:
      _gameObjects[i].x = SOUTH_TO_NORTH_WEST_SIDE_LANE;
      break;
    case SouthToNorthOnEastSide:
    case SouthOnEastSideToWestOnSouthSide:
    case SouthOnEastSideToWestOnNorthSide:
      _gameObjects[i].x = SOUTH_TO_NORTH_EAST_SIDE_LANE;
      break;
    case NorthToSouthOnWestSide:
    case NorthOnWestSideToWestOnSouthSide:
    case NorthOnWestSideToWestOnNorthSide:
      _gameObjects[i].x = NORTH_TO_SOUTH_WEST_SIDE_LANE;
      break;
    case NorthOnEastSideToWestOnSouthSide:
    case NorthToSouthOnEastSide:
    case NorthOnEastSideToWestOnNorthSide:
      _gameObjects[i].x = NORTH_TO_SOUTH_EAST_SIDE_LANE;
      break;
    case WestOnSouthSideToSouthOnWestSide:
    case WestOnSouthSideToSouthOnEastSide:
    case WestOnSouthSideToNorthOnWestSide:
    case WestOnSouthSideToNorthOnEastSide:
      _gameObjects[i].y = EAST_TO_WEST_SOUTH_SIDE_LANE;
      break;
    case WestOnNorthSideToSouthOnWestSide:
    case WestOnNorthSideToSouthOnEastSide:
    case WestOnNorthSideToNorthOnWestSide:
    case WestOnNorthSideToNorthOnEastSide:
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
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      mapSprites[y][x].src = _getTileSrc(GameTile_Grass);
      mapSprites[y][x].textureId = _spriteSheetId;
    }
  }

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = SOUTH_TO_NORTH_WEST_SIDE_LANE + 1; x < NORTH_TO_SOUTH_EAST_SIDE_LANE; x++) {
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
    mapSprites[y][NORTH_TO_SOUTH_WEST_SIDE_LANE].src = _getTileSrc(GameTile_CrosswalkNorthSouth1);
    mapSprites[y][SOUTH_TO_NORTH_WEST_SIDE_LANE].src = _getTileSrc(GameTile_CrosswalkNorthSouth2);
  }

  for (int x = SOUTH_TO_NORTH_WEST_SIDE_LANE + 1; x < NORTH_TO_SOUTH_EAST_SIDE_LANE; x++) {
    mapSprites[EAST_TO_WEST_NORTH_SIDE_LANE][x].src = _getTileSrc(GameTile_CrosswalkEastWest2);
    mapSprites[WEST_TO_EAST_NORTH_SIDE_LANE][x].src = _getTileSrc(GameTile_CrosswalkEastWest1);
    mapSprites[EAST_TO_WEST_SOUTH_SIDE_LANE][x].src = _getTileSrc(GameTile_CrosswalkEastWest2);
    mapSprites[WEST_TO_EAST_SOUTH_SIDE_LANE][x].src = _getTileSrc(GameTile_CrosswalkEastWest1);
  }

  for (int x = 0; x < NORTH_TO_SOUTH_WEST_SIDE_LANE;  x++) {
    for (int y = WEST_TO_EAST_NORTH_SIDE_LANE + 1; y < EAST_TO_WEST_SOUTH_SIDE_LANE; y++) {
      mapSprites[y][x].src = _getTileSrc(GameTile_Road);
    }
    mapSprites[EAST_TO_WEST_NORTH_SIDE_LANE][x].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[WEST_TO_EAST_NORTH_SIDE_LANE][x].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[EAST_TO_WEST_SOUTH_SIDE_LANE][x].src = _getTileSrc(GameTile_SideWalk);
    mapSprites[WEST_TO_EAST_SOUTH_SIDE_LANE][x].src = _getTileSrc(GameTile_SideWalk);
  }

  mapSprites[EAST_TO_WEST_NORTH_SIDE_LANE - 1][NORTH_TO_SOUTH_WEST_SIDE_LANE - 8].src = _getTileSrc(GameTile_NorthToSouthEntryWalkway);
  mapSprites[EAST_TO_WEST_NORTH_SIDE_LANE - 2][NORTH_TO_SOUTH_WEST_SIDE_LANE - 8].src = _getTileSrc(GameTile_NorthToSouthEntryWalkway);
  mapSprites[EAST_TO_WEST_NORTH_SIDE_LANE - 3][NORTH_TO_SOUTH_WEST_SIDE_LANE - 8].src = _getTileSrc(GameTile_NorthToSouthEntryWalkway);
  mapSprites[EAST_TO_WEST_NORTH_SIDE_LANE - 4][NORTH_TO_SOUTH_WEST_SIDE_LANE - 8].src = _getTileSrc(GameTile_NorthToSouthEntryWalkway);
  mapSprites[EAST_TO_WEST_NORTH_SIDE_LANE - 5][NORTH_TO_SOUTH_WEST_SIDE_LANE - 8].src = _getTileSrc(GameTile_NorthToSouthEntryWalkway);

  for (int x = 0; x < MAP_WIDTH; x++) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
      mapSprites[y][x].dest = _getTileDest(mapSprites[y][x].src, x, y);
    }
  }


  Graphic_CreateSpriteFromSprites(
      (Sprite*) mapSprites, 
      (Sprite*) mapSprites[MAP_WIDTH]
  );
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
  _createGameObject(GameTile_Bush, x, y + 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Bush, x + 1, y + 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_FrontPorchStair, x + 2, y + 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Bush, x + 3, y + 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Bush, x + 4, y + 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Bush, x + 5, y + 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Bush, x + 6, y + 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_Bush, x + 7, y + 1, 0, 0, 0, 0, _activeGameObjects++);

  _createGameObject(GameTile_NorthToSouthFence, x - 4, y + 3, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_NorthToSouthFence, x - 4, y + 2, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_NorthToSouthFence, x - 4, y + 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_NorthToSouthFence, x - 4, y, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_NorthToSouthFence, x - 4, y - 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_NorthToSouthFence, x - 4, y - 2, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_NorthToSouthFence, x - 4, y - 3, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_NorthToSouthFence, x - 4, y - 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToNorthFenceCorner, x - 4, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x - 3, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x - 2, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x - 1, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x + 1, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFenceEntrance, x + 2, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x + 3, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x + 4, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x + 5, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x + 6, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_EastToWestFence, x + 7, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_WestToNorthFenceCorner, x + 8, y + 4, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_SouthToNorthFence, x + 8, y + 3, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_SouthToNorthFence, x + 8, y + 2, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_SouthToNorthFence, x + 8, y + 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_SouthToNorthFence, x + 8, y, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_SouthToNorthFence, x + 8, y - 1, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_SouthToNorthFence, x + 8, y - 2, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_SouthToNorthFence, x + 8, y - 3, 0, 0, 0, 0, _activeGameObjects++);
  _createGameObject(GameTile_SouthToNorthFence, x + 8, y - 4, 0, 0, 0, 0, _activeGameObjects++);
}

static void
_createFirstLevel()
{
  _spriteSheetId = Graphic_LoadTexture("sprite-sheet2.bmp");

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      _objectTiles[y][x] = GameTile_Empty;
    }
  }

  _objectTiles[STAND_Y][STAND_X] = GameTile_Stand;


  _createMapSprite();
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      _createSpriteForTileObject(x, y);
    }
  }

  _createCustomer(NorthToSouthOnWestSide, _activeGameObjects++);
  _createCustomer(SouthToNorthOnWestSide, _activeGameObjects++);
  _createCustomer(NorthToSouthOnEastSide, _activeGameObjects++);
  _createCustomer(SouthToNorthOnEastSide, _activeGameObjects++);
  _createCustomer(WestOnNorthSideToNorthOnWestSide, _activeGameObjects++);
  _createCustomer(WestOnNorthSideToSouthOnWestSide, _activeGameObjects++);
  _createCustomer(WestOnNorthSideToNorthOnEastSide, _activeGameObjects++);
  _createCustomer(WestOnNorthSideToSouthOnEastSide, _activeGameObjects++);
  _createCustomer(WestOnSouthSideToNorthOnWestSide, _activeGameObjects++);
  _createCustomer(WestOnSouthSideToSouthOnWestSide, _activeGameObjects++);
  _createCustomer(WestOnSouthSideToNorthOnEastSide, _activeGameObjects++);
  _createCustomer(WestOnSouthSideToSouthOnEastSide, _activeGameObjects++);
  _createCustomer(SouthOnEastSideToWestOnNorthSide, _activeGameObjects++);
  _createCustomer(SouthOnEastSideToWestOnSouthSide, _activeGameObjects++);
  _createCustomer(SouthOnWestSideToWestOnNorthSide, _activeGameObjects++);
  _createCustomer(SouthOnWestSideToWestOnSouthSide, _activeGameObjects++);
  _createCustomer(NorthOnEastSideToWestOnNorthSide, _activeGameObjects++);
  _createCustomer(NorthOnEastSideToWestOnSouthSide, _activeGameObjects++);
  _createCustomer(NorthOnWestSideToWestOnNorthSide, _activeGameObjects++);
  _createCustomer(NorthOnWestSideToWestOnSouthSide, _activeGameObjects++);
  _createCustomerSprites();
  _createGameObject(
      GameTile_StopSignFacingWest, 
      NORTH_TO_SOUTH_WEST_SIDE_LANE - 1,
      WEST_TO_EAST_SOUTH_SIDE_LANE + 1,
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
      GameTile_StopSignFacingNorth,
      NORTH_TO_SOUTH_WEST_SIDE_LANE - 1,
      EAST_TO_WEST_NORTH_SIDE_LANE - 1,
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
      NORTH_TO_SOUTH_WEST_SIDE_LANE - 10,
      EAST_TO_WEST_NORTH_SIDE_LANE - 6
  );
  _pause = false;
  Graphic_CenterCamera();
  _dt = 0;
}

void 
Game_Enter(void)
{
  _cameraDx = 0;
  _cameraDy = 0;
  _activeGameObjects = 0;
  Scene_SetUpdateTo(_update);
  Graphic_InitCamera();
  _createFirstLevel();
}

void
Game_StartSimulation()
{
  _createFirstLevel();
}

void
Game_UpdateSimulation()
{
  _moveGameObjects();
  if (_dt == 15) {
    _dt -= 15;
    _animateGameObjects();
  } else {
    _dt++;
  }
  _reorderGameObjects();
}

