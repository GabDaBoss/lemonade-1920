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

bool Graphic_Init(const char * const title, int w, int h, int font_size);
void Graphic_Quit();
void Graphic_Render();
void Graphic_QueryTextureSize(Id texture_id, int* w, int* h);
void Graphic_QueryWindowSize(int* w, int* h);
void Graphic_Clear();
void Graphic_TranslateAllSprite(int dx, int dy);
void Graphic_TranslateSprite(Id id, int x, int y);
void Graphic_QueryPosition(Id id, int* x, int* y);
void Graphic_DeleteSprite(Id id);
void Graphic_DeleteSpriteAndAttachedTexture(Id id);
void Graphic_DeleteTexture(Id id);
void Graphic_DeleteText(Id id);
void Graphic_CenterSpriteOnScreen(Id id);
void Graphic_CenterSpriteOnScreenWidth(Id id);
void Graphic_CenterSpriteOnScreenHeight(Id id);
void Graphic_CenterSpriteOnScreenWithOffset(Id id, int x, int y);
void Graphic_CenterSpriteOnScreenWidthWithOffset(Id id, int x);
void Graphic_CenterSpriteOnScreenHeightWithOffset(Id id, int y);
void Graphic_QuerySpriteDest(Id id, SDL_Rect *rect);
void Graphic_ResizeSpriteToScreen(Id id);
void Graphic_ZoomSprites(double zoom);
void Graphic_ApplyZoomSprites(double zoom);

void Graphic_SetSpriteToBeAfterAnother(Id id, Id other);
void Graphic_SetSpriteSrcAndDest(Id id, SDL_Rect src, SDL_Rect dest);
void Graphic_SetText(Id id, const char* const text, int x, int y, SDL_Color color);
void Graphic_SetSpriteSize(Id id, int w, int h);
void Graphic_SetSpriteSrcRect(Id id, SDL_Rect src);
void Graphic_SetPosition(Id id, int x, int y);
void Graphic_SetSpriteToInactive(Id id);
void Graphic_SetSpriteToActive(Id id);
void Graphic_SetSpriteDest(Id id, SDL_Rect dest);
void Graphic_CenterSpriteInRect(Id id, SDL_Rect rect);
void Graphic_CenterSpriteInRectButKeepRatio(Id id, SDL_Rect rect);

Id Graphic_LoadTexture(const char* const filename);
Id Graphic_CreateTilesetSprite(Id texture_id, SDL_Rect src, SDL_Rect dest);
Id Graphic_CreateFullTextureSprite(Id texture_id, SDL_Rect dest); 
Id Graphic_CreateText(const char * const text, int x, int y, SDL_Color color);
Id Graphic_CreateTextCentered(const char * const text, SDL_Rect zone, SDL_Color color);
Id Graphic_CreateSolidTexture(Uint32 color);
Id Graphic_CreateTextTexture(const char * const text, SDL_Color color);
Id Graphic_CreateInactiveSprite(Id textureId);
Id Graphic_CreateInactiveText(char * const text, SDL_Color color);

#endif /* RENDERER_H */

