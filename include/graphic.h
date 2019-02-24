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

typedef struct {
} Sprite;

bool graphic_init(const char * const title, int w, int h, int font_size);
void graphic_quit();

Id graphic_loadTexture(const char* const filename);

Id graphic_createTilesetSprite(
  Id texture_id,
  SDL_Rect src,
  SDL_Rect dest
);

Id graphic_createFullTextureSprite(
  Id texture_id,
  SDL_Rect dest
);

void graphic_render();
void graphic_queryTextureSize(Id texture_id, int* w, int* h);
void graphic_queryWindowSize(int* w, int* h);
void graphic_clear();

void graphic_setSpriteSize(Id id, int w, int h);
void graphic_translateSprite(Id id, int x, int y);
void graphic_queryPosition(Id id, int* x, int* y);
void graphic_setPosition(Id id, int x, int y);
void graphic_deleteSprite(Id id);
void graphic_deleteSpriteAndAttachedTexture(Id id);

Id graphic_createText(const char * const text, int x, int y, SDL_Color color);

Id graphic_createTextCentered(const char * const text, 
                              SDL_Rect zone, 
                              SDL_Color color);

void graphic_setText(Id id, 
                     const char* const text, 
                     int x, 
                     int y, 
                     SDL_Color color);

void graphic_deleteText(Id id);

void graphic_set_src_rect(Id id, SDL_Rect src);
void graphic_centerSpriteOnScreen(Id id);
void graphic_centerSpriteOnScreenWidth(Id id);
void graphic_centerSpriteOnScreenHeight(Id id);
void graphic_centerSpriteOnScreenWithOffset(Id id, int x, int y);
void graphic_centerSpriteOnScreenWidthWithOffset(Id id, int x);
void graphic_centerSpriteOnScreenHeightWithOffse(Id id, int y);

void graphic_setBackgroundTexture(Id textureId);
void graphic_resizeBackgroundToScreen();

Id graphic_createSolidTexture(Uint32 color);
void graphic_queryBackgroundDest(SDL_Rect *rect);
void graphic_querySpriteDest(Id id, SDL_Rect *rect);
void graphic_deleteTexture(Id id);

#endif /* RENDERER_H */

