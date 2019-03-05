#include <stdio.h>

#include "graphic.h"

#define MAX_TEXTURES 1024

static SDL_Window* window;
static SDL_Renderer* renderer;
static TTF_Font* font;

static struct {
    SDL_Texture* textures[MAX_TEXTURES];
    SET_STRUCT_FOR_DOD(Id, MAX_TEXTURES);
} textures;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect src;
    SDL_Rect dest;
} Sprite;

static struct {
  Sprite sprite[MAX_SPRITES];
  unsigned int visibleSpriteTotal;
  SET_STRUCT_FOR_DOD(Id, MAX_SPRITES);
} sprites;

static Id backgroundTextureIndex;
static SDL_Rect backgroundSrc;
static SDL_Rect backgroundDest;

static void deleteTextureByIndex(Index index);
static Id createTilesetSprite(SDL_Texture* texture, SDL_Rect src, SDL_Rect dest);
static SDL_Texture* createTextTexture(const char * const text, SDL_Color color, unsigned int *w, unsigned int *h);
static void queryTextureSize(SDL_Texture* texture, int* w, int* h);

bool graphic_init(const char * const title, int w, int h, int font_size)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL couldn't initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        fprintf(stderr, "SDL image couldn't initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    if(TTF_Init() != 0) {
        fprintf(stderr, "SDL ttf couldn't initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    };

    font = TTF_OpenFont(FONT, font_size);
    if(!font) {
        fprintf(stderr, "SDL ttf couldn't open the font! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        w,
        h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (window == NULL) {
        fprintf(stderr, "Window couldn't be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }


    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);


    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    INIT_STRUCT_FOR_DOD_FREE_LIST(sprites, MAX_SPRITES);
    INIT_STRUCT_FOR_DOD_FREE_LIST(textures, MAX_TEXTURES);

    return true;
}

void 
graphic_render()
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    if (backgroundTextureIndex != VOID_INDEX) { 
        SDL_RenderCopy(
            renderer,
            textures.textures[backgroundTextureIndex],
            &backgroundSrc,
            &backgroundDest
        );
    }

    for (unsigned int i = 0; i < sprites.visibleSpriteTotal; i++) {
        SDL_RenderCopy(
            renderer,
            sprites.sprite[i].texture,
            &sprites.sprite[i].src,
            &sprites.sprite[i].dest);
    }

    SDL_RenderPresent(renderer);
}

Id
graphic_loadTexture(const char* const filename)
{
    SDL_Surface* surface = IMG_Load(filename);
    Uint32 colorkey = SDL_MapRGB(surface->format, 0xff, 0, 0xff);
    SDL_SetColorKey(surface, SDL_TRUE, colorkey);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == NULL) {
        fprintf(stderr, "Texture %s could not be loaded! SDL_Error: %s\n", filename, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    Index index;
    Id id;
    GET_NEXT_ID(textures, id, index, MAX_TEXTURES);
    textures.textures[index] = texture;

    return id;
}

Id 
createTilesetSprite( 
    SDL_Texture* texture, 
    SDL_Rect src, 
    SDL_Rect dest) 
{
    Index index;
    Id id;
    GET_NEXT_ID(sprites, id, index, MAX_SPRITES);

    if (sprites.visibleSpriteTotal < index) {
      sprites.sprite[index] = sprites.sprite[sprites.visibleSpriteTotal];
      sprites.indexes[sprites.ids[sprites.visibleSpriteTotal]] = index;
      sprites.ids[sprites.visibleSpriteTotal] = id;
    }

    sprites.sprite[sprites.visibleSpriteTotal].src = src;
    sprites.sprite[sprites.visibleSpriteTotal].dest = dest;
    sprites.sprite[sprites.visibleSpriteTotal].texture = texture;
    sprites.visibleSpriteTotal++;

    return id;
}

Id 
graphic_createTilesetSprite( 
    Id texture_id, 
    SDL_Rect src, 
    SDL_Rect dest) 
{
    Index texture_index;
    GET_INDEX_FROM_ID(textures, texture_id, texture_index);

    return createTilesetSprite(textures.textures[texture_index], src, dest);
}

Id 
graphic_createFullTextureSprite( 
    Id texture_id, 
    SDL_Rect dest) 
{
    SDL_Rect src;
    src.x = 0;
    src.y = 0;
    queryTextureSize(texture_id, &src.w, &src.h);

    return graphic_createTilesetSprite(texture_id, src, dest);
}

void
graphic_setSpriteSize(Id id, int w, int h)
{
  Index index;
  GET_INDEX_FROM_ID(sprites, id, index);
  sprites.sprite[index].dest.w = w;
  sprites.sprite[index].dest.h = h;
}

void
graphic_translateSprite(Id id, int x, int y)
{
  Index index;
  GET_INDEX_FROM_ID(sprites, id, index);

  sprites.sprite[index].dest.x += x;
  sprites.sprite[index].dest.y += y;
}

void
graphic_deleteSprite(Id id) 
{
    Index index, last;

    GET_INDEX_FROM_ID(sprites, id, index);

    sprites.indexes[id] = sprites.next_free_index;
    sprites.next_free_index = id;

    if (index < sprites.visibleSpriteTotal) {
        last = --sprites.visibleSpriteTotal;
        if (id != sprites.ids[last]) {
            sprites.ids[index] = sprites.ids[last];
            sprites.indexes[sprites.ids[last]] = index;
        }
        sprites.sprite[index] = sprites.sprite[last];
        index = last;
        id = sprites.ids[last];
    } 


    last = --sprites.total;
    if (id != sprites.ids[last]) {
        sprites.ids[index] = sprites.ids[last];
        sprites.indexes[sprites.ids[last]] = index;
    }
    sprites.sprite[index] = sprites.sprite[last];
}

void 
queryTextureSize(SDL_Texture* texture, int* w, int* h)
{
    SDL_QueryTexture(texture, NULL, NULL, w, h);
}

void 
graphic_queryTextureSize(Id texture_id, int* w, int* h)
{
    Id index;
    GET_INDEX_FROM_ID(textures, texture_id, index);
    SDL_Texture* texture = textures.textures[index];
    SDL_QueryTexture(texture, NULL, NULL, w, h);
}

void
graphic_quit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    for( int i = 0; i < MAX_TEXTURES; i++ ) {
        if( textures.textures[i] ) {
            SDL_DestroyTexture( textures.textures[i] );
        }
    }

    TTF_CloseFont( font );
    TTF_Quit();

    IMG_Quit();
    SDL_Quit();
}

void 
graphic_queryWindowSize( int* w, int* h )
{
    SDL_GetWindowSize( window, w, h );
}

void graphic_queryPosition(Id id, int * x, int* y)
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);
    (*x) = sprites.sprite[index].dest.x;
    (*y) = sprites.sprite[index].dest.y;
}

void graphic_setPosition(Id id, int x, int y)
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);
    sprites.sprite[index].dest.x = x;
    sprites.sprite[index].dest.y = y;
}

SDL_Texture*
createTextTexture(const char * const text, SDL_Color color, unsigned int *w, unsigned int *h)
{
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);

    if (!surface) {
        fprintf(stderr, "TTF ERROR: %s\n", TTF_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); 

    if (w) {
      *w = surface->w;
    }

    if (h) {
      *h = surface->h;
    }

    SDL_FreeSurface(surface);

    if (!texture) {
        fprintf(stderr, "SDL_ERROR: %s\n", SDL_GetError());
        return NULL;
    }

    return texture;
}

Id 
graphic_createTextTexture(const char * const text, SDL_Color color)
{

    Index index;
    Id id;
    GET_NEXT_ID(textures, id, index, MAX_TEXTURES);

    textures.textures[index] = createTextTexture(text, color, NULL, NULL);
    return id;
}

Id 
graphic_createText(
    const char * const text, 
    int x, 
    int y,
    SDL_Color color) 
{ 
  SDL_Rect src, dest;
  unsigned int w, h;
  SDL_Texture* texture = createTextTexture(text, color, &w, &h);

  src.x = 0;
  src.y = 0;
  src.w = w;
  src.h = h;
  dest.x = x;
  dest.y = y;
  dest.w = w;
  dest.h = h;
  return createTilesetSprite(texture, src, dest);
}

Id graphic_createTextCentered(const char * const text, 
                              SDL_Rect zone, 
                              SDL_Color color)
{
  Id texture = graphic_createTextTexture(text, color);
  SDL_Rect src, dest;
  int w, h;
  graphic_queryTextureSize(texture, &w, &h);

  src.x = 0;
  src.y = 0;
  src.w = w;
  src.h = h;
  dest.x = zone.x + zone.w / 2 - w / 2;
  dest.y = zone.y + zone.h / 2 - h / 2;
  dest.w = w;
  dest.h = h;
  return graphic_createTilesetSprite(texture, src, dest);
}

Id graphic_createInvisbleText(char * const text, SDL_Color color)
{
  Id texture = graphic_createTextTexture(text, color);
  return graphic_createInvisibleSprite(texture);
}

void 
graphic_setText(
    Id id, 
    const char* const text,
    int x,
    int y,
    SDL_Color color) 
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);

    SDL_DestroyTexture(sprites.sprite[index].texture); 

    SDL_Rect src;
    src.x = 0;
    src.y = 0;
    sprites.sprite[index].texture = createTextTexture(text, 
                                                      color, 
                                                      (unsigned int*) &src.w,
                                                      (unsigned int*) &src.h);
    sprites.sprite[index].src = src;
    sprites.sprite[index].dest.x = x;
    sprites.sprite[index].dest.y = y;
    sprites.sprite[index].dest.w = src.w;
    sprites.sprite[index].dest.h = src.h;
}

void 
graphic_deleteText(Id id)
{
  Index index;
  GET_INDEX_FROM_ID(sprites, id, index);

  SDL_DestroyTexture(sprites.sprite[index].texture);

  graphic_deleteSprite(id);
}

void
graphic_clear()
{
    INIT_STRUCT_FOR_DOD_FREE_LIST(sprites, MAX_SPRITES);
    sprites.total = 0;
}

void
graphic_set_src_rect(
    Id id, 
    SDL_Rect src) 
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);
    sprites.sprite[index].src = src;
}

void 
graphic_centerSpriteOnScreen(Id id) 
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);

    int w, h;
    graphic_queryWindowSize(&w, &h);

    SDL_Rect* dest = &sprites.sprite[index].dest;
    dest->x = w / 2 - dest->w / 2;
    dest->y = h / 2 - dest->h / 2;
}

void 
graphic_centerSpriteOnScreenWidth(Id id) 
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);

    int w, h;
    graphic_queryWindowSize(&w, &h);

    SDL_Rect* dest = &sprites.sprite[index].dest;
    dest->x = w / 2 - dest->w / 2;
}

void 
graphic_centerSpriteOnScreenHeight(Id id) 
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);

    int w, h;
    graphic_queryWindowSize(&w, &h);

    SDL_Rect* dest = &sprites.sprite[index].dest;
    dest->y = h / 2 - dest->h / 2;
}

void
graphic_centerSpriteOnScreenWithOffset(Id id, int x, int y)
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);

    int w, h;
    graphic_queryWindowSize(&w, &h);

    SDL_Rect* dest = &sprites.sprite[index].dest;
    dest->x = w / 2 - dest->w / 2 + x;
    dest->y = h / 2 - dest->h / 2 + y;
}

void
graphic_centerSpriteOnScreenWidthWithOffset(Id id, int x)
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);

    int w, h;
    graphic_queryWindowSize(&w, &h);

    SDL_Rect* dest = &sprites.sprite[index].dest;
    dest->x = w / 2 - dest->w / 2 + x;
}

void
graphic_centerSpriteOnScreenHeightWithOffse(Id id, int y)
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);

    int w, h;
    graphic_queryWindowSize(&w, &h);

    SDL_Rect* dest = &sprites.sprite[index].dest;
    dest->y = h / 2 - dest->h / 2 + y;
}

void
graphic_setBackgroundTexture(Id textureId)
{
    unsigned int index;
    GET_INDEX_FROM_ID(textures, textureId, index);
    backgroundTextureIndex = index;
    backgroundSrc.x = 0;
    backgroundSrc.y = 0;
    SDL_QueryTexture(textures.textures[index], 
                     NULL, 
                     NULL, 
                     &backgroundSrc.w,
                     &backgroundSrc.h);
}

void graphic_resizeBackgroundToScreen()
{
    int backgroundTextureWidth;
    int backgroundTextureHeight;
    SDL_QueryTexture(textures.textures[backgroundTextureIndex], 
                     NULL, 
                     NULL, 
                     &backgroundTextureWidth,
                     &backgroundTextureHeight);
    double ratio = (double) backgroundTextureWidth / backgroundTextureHeight;
    int windowWidth, windowHeight;
    graphic_queryWindowSize(&windowWidth, &windowHeight);

    int diff = (windowWidth - windowHeight * ratio) / 2;
    if (diff < 0) 
    {
      backgroundSrc.x = -diff;
      backgroundSrc.w = windowWidth;
      backgroundDest.x = 0;
      backgroundDest.w = windowWidth;
    }
    else
    {
      backgroundSrc.x = 0;
      backgroundSrc.w = backgroundTextureWidth;
      backgroundDest.x = diff;
      backgroundDest.w = windowHeight * ratio;
    }
    backgroundDest.y = 0;
    backgroundDest.h = windowHeight;
}

Id
graphic_createSolidTexture(Uint32 color)
{
  SDL_Surface* surface = 
    SDL_CreateRGBSurfaceWithFormat(0,
                                   1,
                                   1,
                                   8,
                                   SDL_PIXELFORMAT_RGB888);

  SDL_Rect rect = {0, 0, 1, 1}; 
  SDL_FillRect(surface, &rect, color);
                    
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  Id textureId;
  Index textureIndex;
  GET_NEXT_ID(textures, textureId, textureIndex, MAX_TEXTURES);
  textures.textures[textureIndex] = texture;

  return textureId;
}

void
graphic_queryBackgroundDest(SDL_Rect *rect)
{
  *rect = backgroundDest;
}

void
graphic_querySpriteDest(Id id, SDL_Rect *rect)
{
  unsigned int index;
  GET_INDEX_FROM_ID(sprites, id, index);
  *rect = sprites.sprite[index].dest;
}

void 
graphic_setSpriteToInvisible(Id id)
{
    Index index, last;
    GET_INDEX_FROM_ID(sprites, id, index);
    if (index >= sprites.visibleSpriteTotal) {
        return;
    }

    last = --sprites.visibleSpriteTotal;
    if (id != sprites.ids[last]) {
        sprites.ids[index] = sprites.ids[last];
        sprites.indexes[sprites.ids[last]] = index;
        sprites.ids[last] = id;
        sprites.indexes[id] = last;
    }
    Sprite tmp = sprites.sprite[last];
    sprites.sprite[last] = sprites.sprite[index];
    sprites.sprite[index] = tmp;
}

void 
graphic_setSpriteToVisible(Id id)
{
    Index index, last;
    GET_INDEX_FROM_ID(sprites, id, index);
    if (index < sprites.visibleSpriteTotal) {
        return;
    }

    last = sprites.visibleSpriteTotal++;
    if (id != sprites.ids[last]) {
        sprites.ids[index] = sprites.ids[last];
        sprites.indexes[sprites.ids[last]] = index;
        sprites.ids[last] = id;
        sprites.indexes[id] = last;
    }
    Sprite tmp = sprites.sprite[last];
    sprites.sprite[last] = sprites.sprite[index];
    sprites.sprite[index] = tmp;
}

void
graphic_setSpriteDest(Id id, SDL_Rect dest)
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);

    sprites.sprite[index].dest = dest;
}

void
graphic_centerSpriteInRect(Id id, SDL_Rect rect)
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);
    int w, h;
    queryTextureSize(sprites.sprite[index].texture, &w, &h);
    SDL_Rect dest;

    dest.x = rect.x + rect.w / 2 - w / 2;
    dest.y = rect.y + rect.h / 2 - h / 2;
    dest.w = w;
    dest.h = h;

    sprites.sprite[index].dest = dest;
}

void
graphic_centerSpriteInRectButKeepRatio(Id id, SDL_Rect rect)
{
    Index index;
    GET_INDEX_FROM_ID(sprites, id, index);
    int w, h;
    queryTextureSize(sprites.sprite[index].texture, &w, &h);
    SDL_Rect dest;

    dest.x = rect.x;
    dest.y = rect.y;
    dest.w = rect.w;
    dest.h = rect.h;

    SDL_Rect src;
    src.x = (w - h) / 2;
    src.y = 0;
    src.w = h;
    src.h = h;
    sprites.sprite[index].src = src;
    sprites.sprite[index].dest = dest;
}

Id
graphic_createInvisibleSprite(Id textureId)
{
    Index index;
    Id id;
    GET_NEXT_ID(sprites, id, index, MAX_SPRITES);
    
    Index textureIndex;
    GET_INDEX_FROM_ID(textures, textureId, textureIndex);

    sprites.sprite[index].src.x = 0;
    sprites.sprite[index].src.y = 0;
    sprites.sprite[index].texture = textures.textures[textureIndex];
    queryTextureSize(textures.textures[textureIndex], &sprites.sprite[index].src.w, &sprites.sprite[index].src.h);

    sprites.sprite[index].dest.x = 0;
    sprites.sprite[index].dest.y = 0;
    sprites.sprite[index].dest.w = 0;
    sprites.sprite[index].dest.h = 0;

    return id;
}

