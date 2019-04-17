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

static GameTiles groundTiles[MAP_HEIGHT][MAP_WIDTH];
static GameTiles objectTiles[MAP_HEIGHT][MAP_WIDTH];
static Id tilesSpriteId [MAP_HEIGHT][MAP_WIDTH];
static Id tilesObjectSpriteId [MAP_HEIGHT][MAP_WIDTH];

typedef struct {
  Id sprite;
  int x, y;
} Customer;

static Customer customers[MAX_CUSTOMERS];

static double cameraDx;
static double cameraDy;
static struct {
  int x, y, w, h, dx, dy;
} map;

static int zoom;
static int tileWidth;
static int tileHeight;

static int dt = 0;

static void
calculateTileWidth()
{
   tileWidth = DEFAULT_TILE_WIDTH * zoom;
}

static void 
calculateTileHeight()
{
  tileHeight = DEFAULT_TILE_HEIGHT * zoom;
}

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

    map.x += dx * zoom;
    map.y += dy * zoom;
    map.dx = map.x + (double) (MAP_HEIGHT - 1) / 2 * (tileWidth + zoom * 2);
    map.dy = map.y;
    Graphic_TranslateAllSprite(dx * zoom, dy * zoom);
  }
}

static void
setSrcForTile(GameTiles tile, SDL_Rect* src)
{
  switch (tile) {
    case GameTile_Empty:
      src->x = src->y = src->w = src->h = 0;
      break;
    case GameTile_Grass:
      src->x = 0;
      src->y = 0;
      src->w = 14;
      src->h = 8;
      break;
    case GameTile_SideWalk:
      src->x = 14;
      src->y = 0;
      src->w = 14;
      src->h = 8;
      break;
    case GameTile_Road:
      src->x = 28;
      src->y = 0;
      src->w = 14;
      src->h = 8;
      break;
    case GameTile_StandLeft:
      src->x = 0;
      src->y = 8;
      src->w = 14;
      src->h = 47;
      break;
    case GameTile_StandCenter:
      src->x = 14;
      src->y = 8;
      src->w = 14;
      src->h = 47;
      break;
    case GameTile_StandRight:
      src->x = 28;
      src->y = 8;
      src->w = 14;
      src->h = 47;
      break;
    case GameTile_StandingCharacterDown:
      src->x = 0;
      src->y = 55;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_StandingCharacterRight:
      src->x = 14;
      src->y = 55;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_StandingCharacterUp:
      src->x = 28;
      src->y = 55;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_StandingCharacterLeft:
      src->x = 42;
      src->y = 55;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_WalkingCharacterDown1:
      src->x = 0;
      src->y = 95;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_WalkingCharacterRight1:
      src->x = 14;
      src->y = 95;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_WalkingCharacterUp1:
      src->x = 28;
      src->y = 95;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_WalkingCharacterLeft1:
      src->x = 42;
      src->y = 95;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_WalkingCharacterDown2:
      src->x = 0;
      src->y = 135;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_WalkingCharacterRight2:
      src->x = 14;
      src->y = 135;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_WalkingCharacterUp2:
      src->x = 28; 
      src->y = 135;
      src->w = 14;
      src->h = 40;
      break;
    case GameTile_WalkingCharacterLeft2:
      src->x = 42;
      src->y = 135;
      src->w = 14;
      src->h = 40;
      break;
  }
}

static void moveCharacters(int x, int y, int newY)
{
  objectTiles[y][x] = GameTile_Empty;
  SDL_Rect src;
  SDL_Rect dest;

  setSrcForTile(objectTiles[newY][x], &src);
  
  dest.x = x * (tileWidth / 2 + zoom) - y * (tileWidth / 2 + zoom) + map.dx;
  dest.y = x * tileHeight / 2 + y * tileHeight / 2 + map.dy
    - (src.h - DEFAULT_TILE_HEIGHT) * zoom;
  dest.w = src.w * zoom;
  dest.h = src.h * zoom;

  Graphic_SetSpriteSrcAndDest(
      tilesObjectSpriteId[y][x], 
      src,
      dest
  );
  Graphic_SetSpriteToBeAfterAnother(
      tilesObjectSpriteId[y][x], 
      tilesSpriteId[newY][x]
  );

  tilesObjectSpriteId[newY][x] = tilesObjectSpriteId[y][x];
  tilesObjectSpriteId[y][x] = VOID_ID;
}

static void 
update(void)
{
  if (input_is_quit_pressed()) {
    graphic_clear();
    MainMenu_Enter();
  }


  if (dt == 10) {
    for (int y = MAP_HEIGHT; y--;) {
      int nextY = (y + 1) % MAP_HEIGHT;
      if (objectTiles[y][46] == GameTile_WalkingCharacterDown1) {
        objectTiles[nextY][46] = GameTile_WalkingCharacterDown2;
      } else if (objectTiles[y][46] == GameTile_WalkingCharacterDown2) {
        objectTiles[nextY][46] = GameTile_WalkingCharacterDown1;
      } else {
        continue;
      }
      moveCharacters(46, y, nextY);
    }

    for (int y = 0; y < MAP_HEIGHT; y++) {
      int nextY = y - 1 >= 0 ? y - 1 : MAP_HEIGHT - 1;
      if (objectTiles[y][47] == GameTile_WalkingCharacterUp1) {
        objectTiles[nextY][47] = GameTile_WalkingCharacterUp2;
      } else if (objectTiles[y][47] == GameTile_WalkingCharacterUp2) {
        objectTiles[nextY][47] = GameTile_WalkingCharacterUp1;
      } else {
        continue;
      }
      moveCharacters(47, y, nextY);
    }
    dt -= 10;
  }
  dt++;
  handleCamera();
}

void
createSpriteForTile(int x, int y, int dx, int dy)
{
  SDL_Rect src;
  SDL_Rect dest;

  setSrcForTile(groundTiles[y][x], &src);
  
  dest.x = x * (tileWidth / 2 + zoom) - y * (tileWidth / 2 + zoom) + dx;
  dest.y = x * (tileHeight / 2) + y * (tileHeight / 2) + dy;
  dest.w = src.w * zoom;
  dest.h = src.h * zoom;

  tilesSpriteId[y][x] = graphic_createTilesetSprite(
      spriteSheetId, 
      src, 
      dest
  );
}

void
createSpriteForTileObject(int x, int y, int dx, int dy)
{
  if (objectTiles[y][x] == GameTile_Empty) {
    return;
  }
  SDL_Rect src;
  SDL_Rect dest;

  setSrcForTile(objectTiles[y][x], &src);
  
  dest.x = x * (tileWidth / 2 + zoom) - y * (tileWidth / 2 + zoom) + dx;
  dest.y = x * tileHeight / 2 + y * tileHeight / 2 + dy
    - (src.h - DEFAULT_TILE_HEIGHT) * zoom;
  dest.w = src.w * zoom;
  dest.h = src.h * zoom;

  tilesObjectSpriteId[y][x] = graphic_createTilesetSprite(
      spriteSheetId, 
      src, 
      dest
  );
}

void 
Game_Enter(void)
{
  Scene_SetUpdateTo(update);

  zoom = 3;
  calculateTileWidth();
  calculateTileHeight();

  spriteSheetId = graphic_loadTexture("sprite-sheet.bmp");

  int w, h;
  graphic_queryWindowSize(&w, &h);

  map.w = (double) (MAP_WIDTH + MAP_HEIGHT) / 2 * (tileWidth + zoom * 2);
  map.h = (double) (MAP_WIDTH + MAP_HEIGHT) / 2 * tileHeight;
  map.x = (double) (w - map.w) / 2;
  map.y = (double) (h - map.h) / 2;
  map.dx = map.x + (double) (MAP_HEIGHT - 1) / 2 * (tileWidth + zoom * 2);
  map.dy = map.y;

  for (int y = 0; y < MAP_HEIGHT; y++) 
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      groundTiles[y][x] = GameTile_Grass;
      objectTiles[y][x] = GameTile_Empty;
    }
  }

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 48; x < 56; x++)
    {
      groundTiles[y][x] = GameTile_Road;
    }
  }

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    groundTiles[y][46] = GameTile_SideWalk;
    groundTiles[y][47] = GameTile_SideWalk;
    groundTiles[y][56] = GameTile_SideWalk;
    groundTiles[y][57] = GameTile_SideWalk;
  }

  objectTiles[40][45] = GameTile_StandLeft;
  objectTiles[39][45] = GameTile_StandCenter;
  objectTiles[38][45] = GameTile_StandRight;

  objectTiles[0][46] = GameTile_WalkingCharacterDown1;
  objectTiles[99][47] = GameTile_WalkingCharacterUp1;

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      createSpriteForTile(x, y, map.dx, map.dy);
      createSpriteForTileObject(x, y, map.dx, map.dy);
    }
  }
}

