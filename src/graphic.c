#include <stdio.h>

#include "graphic.h"

#define MAX_TEXTURES 1024


static SDL_Window* window;
static SDL_Renderer* renderer;
static TTF_Font* font;

static struct {
    SDL_Texture* textures[MAX_TEXTURES];
    unsigned int w[MAX_TEXTURES];
    unsigned int h[MAX_TEXTURES];
    SET_STRUCT_FOR_DOD(TextureId, MAX_TEXTURES);
} textures;

static struct {
    unsigned char texture_index[MAX_SPRITES];
    SDL_Rect src[MAX_SPRITES];
    SDL_Rect dest[MAX_SPRITES];
    SET_STRUCT_FOR_DOD(SpriteId, MAX_SPRITES);
} sprites;

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

void graphic_render()
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);
    for (unsigned int i = 0; i < sprites.total; i++) {
        SDL_RenderCopy(
            renderer,
            textures.textures[sprites.texture_index[i]],
            &sprites.src[i],
            &sprites.dest[i]);
    }
    SDL_RenderPresent(renderer);
}

TextureId graphic_loadTexture(const char* const filename)
{
  SDL_Surface* surface = IMG_Load(filename);
  Uint32 colorkey = SDL_MapRGB(surface->format, 0xff, 0, 0xff);
  SDL_SetColorKey(surface, SDL_TRUE, colorkey);

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  unsigned int w = surface->w, h = surface->h;
  SDL_FreeSurface(surface);
  if (texture == NULL) 
  {
    fprintf(stderr, "Texture %s could not be loaded! SDL_Error: %s\n", filename, SDL_GetError());
    exit(EXIT_FAILURE);
  }

  SpriteId index, id;
  GET_NEXT_ID(textures, id, index, MAX_TEXTURES);
  textures.textures[index] = texture;
  textures.w[index] = w;
  textures.h[index] = h;

  return id;
}

SpriteId 
graphic_createTilesetSprite( 
  TextureId texture_id, 
  SDL_Rect src, 
  SDL_Rect dest
) {
  TextureId texture_index;
  GET_INDEX_FROM_ID(textures, texture_id, texture_index);

  SpriteId index, id;
  GET_NEXT_ID(sprites, id, index, MAX_SPRITES);

  sprites.texture_index[index] = texture_index;

  sprites.src[index] = src;
  sprites.dest[index] = dest;

  return id;
}

SpriteId 
graphic_createFullTextureSprite( 
  TextureId texture_id, 
  SDL_Rect dest
) {
  TextureId texture_index = textures.indexes[texture_id];

  SpriteId index, id;
  GET_NEXT_ID(sprites, id, index, MAX_SPRITES);

  sprites.texture_index[index] = texture_index;

  SDL_Rect src = {0};
  src.w = textures.w[texture_index];
  src.h = textures.h[texture_index];
  sprites.src[index] = src;
  sprites.dest[index] = dest;

  return id;
}

void
graphic_setSpriteSize(SpriteId id, int w, int h)
{
  SpriteId index;
  GET_INDEX_FROM_ID(sprites, id, index);
  sprites.dest[index].w = w;
  sprites.dest[index].h = h;
}

void
graphic_translateSprite(SpriteId id, int x, int y)
{
  SpriteId index;
  GET_INDEX_FROM_ID(sprites, id, index);
  sprites.dest[index].x += x;
  sprites.dest[index].y += y;
}

void
graphic_deleteSprite(SpriteId id) {
  SpriteId index, last;
  DELETE_DOD_ELEMENT_BY_ID(sprites, id, index, last);

  sprites.texture_index[index] = sprites.texture_index[last];
  sprites.dest[index] = sprites.dest[last];
  sprites.src[index] = sprites.src[last];
}

void 
graphic_queryTextureSize(TextureId texture_id, int* w, int* h)
{
  TextureId index;
  GET_INDEX_FROM_ID(textures, texture_id, index);
  SDL_Texture* texture = textures.textures[index];
  SDL_QueryTexture(texture, NULL, NULL, w, h);
}

void
graphic_quit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    for( int i = 0; i < MAX_TEXTURES; i++ )
    {
      if( textures.textures[i] )
      {
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

void graphic_queryPosition(SpriteId id, int * x, int* y)
{
  SpriteId index;
  GET_INDEX_FROM_ID(sprites, id, index);
  (*x) = sprites.dest[index].x;
  (*y) = sprites.dest[index].y;
}

void graphic_setPosition(SpriteId id, int x, int y)
{
  SpriteId index;
  GET_INDEX_FROM_ID(sprites, id, index);
  sprites.dest[index].x = x;
  sprites.dest[index].y = y;
}

SpriteTextId 
graphic_createText(
  const char * const text, 
  int x, 
  int y,
  SDL_Color color
) {
  SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);

  if (!surface) {
    fprintf(stderr, "TTF ERROR: %s\n", TTF_GetError());
    return -1;
  }

  SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, surface); 

  SDL_Rect src = {0};
  src.w = surface->w;
  src.h = surface->h;
  SDL_FreeSurface(surface);

  if (!text_texture) {
    fprintf(stderr, "SDL_ERROR: %s\n", SDL_GetError());
    return -1;
  }

  TextureId texture_index, texture_id;
  GET_NEXT_ID(textures, texture_id, texture_index, MAX_TEXTURES);

  textures.textures[texture_index] = text_texture;
  textures.w[texture_index] = src.w;
  textures.h[texture_index] = src.h;

  SpriteId index, id;
  GET_NEXT_ID(sprites, id, index, MAX_SPRITES);

  sprites.texture_index[index] = texture_index;
  sprites.src[index] = src;
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  dest.w = src.w;
  dest.h = src.h;
  sprites.dest[index].x = x;
  sprites.dest[index].y = y;
  sprites.dest[index].w = src.w;
  sprites.dest[index].h = src.h;

  return id;
}

void 
graphic_setText(
  SpriteTextId id, 
  const char* const text,
  int x,
  int y,
  SDL_Color color
) {
  SpriteId index;
  GET_INDEX_FROM_ID(sprites, id, index);

  TextureId texture_index = sprites.texture_index[index];
  SDL_DestroyTexture(textures.textures[texture_index]);

  SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
  if (!surface) {
    return;
  }

  SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, surface); 

  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  src.w = surface->w;
  src.h = surface->h;
  SDL_FreeSurface(surface);

  if (!text_texture) {
    return;
  }

  textures.textures[texture_index] = text_texture;
  textures.w[texture_index] = src.w;
  textures.h[texture_index] = src.h;

  sprites.src[index] = src;
  sprites.dest[index].x = x;
  sprites.dest[index].y = y;
  sprites.dest[index].w = src.w;
  sprites.dest[index].h = src.h;
}

void graphic_deleteText(SpriteTextId id)
{

}

void
graphic_clear()
{
    INIT_STRUCT_FOR_DOD_FREE_LIST(sprites, MAX_SPRITES);
    sprites.total = 0;
}

void
graphic_set_src_rect(
  SpriteTextId id, 
  SDL_Rect src
) {
  SpriteId index;
  GET_INDEX_FROM_ID(sprites, id, index);
  sprites.src[index] = src;
}

void 
graphic_centerSpriteOnScreen(SpriteId id) {
  unsigned int index;
  GET_INDEX_FROM_ID(sprites, id, index);

  int w, h;
  graphic_queryWindowSize(&w, &h);

  SDL_Rect* dest = &sprites.dest[index];
  dest->x = w / 2 - dest->w / 2;
  dest->y = h / 2 - dest->h / 2;
}

void 
graphic_centerSpriteOnScreenWidth(SpriteId id) {
  unsigned int index;
  GET_INDEX_FROM_ID(sprites, id, index);

  int w, h;
  graphic_queryWindowSize(&w, &h);

  SDL_Rect* dest = &sprites.dest[index];
  dest->x = w / 2 - dest->w / 2;
}

void 
graphic_centerSpriteOnScreenHeight(SpriteId id) {
  unsigned int index;
  GET_INDEX_FROM_ID(sprites, id, index);

  int w, h;
  graphic_queryWindowSize(&w, &h);

  SDL_Rect* dest = &sprites.dest[index];
  dest->y = h / 2 - dest->h / 2;
}

void
graphic_centerSpriteOnScreenWithOffset(SpriteId id, int x, int y)
{
  unsigned int index;
  GET_INDEX_FROM_ID(sprites, id, index);

  int w, h;
  graphic_queryWindowSize(&w, &h);

  SDL_Rect* dest = &sprites.dest[index];
  dest->x = w / 2 - dest->w / 2 + x;
  dest->y = h / 2 - dest->h / 2 + y;
}

void
graphic_centerSpriteOnScreenWidthWithOffset(SpriteId id, int x)
{
  unsigned int index;
  GET_INDEX_FROM_ID(sprites, id, index);

  int w, h;
  graphic_queryWindowSize(&w, &h);

  SDL_Rect* dest = &sprites.dest[index];
  dest->x = w / 2 - dest->w / 2 + x;
}

void
graphic_centerSpriteOnScreenHeightWithOffse(SpriteId id, int y)
{
  unsigned int index;
  GET_INDEX_FROM_ID(sprites, id, index);

  int w, h;
  graphic_queryWindowSize(&w, &h);

  SDL_Rect* dest = &sprites.dest[index];
  dest->y = h / 2 - dest->h / 2 + y;
}
