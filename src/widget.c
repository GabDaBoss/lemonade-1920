#include "widget.h"
#include "graphic.h"

#define MAX_Widget_ELEMENTS 1000

#define AUTO -1

typedef struct {
  double w, h, x, y;
  Widget_HorizontalAlignment horizontalAlignment;
  Widget_VerticalAlignment verticalAlignment;
  UnitInPercentFlags unitInPercentFlags;
  Index  parent;
  Uint32 backgroundColor;
  Uint32 bordersColor;
  SDL_Texture* texture;
  SDL_Rect dest;
  SDL_Rect src;
} Element;


static struct {
  Element elements[MAX_Widget_ELEMENTS];
  SET_STRUCT_FOR_DOD(Id, MAX_Widget_ELEMENTS);
} _elements;

Id 
Widget_Create(Id parent)
{
  Index index;
  Id id;
  GET_NEXT_ID(_elements, id, index, MAX_Widget_ELEMENTS);

  if (parent != VOID_ID) {
    Index parentIdx;
    GET_INDEX_FROM_ID(_elements, parent, parentIdx);
    _elements.elements[index].parent = parent;
  } else {
    _elements.elements[index].parent = VOID_INDEX;
  }


  return id;
}

void 
Widget_AddEventListener(Id el, Widget_Events e, Widget_OnHandlerFunc handler)
{

}

void 
Widget_RemoveEventListener(Id el, Widget_Events e, Widget_OnHandlerFunc handler)
{

}

void 
Widget_Render()
{
  for (Index i = 0; i < _elements.total; i++) {
    Element element = _elements.elements[i];
    SDL_Rect dest = {0};
    SDL_Rect parentDest = {0};

    if (element.parent == VOID_INDEX) {
      Graphic_QueryWindowSize(&parentDest.w, &parentDest.h);
    } else {
      parentDest = _elements.elements[element.parent].dest;
    }

    dest.x = element.x;
    if (element.unitInPercentFlags & UnitInPercentFlags_X) {
      dest.x *= parentDest.w / 100;
    }
    dest.x += parentDest.x;

    dest.y = element.y;
    if (element.unitInPercentFlags & UnitInPercentFlags_Y) {
      dest.y *= parentDest.w / 100;
    }
    dest.y += parentDest.y;

    dest.w = element.w;
    if (element.unitInPercentFlags & UnitInPercentFlags_Width) {
      dest.w *= parentDest.w / 100;
    }

    dest.h = element.h;
    if (element.unitInPercentFlags & UnitInPercentFlags_Height) {
      dest.h *= parentDest.h / 100;
    }

    switch (element.horizontalAlignment) {
        case Widget_HorizontalAlignRight:
          dest.x = parentDest.x + parentDest.w - dest.w;
          break;
        case Widget_HorizontalAlignCenter:
          dest.x = (parentDest.x + parentDest.w) / 2 - dest.w / 2;
          break;
        default: break;
    }

    switch (element.verticalAlignment) {
        case Widget_VerticalAlignBottom:
          dest.y = parentDest.y + parentDest.h - dest.h;
          break;
        case Widget_VerticalAlignCenter:
          dest.y = (parentDest.y + parentDest.h) / 2 - dest.h / 2;
          break;
        default: break;
    }

    _elements.elements[i].dest = dest;
  }

  for (Index i = 0; i < _elements.total; i++) {
    Element el = _elements.elements[i];

    Graphic_FillRect(el.dest, el.backgroundColor);

    if (el.texture) {
      Graphic_RenderCopy(el.texture, &el.src, &el.dest);
    }
  }
}

void
Widget_Init()
{
  INIT_STRUCT_FOR_DOD_FREE_LIST(_elements, MAX_Widget_ELEMENTS);
  /*
  _elements.total = 1;
  _elements.elements[Widget_ROOT].backgroundColor = 0xFF00FFFF;
  _elements.elements[Widget_ROOT].x = 0;
  _elements.elements[Widget_ROOT].y = 20;
  _elements.elements[Widget_ROOT].width = 100;
  _elements.elements[Widget_ROOT].unitInPercentFlags |= UnitInPercentFlags_Width;
  _elements.elements[Widget_ROOT].height = 20;
  _elements.elements[Widget_ROOT].parent = VOID_INDEX;
  SDL_Color color = {0xFF, 0xFF, 0xFF, 0xFF};
  _elements.elements[Widget_ROOT].textTexture = Graphic_CreateTextSDLTexture(
      "Lemon2092", color, NULL, NULL);

  
  _elements.elements[_elements.total].backgroundColor = 0x00FFFFFF;
  _elements.elements[_elements.total].x = 20;
  _elements.elements[_elements.total].y = 20;
  _elements.elements[_elements.total].width = 20;
  _elements.elements[_elements.total].height = 20;
  _elements.elements[_elements.total++].parent = Widget_ROOT;
  */
}

void 
Widget_SetAligments(
    Id id, 
    Widget_HorizontalAlignment horizontalAlignment, 
    Widget_VerticalAlignment verticalAlignment) 
{
  Index idx;
  GET_INDEX_FROM_ID(_elements, id, idx);

  _elements.elements[idx].horizontalAlignment = horizontalAlignment;
  _elements.elements[idx].verticalAlignment = verticalAlignment;
}

void 
Widget_SetText(Id id, const char * const text)
{
  Index idx;
  GET_INDEX_FROM_ID(_elements, id, idx);
  SDL_Color color = {0xFF, 0xFF, 0xFF, 0xFF};
  _elements.elements[idx].texture = Graphic_CreateTextSDLTexture(
      text, color, NULL, NULL);
}

void 
Widget_SetImage(Id id, const char * const image)
{
  Index idx;
  GET_INDEX_FROM_ID(_elements, id, idx);
  _elements.elements[idx].texture = Graphic_CreateSDLTexture(image);
  _elements.elements[idx].src.x = 0;
  _elements.elements[idx].src.y = 0;

  Graphic_QuerySDLTextureSize(
      _elements.elements[idx].texture,
      &_elements.elements[idx].src.w,
      &_elements.elements[idx].src.h
  );
}

void 
Widget_SetPosition(
    Id id, 
    double x, 
    double y, 
    double w, 
    double h, 
    UnitInPercentFlags flags
) {
  Index idx;
  GET_INDEX_FROM_ID(_elements, id, idx);

  _elements.elements[idx].x = x;
  _elements.elements[idx].y = y;
  _elements.elements[idx].w = w;
  _elements.elements[idx].h = h;
  _elements.elements[idx].unitInPercentFlags = flags;
}

void 
Widget_SetSrc(Id id, SDL_Rect src)
{
  Index idx;
  GET_INDEX_FROM_ID(_elements, id, idx);
  _elements.elements[idx].src = src;
}
