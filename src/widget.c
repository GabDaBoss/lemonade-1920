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
    printf("render gui: %d\n", i);
    SDL_Rect dest = {0};
    Element el = _elements.elements[i];

    for (Index j = i; j != VOID_INDEX; j = _elements.elements[j].parent){
      dest.x += _elements.elements[j].x;
      dest.y += _elements.elements[j].y;
    }

    if (el.unitInPercentFlags & UnitInPercentFlags_Width) {
      if (el.parent == VOID_INDEX) {
        Graphic_QueryWindowSize(&dest.w, NULL);
        dest.w *= el.w / 100;
      } else {
        dest.w = _elements.elements[el.parent].w * el.w / 100;
      }
    } else {
      dest.w = el.w;
    }

    dest.h = el.h;

    Graphic_FillRect(dest, el.backgroundColor);

    if (el.texture) {
      SDL_Rect textDest = dest;
      SDL_QueryTexture(el.texture, NULL, NULL, &textDest.w, &textDest.h);

      switch (el.horizontalAlignment) {
        case Widget_HorizontalAlignRight:
          textDest.x = dest.x + dest.w - textDest.w;
          break;
        case Widget_HorizontalAlignCenter:
          textDest.x = (dest.x + dest.w) / 2 - textDest.w / 2;
          break;
        default: break;
      }

      switch (el.verticalAlignment) {
        case Widget_VerticalAlignBottom:
          textDest.y = dest.y + dest.h - textDest.h;
          break;
        case Widget_VerticalAlignCenter:
          textDest.y = (dest.y + dest.h) / 2 - textDest.h / 2;
          break;
        default: break;
      }
      Graphic_RenderCopy(el.texture, NULL, &textDest);
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
