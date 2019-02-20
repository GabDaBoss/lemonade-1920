#include "game.h"

typedef enum {
  TILE_FLOOR = 1 << 0,
  TILE_BALL = 1 << 1,
} TileTypes;

static struct {
  TileTypes types[GAME_TILEMAP_WIDTH][GAME_TILEMAP_HEIGHT];
} tiles;

static double playerX, playerY;

void generateTiles();

void
game_start()
{
  generateTiles();
}

void
generateTiles()
{
  int w = GAME_TILEMAP_WIDTH, h = GAME_TILEMAP_HEIGHT;
  /**
   * Can turn the tile into a floor if turning it into a tile would create a
   * square of 4 tiles
   *
   * X X O  O X X  O O O  O O O
   * X C O  O C X  O C X  X C O
   * O O O  O O O  O X X  X X O
   **/
  TileTypes leftType = 0;
  TileTypes rightType = 0;
  TileTypes upType = 0;
  TileTypes upLeftType = 0;
  TileTypes upRightType = 0;
  TileTypes downLeftType = 0;
  TileTypes downRightType = 0;
  TileTypes downType = 0;

  for (int x = 0; x < w; x++) {
    upLeftType = 0;
    upType = 0;
    upRightType = 0;
    for (int y = 0; y < h; y++) {
      if (x > 0) {
        leftType = tiles.types[x - 1][y];
      }

      if (x < w - 1) {
        rightType = tiles.types[x + 1][y];
      }

      if (y > 0) {
        if (x > 0) {
          upLeftType = tiles.types[x - 1][y - 1];
        }

        upType = tiles.types[x][y - 1];

        if (x < w - 1) {
          upRightType = tiles.types[x + 1][y -1];
        }
      }

      if (y < h - 1) {
        if (x > 0) {
          downLeftType = tiles.types[x - 1][y + 1];
        } else {
          downLeftType = 0;
        }

        downType = tiles.types[x][y + 1];

        if (x < w - 1) {
          downRightType = tiles.types[x + 1][y + 1];
        } else {
          downRightType = 0;
        }
      }

      if (leftType | TILE_FLOOR && 
          upLeftType | TILE_FLOOR &&
          upType | TILE_FLOOR) {
        continue;
      }

      if (rightType | TILE_FLOOR &&
          upRightType | TILE_FLOOR &&
          upType | TILE_FLOOR) {
        continue;
      }

      tiles.types[x][y] |= TILE_FLOOR;
    }
  }
}
