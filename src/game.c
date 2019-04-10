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

/*
static const unsigned char tilemapWidth = 100;
static const unsigned char tilemapHeight = 100;
static const unsigned char tileWidth = 14;
static const unsigned char tileHeight = 8;
*/

#define tilemapWidth 100
#define tilemapHeight 100
#define tileWidth 14
#define tileHeight 8

static GameTiles groundTiles[tilemapHeight][tilemapWidth];
static GameTiles objectTiles[tilemapHeight][tilemapWidth];
static Id tilesSpriteId [tilemapHeight][tilemapWidth];
static double cameraDx;
static double cameraDy;
static struct {
  int x, y, w, h;
} map;

static void 
handleCamera() {
  int x, y, w, h;
  Input_QueryMousePosition(&x, &y);
  graphic_queryWindowSize(&w, &h);
  if (x < w * .20 || x > w * .80 || y < h * .20 || y > h * .80) 
  {
    cameraDx += (double) (x - w / 2) / (w / 2);
    cameraDy += (double) (y - h / 2) / (h / 2);
    int dx = 0, dy = 0;
    if (cameraDx >= 1) {
      dx = -1;
      cameraDx--;
    } else if (cameraDx <= -1) {
      dx = 1;
      cameraDx++;
    }

    if (cameraDy >= 1) {
      dy = -1;
      cameraDy--;
    } else if (cameraDy <= -1) {
      dy = 1;
      cameraDy++;
    }

    if (dx > 0 && map.x >= 0) {
      dx = 0;
    }

    if (dx < 0 && map.x <= w - map.w) {
      dx = 0;
    }

    if (dy > 0 && map.y >= 0) {
      dy = 0;
    }

    if (dy < 0 && map.y <= h - map.h) {
      dy = 0;
    }

    if (!dx && !dy) {
      return;
    }

    map.x += dx;
    map.y += dy;
    Graphic_TranslateAllSprite(dx, dy);
  }
}

static void 
update(void)
{
  if (input_is_quit_pressed()) {
    graphic_clear();
    MainMenu_Enter();
  }

  handleCamera();
}

void 
Game_Enter(void)
{
  Scene_SetUpdateTo(update);

  spriteSheetId = graphic_loadTexture("sprite-sheet.bmp");

  int w, h, dx, dy;
  graphic_queryWindowSize(&w, &h);

  map.w = (double) (tilemapWidth + tilemapHeight) / 2 * tileWidth;
  map.h = (double) (tilemapWidth + tilemapHeight) / 2 * tileHeight;
  map.x = (double) (w - map.w) / 2;
  map.y = (double) (h - map.h) / 2;
  dx = map.x + (double) (tilemapHeight - 1) / 2 * tileWidth;
  dy = map.y;

  for (int y = 0; y < tilemapHeight; y++) 
  {
    for (int x = 0; x < tilemapWidth; x++)
    {
      groundTiles[y][x] = GameTile_Grass;
      objectTiles[y][x] = GameTile_Empty;
    }
  }

  for (int y = 48; y < 56; y++)
  {
    for (int x = 0; x < tilemapWidth; x++)
    {
      groundTiles[y][x] = GameTile_Road;
    }
  }

  for (int x = 0; x < tilemapWidth; x++)
  {
    groundTiles[46][x] = GameTile_SideWalk;
    groundTiles[47][x] = GameTile_SideWalk;
  }

  for (int x = 0; x < tilemapWidth; x++)
  {
    groundTiles[56][x] = GameTile_SideWalk;
    groundTiles[57][x] = GameTile_SideWalk;
  }

  for (int y = 0; y < tilemapHeight; y++)
  {
    for (int x = 0; x < tilemapWidth; x++)
    {
      SDL_Rect src;
      SDL_Rect dest;

      switch (groundTiles[y][x])
      {
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
      
      dest.x = x * (double) tileWidth / 2 - y * (double) tileWidth / 2 + dx;
      dest.y = x * (double) tileHeight / 2 + y * (double) tileHeight / 2 + dy;
      dest.w = src.w;
      dest.h = src.h;

      tilesSpriteId[y][x] = graphic_createTilesetSprite(
          spriteSheetId, 
          src, 
          dest
      );
    }
  }
}

