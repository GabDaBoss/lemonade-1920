#include <stdio.h>

#include "graphic.h"

#define MAX_TEXTURES 1024

typedef struct {
    SDL_Texture* texture;
    SDL_Rect src;
    SDL_Rect dest;
} _Sprite;

typedef struct {
  double x, y, w, h;
} RectF;

static SDL_Window* _window;
static SDL_Renderer* _renderer;
static TTF_Font* _font;

static struct {
    SDL_Texture* textures[MAX_TEXTURES];
    SET_STRUCT_FOR_DOD(Id, MAX_TEXTURES);
} _textures;

static struct {
  _Sprite sprite[MAX_SPRITES];
  RectF rectF[MAX_SPRITES];
  unsigned int totalActive;
  SET_STRUCT_FOR_DOD(Id, MAX_SPRITES);
} _sprites;

static struct {
  int x, y;
  double zoom;
  struct {
    int x, y, w, h;
    bool dirty;
  } bounds;
} _camera;

static void
_updateCameraBoundLeft(int x)
{
  if (x < _camera.bounds.x) {
    _camera.bounds.w += _camera.bounds.x - x;
    _camera.bounds.x = x;
  } 
}

static void
_updateCameraBoundRight(int x, int w)
{
  int dw = (x + w) - (_camera.bounds.x + _camera.bounds.w);
  if (dw > 0) {
    _camera.bounds.w += dw;
  }
}

static void
_updateCameraBoundTop(int y)
{
  if (y < _camera.bounds.x) {
    _camera.bounds.h += _camera.bounds.y - y;
    _camera.bounds.y = y;
  } 
}

static void
_updateCameraBoundBottom(int y, int h)
{
  int dh = (y + h) - (_camera.bounds.y + _camera.bounds.h);
  if (dh > 0) {
    _camera.bounds.h += dh;
  }
}

static void
_updateCameraBounds()
{
  if (_camera.bounds.dirty) {
    _camera.bounds.x = 0;
    _camera.bounds.y = 0;
    _camera.bounds.w = 0;
    _camera.bounds.h = 0;
    _camera.bounds.dirty = false;

    for (Index i = 0; i < _sprites.totalActive; i++) {
      _updateCameraBoundLeft(_sprites.sprite[i].dest.x / _camera.zoom);
      _updateCameraBoundRight(
        _sprites.sprite[i].dest.x / _camera.zoom,
        _sprites.sprite[i].dest.w / _camera.zoom
      );
      _updateCameraBoundTop(_sprites.sprite[i].dest.y / _camera.zoom);
      _updateCameraBoundBottom(
        _sprites.sprite[i].dest.y / _camera.zoom, 
        _sprites.sprite[i].dest.h / _camera.zoom
      );
    }
  }
}

static SDL_Rect 
_applyCameraToDest(SDL_Rect dest)
{
  dest.x -= _camera.x;
  dest.y -= _camera.y;
  dest.x *= _camera.zoom;
  dest.y *= _camera.zoom;
  dest.w *= _camera.zoom;
  dest.h *= _camera.zoom;

  return dest;
}

static void
_swap(Index it, Id id, Index with)
{
    _sprites.sprite[it] = _sprites.sprite[with];
    _sprites.rectF[it] = _sprites.rectF[with];
    _sprites.indexes[_sprites.ids[with]] = it;
    _sprites.ids[with] = id;
}

static RectF
_convertRectToRectF(SDL_Rect rect)
{
  RectF rectF;
  rectF.x = rect.x;
  rectF.y = rect.y;
  rectF.w = rect.w;
  rectF.h = rect.h;
  return rectF;
}

static Id 
_createTilesetSprite(SDL_Texture* texture, SDL_Rect src, SDL_Rect dest) 
{
  Index index;
  Id id;
  GET_NEXT_ID(_sprites, id, index, MAX_SPRITES);

  if (_sprites.totalActive < index) {
    _swap(index, id, _sprites.totalActive);
  }

  _sprites.sprite[_sprites.totalActive].src = src;
  _sprites.rectF[_sprites.totalActive] = _convertRectToRectF(dest);
  _sprites.sprite[_sprites.totalActive].dest = _applyCameraToDest(dest);
  _sprites.sprite[_sprites.totalActive].texture = texture;
  _sprites.totalActive++;

  return id;
}

static SDL_Texture*
_createTextTexture(
  const char * const text, 
  SDL_Color color, 
  unsigned int *w, 
  unsigned int *h)
{
  SDL_Surface* surface = TTF_RenderText_Solid(_font, text, color);

  if (!surface) {
    fprintf(stderr, "TTF ERROR: %s\n", TTF_GetError());
    return NULL;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface); 

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

bool 
Graphic_Init(const char * const title, int w, int h, int font_size) 
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL couldn't initialize! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  if (IMG_Init(IMG_INIT_PNG) == 0) {
    fprintf(stderr, "SDL image couldn't initialize! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  if (TTF_Init() != 0) {
    fprintf(stderr, "SDL ttf couldn't initialize! SDL_Error: %s\n", SDL_GetError());
    return false;
  };

  _font = TTF_OpenFont(FONT, font_size);
  if (!_font) {
    fprintf(stderr, "SDL ttf couldn't open the font! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  _window = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    w,
    h,
    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
  );

  if (_window == NULL) {
    fprintf(stderr, "Window couldn't be created! SDL_Error: %s\n", SDL_GetError());
    return false;
  }


  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

  if (_renderer == NULL) {
    fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);


  SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

  INIT_STRUCT_FOR_DOD_FREE_LIST(_sprites, MAX_SPRITES);
  INIT_STRUCT_FOR_DOD_FREE_LIST(_textures, MAX_TEXTURES);

  return true;
}

void 
Graphic_Render() 
{
  SDL_SetRenderDrawColor(_renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(_renderer);

  for (unsigned int i = 0; i < _sprites.totalActive; i++) {
    SDL_RenderCopy(
      _renderer,
      _sprites.sprite[i].texture,
      &_sprites.sprite[i].src,
      &_sprites.sprite[i].dest
    );
  }
  SDL_RenderPresent(_renderer);
}

Id 
Graphic_LoadTexture(const char* const filename) 
{
  SDL_Surface* surface = IMG_Load(filename);
  Uint32 colorkey = SDL_MapRGB(surface->format, 0xff, 0, 0xff);
  SDL_SetColorKey(surface, SDL_TRUE, colorkey);

  SDL_Texture *texture = SDL_CreateTextureFromSurface(_renderer, surface);
  SDL_FreeSurface(surface);
  if (texture == NULL) {
    fprintf(
      stderr, "Texture %s could not be loaded! SDL_Error: %s\n", 
      filename, 
      SDL_GetError()
    );
    exit(EXIT_FAILURE);
  }

  Index index;
  Id id;
  GET_NEXT_ID(_textures, id, index, MAX_TEXTURES);
  _textures.textures[index] = texture;

  return id;
}


Id 
Graphic_CreateTilesetSprite(Id texture_id, SDL_Rect src, SDL_Rect dest) 
{
  Index texture_index;
  GET_INDEX_FROM_ID(_textures, texture_id, texture_index);

  _camera.bounds.dirty = true;
  return _createTilesetSprite(_textures.textures[texture_index], src, dest);
}

Id 
Graphic_CreateFullTextureSprite(Id texture_id, SDL_Rect dest) 
{
  Index texture_index;
  GET_INDEX_FROM_ID(_textures, texture_id, texture_index);
  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  SDL_QueryTexture(
    _textures.textures[texture_index], 
    NULL, 
    NULL, 
    &src.w, 
    &src.h
  );

  _camera.bounds.dirty = true;
  return Graphic_CreateTilesetSprite(texture_id, src, dest);
}

void
Graphic_SetSpriteSize(Id id, int w, int h) 
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  _sprites.rectF[index].w = w;
  _sprites.rectF[index].h = h;
  _sprites.sprite[index].dest.w = w * _camera.zoom;
  _sprites.sprite[index].dest.h = h * _camera.zoom;
  _camera.bounds.dirty = true;
}

void
Graphic_TranslateSprite(Id id, int x, int y) 
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  _sprites.rectF[index].x = (int) _sprites.rectF[index].x + x;
  _sprites.rectF[index].y = (int) _sprites.rectF[index].y + y;
  _sprites.sprite[index].dest.x += x * _camera.zoom;
  _sprites.sprite[index].dest.y += y * _camera.zoom;
  _camera.bounds.dirty = true;
}

void
Graphic_DeleteSprite(Id id) 
{
  Index index, last;

  GET_INDEX_FROM_ID(_sprites, id, index);

  _sprites.indexes[id] = _sprites.next_free_index;
  _sprites.next_free_index = id;

  if (index < _sprites.totalActive) {
    last = --_sprites.totalActive;
    if (id != _sprites.ids[last]) {
      _sprites.ids[index] = _sprites.ids[last];
      _sprites.indexes[_sprites.ids[last]] = index;
    }
    _sprites.sprite[index] = _sprites.sprite[last];
    _sprites.rectF[index] = _sprites.rectF[last];
    index = last;
    id = _sprites.ids[last];
  } 


  last = --_sprites.total;
  if (id != _sprites.ids[last]) {
    _sprites.ids[index] = _sprites.ids[last];
    _sprites.indexes[_sprites.ids[last]] = index;
  }
  _sprites.sprite[index] = _sprites.sprite[last];
  _sprites.rectF[index] = _sprites.rectF[last];
  _camera.bounds.dirty = true;
}


void 
Graphic_QueryTextureSize(Id texture_id, int* w, int* h) 
{
  Id index;
  GET_INDEX_FROM_ID(_textures, texture_id, index);
  SDL_Texture* texture = _textures.textures[index];
  SDL_QueryTexture(texture, NULL, NULL, w, h);
}

void
Graphic_Quit()
{
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);
  for (Index i = 0; i < _textures.total; i++ ) {
    if (_textures.textures[i]) {
      SDL_DestroyTexture(_textures.textures[i]);
    }
  }

  TTF_CloseFont(_font);
  TTF_Quit();

  IMG_Quit();
  SDL_Quit();
}

void 
Graphic_QueryWindowSize(int* w, int* h)
{
  SDL_GetWindowSize(_window, w, h);
}

void Graphic_QueryPosition(Id id, int * x, int* y)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);
  (*x) = _sprites.sprite[index].dest.x / _camera.zoom + _camera.x;
  (*y) = _sprites.sprite[index].dest.y / _camera.zoom + _camera.y;
}

void Graphic_SetPosition(Id id, int x, int y)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);
  _sprites.sprite[index].dest.x = (x - _camera.x) * _camera.zoom;
  _sprites.sprite[index].dest.y = (y - _camera.y) * _camera.zoom;
  _sprites.rectF[index].x = x;
  _sprites.rectF[index].y = y;
  _camera.bounds.dirty = true;
}


Id 
Graphic_CreateTextTexture(const char * const text, SDL_Color color)
{
  Index index;
  Id id;
  GET_NEXT_ID(_textures, id, index, MAX_TEXTURES);

  _textures.textures[index] = _createTextTexture(text, color, NULL, NULL);
  _camera.bounds.dirty = true;
  return id;
}

Id 
Graphic_CreateText(
  const char * const text, 
  int x, 
  int y,
  SDL_Color color) 
{ 
  SDL_Rect src, dest;
  unsigned int w, h;
  SDL_Texture* texture = _createTextTexture(text, color, &w, &h);

  src.x = 0;
  src.y = 0;
  src.w = w;
  src.h = h;
  dest.x = (x - _camera.x) * _camera.zoom;
  dest.y = (y - _camera.x) * _camera.zoom;
  dest.w = w * _camera.zoom;
  dest.h = h * _camera.zoom;
  _camera.bounds.dirty = true;
  return _createTilesetSprite(texture, src, dest);
}

Id 
Graphic_CreateTextCentered(
  const char * const text, 
  SDL_Rect zone, 
  SDL_Color color)
{
  Id texture = Graphic_CreateTextTexture(text, color);
  SDL_Rect src, dest;
  int w, h;
  Graphic_QueryTextureSize(texture, &w, &h);

  src.x = 0;
  src.y = 0;
  src.w = w;
  src.h = h;
  dest.x = zone.x + zone.w / 2 - w / 2;
  dest.y = zone.y + zone.h / 2 - h / 2;
  dest.w = w;
  dest.h = h;
  return Graphic_CreateTilesetSprite(texture, src, dest);
}

Id 
Graphic_CreateInactiveText(char * const text, SDL_Color color)
{
  Id texture = Graphic_CreateTextTexture(text, color);
  return Graphic_CreateInactiveSprite(texture);
}

void 
Graphic_SetText(
  Id id, 
  const char* const text,
  int x,
  int y,
  SDL_Color color) 
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  SDL_DestroyTexture(_sprites.sprite[index].texture); 

  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  _sprites.sprite[index].texture = _createTextTexture(
    text, 
    color, 
    (unsigned int*) &src.w,
    (unsigned int*) &src.h
  );
  _sprites.sprite[index].src = src;
  _sprites.rectF[index].x = x;
  _sprites.rectF[index].y = y;
  _sprites.sprite[index].dest.x = (x - _camera.x) * _camera.zoom;
  _sprites.sprite[index].dest.y = (y - _camera.x) * _camera.zoom;
  _sprites.sprite[index].dest.w = src.w * _camera.zoom;
  _sprites.sprite[index].dest.h = src.h * _camera.zoom;
  _camera.bounds.dirty = true;
}

void 
Graphic_DeleteText(Id id)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  SDL_DestroyTexture(_sprites.sprite[index].texture);

  Graphic_DeleteSprite(id);
  _camera.bounds.dirty = true;
}

void
Graphic_Clear()
{
  INIT_STRUCT_FOR_DOD_FREE_LIST(_sprites, MAX_SPRITES);
  INIT_STRUCT_FOR_DOD_FREE_LIST(_textures, MAX_TEXTURES);
  _sprites.totalActive = 0;
  _sprites.total = 0;
  _textures.total = 0;
  _camera.bounds.dirty = true;
}

void 
Graphic_SetSpriteSrcAndDest(Id id, SDL_Rect src, SDL_Rect dest)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);
  _sprites.rectF[index] = _convertRectToRectF(dest);
  _sprites.sprite[index].src = src;
  _sprites.sprite[index].dest = _applyCameraToDest(dest);
  _camera.bounds.dirty = true;
}

void
Graphic_SetSpriteSrcRect(Id id, SDL_Rect src) 
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);
  _sprites.sprite[index].src = src;
}

void 
Graphic_CenterSpriteOnScreen(Id id) 
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  SDL_Rect* dest = &_sprites.sprite[index].dest;
  dest->x = w / 2 - dest->w / 2;
  dest->y = h / 2 - dest->h / 2;
  _sprites.rectF[index].x = dest->x;
  _sprites.rectF[index].y = dest->y;
  _camera.bounds.dirty = true;
}

void 
Graphic_CenterSpriteOnScreenWidth(Id id) 
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  SDL_Rect* dest = &_sprites.sprite[index].dest;
  dest->x = w / 2 - dest->w / 2;
  _sprites.rectF[index].x = dest->x;
  _camera.bounds.dirty = true;
}

void 
Graphic_CenterSpriteOnScreenHeight(Id id) 
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  SDL_Rect* dest = &_sprites.sprite[index].dest;
  dest->y = h / 2 - dest->h / 2;
  _sprites.rectF[index].y = dest->y;
  _camera.bounds.dirty = true;
}

void
Graphic_CenterSpriteOnScreenWithOffset(Id id, int x, int y)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  SDL_Rect* dest = &_sprites.sprite[index].dest;
  dest->x = w / 2 - dest->w / 2 + x;
  dest->y = h / 2 - dest->h / 2 + y;
  _sprites.rectF[index].x = dest->x;
  _sprites.rectF[index].y = dest->y;
  _camera.bounds.dirty = true;
}

void
Graphic_CenterSpriteOnScreenWidthWithOffset(Id id, int x)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  SDL_Rect* dest = &_sprites.sprite[index].dest;
  dest->x = w / 2 - dest->w / 2 + x;
  _sprites.rectF[index].x = dest->x;
  _camera.bounds.dirty = true;
}

void
Graphic_CenterSpriteOnScreenHeightWithOffset(Id id, int y)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  SDL_Rect* dest = &_sprites.sprite[index].dest;
  dest->y = h / 2 - dest->h / 2 + y;
  _sprites.rectF[index].y = dest->y;
  _camera.bounds.dirty = true;
}

void 
Graphic_ResizeSpriteToScreen(Id id)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);
  int backgroundTextureWidth;
  int backgroundTextureHeight;
  SDL_QueryTexture(
    _sprites.sprite[index].texture, 
    NULL, 
    NULL, 
    &backgroundTextureWidth,
    &backgroundTextureHeight
  );
  double ratio = (double) backgroundTextureWidth / backgroundTextureHeight;
  int windowWidth, windowHeight;
  Graphic_QueryWindowSize(&windowWidth, &windowHeight);

  int diff = (windowWidth - windowHeight * ratio) / 2;
  if (diff < 0) 
  {
    _sprites.sprite[index].src.x = -diff;
    _sprites.sprite[index].src.w = windowWidth;
    _sprites.sprite[index].dest.x = 0;
    _sprites.sprite[index].dest.w = windowWidth;
  }
  else
  {
    _sprites.sprite[index].src.x = 0;
    _sprites.sprite[index].src.w = backgroundTextureWidth;
    _sprites.sprite[index].dest.x = diff;
    _sprites.sprite[index].dest.w = windowHeight * ratio;
  }
  _sprites.sprite[index].dest.y = 0;
  _sprites.sprite[index].dest.h = windowHeight;
  _sprites.rectF[index] = _convertRectToRectF(_sprites.sprite[index].dest);
  _camera.bounds.dirty = true;
}

Id
Graphic_CreateSolidTexture(Uint32 color)
{
  SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
    0,
    1,
    1,
    8,
    SDL_PIXELFORMAT_RGB888
  );

  SDL_Rect rect = {0, 0, 1, 1}; 
  SDL_FillRect(surface, &rect, color);
                    
  SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);
  SDL_FreeSurface(surface);

  Id textureId;
  Index textureIndex;
  GET_NEXT_ID(_textures, textureId, textureIndex, MAX_TEXTURES);
  _textures.textures[textureIndex] = texture;

  return textureId;
}

void
Graphic_QuerySpriteDest(Id id, SDL_Rect *rect)
{
  unsigned int index;
  GET_INDEX_FROM_ID(_sprites, id, index);
  *rect = _sprites.sprite[index].dest;
  rect->x /= _camera.zoom;
  rect->y /= _camera.zoom;
  rect->w /= _camera.zoom;
  rect->h /= _camera.zoom;
  rect->x += _camera.x;
  rect->y += _camera.y;
}

void 
Graphic_SetSpriteToInactive(Id id)
{
  Index index, last;
  GET_INDEX_FROM_ID(_sprites, id, index);
  if (index >= _sprites.totalActive) {
    return;
  }

  last = --_sprites.totalActive;
  if (id != _sprites.ids[last]) {
    _sprites.ids[index] = _sprites.ids[last];
    _sprites.indexes[_sprites.ids[last]] = index;
    _sprites.ids[last] = id;
    _sprites.indexes[id] = last;
  }
  _Sprite tmp = _sprites.sprite[last];
  RectF tRectF = _sprites.rectF[last];
  _sprites.sprite[last] = _sprites.sprite[index];
  _sprites.rectF[last] = _sprites.rectF[index];
  _sprites.sprite[index] = tmp;
  _sprites.rectF[index] = tRectF;
  _camera.bounds.dirty = true;
}

void 
Graphic_SetSpriteToActive(Id id)
{
  Index index, last;
  GET_INDEX_FROM_ID(_sprites, id, index);
  if (index < _sprites.totalActive) {
    return;
  }

  last = _sprites.totalActive++;
  if (id != _sprites.ids[last]) {
    _sprites.ids[index] = _sprites.ids[last];
    _sprites.indexes[_sprites.ids[last]] = index;
    _sprites.ids[last] = id;
    _sprites.indexes[id] = last;
  }
  _Sprite tmp = _sprites.sprite[last];
  RectF tRectF = _sprites.rectF[last];
  _sprites.sprite[last] = _sprites.sprite[index];
  _sprites.rectF[last] = _sprites.rectF[index];
  _sprites.sprite[index] = tmp;
  _sprites.rectF[index] = tRectF;
  _camera.bounds.dirty = true;
}

void
Graphic_SetSpriteDest(Id id, SDL_Rect dest)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);

  _sprites.sprite[index].dest = _applyCameraToDest(dest);
  _sprites.rectF[index] = _convertRectToRectF(dest);
  _camera.bounds.dirty = true;
}

void
Graphic_CenterSpriteInRect(Id id, SDL_Rect rect)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);
  int w, h;
  SDL_QueryTexture(
    _sprites.sprite[index].texture, 
    NULL, 
    NULL, 
    &w, 
    &h
  );

  RectF rectF;
  rectF.x = rect.x + rect.w / 2 - w / 2;
  rectF.y = rect.y + rect.h / 2 - h / 2;
  rectF.w = w;
  rectF.h = h;
  SDL_Rect dest;

  dest.x = (rectF.x - _camera.x) * _camera.zoom;
  dest.y = (rectF.y - _camera.y) * _camera.zoom;
  dest.w = rectF.w * _camera.zoom;
  dest.h = rectF.h * _camera.zoom;

  _sprites.sprite[index].dest = dest;
  _sprites.rectF[index] = rectF;
  _camera.bounds.dirty = true;
}

void
Graphic_CenterSpriteInRectButKeepRatio(Id id, SDL_Rect rect)
{
  Index index;
  GET_INDEX_FROM_ID(_sprites, id, index);
  int w, h;
  SDL_QueryTexture(
    _sprites.sprite[index].texture, 
    NULL, 
    NULL, 
    &w, 
    &h
  );
  SDL_Rect dest;

  _sprites.rectF[index] = _convertRectToRectF(rect);

  dest.x = (rect.x - _camera.x) * _camera.zoom;
  dest.y = (rect.y - _camera.y) * _camera.zoom;
  dest.w = rect.w;
  dest.h = rect.h;

  SDL_Rect src;
  src.x = (w - h) / 2;
  src.y = 0;
  src.w = h;
  src.h = h;
  _sprites.sprite[index].src = src;
  _sprites.sprite[index].dest = dest;
  _camera.bounds.dirty = true;
}

Id
Graphic_CreateInactiveSprite(Id textureId)
{
  Index index;
  Id id;
  GET_NEXT_ID(_sprites, id, index, MAX_SPRITES);
  
  Index textureIndex;
  GET_INDEX_FROM_ID(_textures, textureId, textureIndex);

  _sprites.sprite[index].src.x = 0;
  _sprites.sprite[index].src.y = 0;
  _sprites.sprite[index].texture = _textures.textures[textureIndex];
  SDL_QueryTexture(
    _textures.textures[textureIndex], 
    NULL, 
    NULL, 
    &_sprites.sprite[index].src.w,
    &_sprites.sprite[index].src.h
  );

  _sprites.sprite[index].dest.x = 0;
  _sprites.sprite[index].dest.y = 0;
  _sprites.sprite[index].dest.w = 0;
  _sprites.sprite[index].dest.h = 0;
  _sprites.rectF[index].x = 0;
  _sprites.rectF[index].y = 0;
  _sprites.rectF[index].w = 0;
  _sprites.rectF[index].h = 0;

  return id;
}

void 
Graphic_DeleteTexture(Id id)
{
  Index idx, last;
  DELETE_DOD_ELEMENT_BY_ID(_textures, id, idx, last);
  SDL_Texture* ptr = _textures.textures[idx];
  SDL_DestroyTexture(ptr);
  
  for (Index i = 0; i < _sprites.total; i++)
  {
    if (_sprites.sprite[i].texture != ptr)
    {
      continue;
    }

    Index spriteId, spriteLast;
    DELETE_DOD_ELEMENT_BY_INDEX(_sprites, spriteId, i, spriteLast);
    if (i < _sprites.totalActive - 1)
    {
      _sprites.totalActive--;
      _sprites.sprite[i] = _sprites.sprite[_sprites.totalActive];
      _sprites.rectF[i] = _sprites.rectF[_sprites.totalActive];
      _sprites.sprite[_sprites.totalActive] = _sprites.sprite[spriteLast];
      _sprites.rectF[_sprites.totalActive] = _sprites.rectF[spriteLast];
    } 
    else 
    { 
      if (i == _sprites.totalActive - 1) 
      {
        _sprites.totalActive--;
      }
      _sprites.sprite[i] = _sprites.sprite[spriteLast];
      _sprites.rectF[i] = _sprites.rectF[spriteLast];
    }
    i--;
    _camera.bounds.dirty = true;
  }
}

void
Graphic_TranslateAllSprite(int dx, int dy)
{
  _camera.bounds.x += dx;
  _camera.bounds.y += dy;
  for (Index i = 0; i < _sprites.totalActive; i++) {
    _sprites.sprite[i].dest.x += dx * _camera.zoom;
    _sprites.sprite[i].dest.y += dy * _camera.zoom;
    // _sprites.rectF[i].x += dx;
    // _sprites.rectF[i].y += dy;
  }
}

void 
Graphic_SetSpriteToBeAfterAnother(Id id, Id other)
{
  Index idx, otherIdx;
  GET_INDEX_FROM_ID(_sprites, id, idx);
  GET_INDEX_FROM_ID(_sprites, other, otherIdx);

  _Sprite sprite = _sprites.sprite[idx];
  RectF rectF = _sprites.rectF[idx];
  if (idx == otherIdx) {
    return;
  }

  if (idx < otherIdx) {
    for (Index i = idx; i < otherIdx; i++) {
      _sprites.sprite[i] = _sprites.sprite[i + 1];
      _sprites.rectF[i] = _sprites.rectF[i + 1];
      _sprites.ids[i] = _sprites.ids[i + 1];
      _sprites.indexes[_sprites.ids[i]] = i;
    }

    _sprites.sprite[otherIdx] = sprite;
    _sprites.rectF[otherIdx] = rectF;
    _sprites.ids[otherIdx] = id;
    _sprites.indexes[id] = otherIdx;
  } else {
    for (Index i = idx; i > otherIdx + 1; i--) {
      _sprites.sprite[i] = _sprites.sprite[i - 1];
      _sprites.rectF[i] = _sprites.rectF[i - 1];
      _sprites.ids[i] = _sprites.ids[i - 1];
      _sprites.indexes[_sprites.ids[i]] = i;
    }

    _sprites.sprite[otherIdx + 1] = sprite;
    _sprites.rectF[otherIdx + 1] = rectF;
    _sprites.ids[otherIdx + 1] = id;
    _sprites.indexes[id] = otherIdx + 1;
  }
}

void 
Graphic_ZoomSprites(double zoom)
{
  _camera.zoom *= zoom;
  _camera.bounds.x *= zoom;
  _camera.bounds.y *= zoom;
  _camera.bounds.w *= zoom;
  _camera.bounds.h *= zoom;
  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  for (Index i = 0; i < _sprites.total; i++) {
    _sprites.sprite[i].dest.x = 
      (_sprites.rectF[i].x - _camera.x) * _camera.zoom - 
      w / 2 * (_camera.zoom - 1);
    _sprites.sprite[i].dest.y = 
      (_sprites.rectF[i].y - _camera.y) * _camera.zoom -
      h / 2 * (_camera.zoom - 1);
    
    _sprites.sprite[i].dest.w *= zoom;
    _sprites.sprite[i].dest.h *= zoom;
  }
}

void 
Graphic_MoveCamera(int dx, int dy)
{
  _updateCameraBounds();
  
  int w, h;
  Graphic_QueryWindowSize(&w, &h);
  w /= _camera.zoom;
  h /= _camera.zoom;

  if ((_camera.bounds.x >= 0 && dx > 0) ||
      (_camera.bounds.x + _camera.bounds.w <= w && dx < 0)) {
    dx = 0;
  }

  if ((_camera.bounds.y >= 0 && dy > 0) ||
      (_camera.bounds.y + _camera.bounds.h <= h && dy < 0)) {
    dy = 0;
  }

  if (!dx && !dy) {
    return;
  }
  
  _camera.x -= dx;
  _camera.y -= dy;
  Graphic_TranslateAllSprite(dx, dy);
}

void 
Graphic_InitCamera()
{
  _camera.x = 0;
  _camera.y = 0;
  _camera.bounds.x = 0;
  _camera.bounds.y = 0;
  _camera.bounds.w = 0;
  _camera.bounds.h = 0;
  _camera.zoom = 1.0;
  _camera.bounds.dirty = false;
}

double
Graphic_GetCameraZoom()
{
  return _camera.zoom;
}

void 
Graphic_TranslateSpriteFloat(
  Id id, 
  double x, 
  double y)
{
  Index idx;
  GET_INDEX_FROM_ID(_sprites, id, idx);

  int w, h;
  Graphic_QueryWindowSize(&w, &h);
  _sprites.rectF[idx].x += x;
  _sprites.rectF[idx].y += y;
  _sprites.sprite[idx].dest.x = 
    (_sprites.rectF[idx].x - _camera.x) * _camera.zoom - 
    w / 2 * (_camera.zoom - 1);
  _sprites.sprite[idx].dest.y = 
    (_sprites.rectF[idx].y - _camera.y) * _camera.zoom -
    h / 2 * (_camera.zoom - 1);
}

void 
Graphic_CenterCamera()
{
  _updateCameraBounds();

  int w, h;
  Graphic_QueryWindowSize(&w, &h);

  int dx = 0, dy = 0;
  dx = _camera.x + w / 2 - _camera.bounds.x - _camera.bounds.w / 2;
  dy = _camera.y + h / 2 - _camera.bounds.y - _camera.bounds.h / 2;
  _camera.x -= dx;
  _camera.y -= dy;
  Graphic_TranslateAllSprite(dx, dy);
}

Id 
Graphic_CreateSpriteFromSprites(Sprite *start, Sprite *end)
{
  if (start == NULL) {
    return VOID_ID;
  }

  int left = start->dest.x; 
  int top = start->dest.y;
  int right = left + start->dest.w;
  int bottom = top + start->dest.h;
  for (Sprite* curr = start + 1; curr < end; curr++) {
    if (curr->dest.x < left) {
      left = curr->dest.x;
    }

    if (curr->dest.x + curr->dest.w > right) {
      right = curr->dest.x + curr->dest.w;
    }

    if (curr->dest.y < top) {
      top = curr->dest.y;
    }

    if (curr->dest.y + curr->dest.h > bottom) {
      bottom = curr->dest.y + curr->dest.h;
    }
  }

  SDL_Texture* texture = SDL_CreateTexture(
    _renderer, 
    SDL_GetWindowPixelFormat(_window), 
    SDL_TEXTUREACCESS_TARGET,
    right - left, 
    bottom - top
  );

  SDL_Texture* t = SDL_GetRenderTarget(_renderer);
  SDL_SetRenderTarget(_renderer, texture);
  SDL_SetRenderDrawColor(_renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(_renderer);
  for (Sprite* curr = start; curr < end; curr++) {
    Index textureIdx;
    GET_INDEX_FROM_ID(_textures, curr->textureId, textureIdx);
    curr->dest.x -= left;
    curr->dest.y -= top;
    SDL_RenderCopy(_renderer, _textures.textures[textureIdx], &curr->src, &curr->dest);
  }
  SDL_RenderPresent(_renderer);
  SDL_SetRenderTarget(_renderer, t);

  SDL_Rect dest, src;
  dest.x = left;
  dest.y = top;
  src.x = 0;
  src.y = 0;
  src.w = dest.w = right - left;
  src.h = dest.h = bottom - top;

  printf("src.x: %d, src.y: %d, src.w: %d, src.h: %d, dest.x: %d, dest.y: %d, dest.w: %d, dest.h: %d\n", 
      src.x, src.y, src.w, src.h, dest.x, dest.y, dest.w, dest.h
  );
  return _createTilesetSprite(texture, src, dest);
}
