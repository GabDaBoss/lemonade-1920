#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <limits.h>

#include "utils.h"

#define MAX_SPRITES 1024 * 1024
#define FONT "3270Medium.ttf"

typedef enum TextureIds {
    TilesetTexture,
    TotalTextureIds
} TextureIds;

typedef SmallId TextureId;
typedef Id SpriteId;
typedef TinyId TilesetId;
typedef Id SpriteTextId;

typedef struct {
} Sprite;

bool graphic_init(const char * const title, int w, int h, int font_size);
void graphic_quit();

TextureId graphic_loadTexture(const char* const filename);

SpriteId graphic_createTilesetSprite(
  TextureId texture_id,
  SDL_Rect src,
  SDL_Rect dest
);

SpriteId graphic_createFullTextureSprite(
  TextureId texture_id,
  SDL_Rect dest
);

void graphic_render();
void graphic_queryTextureSize(TextureId texture_id, int* w, int* h);
void graphic_queryWindowSize(int* w, int* h);
void graphic_clear();

void graphic_setSpriteSize(SpriteId id, int w, int h);
void graphic_translateSprite(SpriteId id, int x, int y);
void graphic_queryPosition(SpriteId id, int * x, int* y);
void graphic_setPosition(SpriteId id, int x, int y);
void graphic_deleteSprite(SpriteId id);

SpriteTextId 
graphic_createText(
    const char * const text, 
    int x, 
    int y,
    SDL_Color color
);

void graphic_setText(
  SpriteTextId id, 
  const char* const text,
  int x,
  int y,
  SDL_Color color
);

void graphic_deleteText(SpriteTextId id);

void graphic_set_src_rect(SpriteTextId id, SDL_Rect src);
void graphic_centerSpriteOnScreen(SpriteId id);
void graphic_centerSpriteOnScreenWidth(SpriteId id);
void graphic_centerSpriteOnScreenHeight(SpriteId id);
void graphic_centerSpriteOnScreenWithOffset(SpriteId id, int x, int y);
void graphic_centerSpriteOnScreenWidthWithOffset(SpriteId id, int x);
void graphic_centerSpriteOnScreenHeightWithOffse(SpriteId id, int y);

void graphic_setBackgroundTexture(TextureId textureId);
void graphic_resizeBackgroundToScreen();

#endif /* RENDERER_H */

