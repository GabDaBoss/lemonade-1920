#include "gui.h"
#include "graphic.h"

#define MAX_GUI_ELEMENTS 1000

#define AUTO -1

typedef enum {
  FixedValueFlags_Width = 1 << 1,
  FixedValueFlags_Height = 1 << 2,
  FixedValueFlags_X = 1 << 3,
  FixedValueFlags_Y = 1 << 4
} FixedValueFlags;

typedef enum {
  UnitInPercentFlags_Width = 1 << 1,
  UnitInPercentFlags_Height = 1 << 2,
  UnitInPercentFlags_X = 1 << 3,
  UnitInPercentFlags_Y = 1 << 4,
} UnitInPercentFlags;

typedef enum {
  AlignLeft, AlignRight, AlignCenter
} Alignment;

typedef struct {
  double width, height, x, y;
  Alignment horizontalAlignment, verticalAlignment;
  FixedValueFlags fixedValues;
  UnitInPercentFlags unitInPercentFlags;
  Index  parent;
  Uint32 backgroundColor;
  Uint32 bordersColor;
  SDL_Texture* textTexture;
} Element;


static struct {
  Element elements[MAX_GUI_ELEMENTS];
  SET_STRUCT_FOR_DOD(Id, MAX_GUI_ELEMENTS);
} _elements;

static void 
_setElementText(Element* el, const char * const text)
{
  SDL_Color color;
  color.r = 0;
  color.g = 0;
  color.b = 0;
  color.a = 0xFF;
  el->textTexture = Graphic_CreateTextSDLTexture(text, color, NULL, NULL);
}

Id 
GUI_AddElement(Id parent)
{
  Index index;
  Id id;
  GET_NEXT_ID(_elements, id, index, MAX_GUI_ELEMENTS);

  _elements.elements[index].parent = parent;

  return id;
}

void 
GUI_AddEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler)
{

}

void 
GUI_RemoveEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler)
{

}

void 
GUI_Render()
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
        dest.w *= el.width / 100;
      } else {
        dest.w = _elements.elements[el.parent].width * el.width / 100;
      }
    } else {
      dest.w = el.width;
    }

    dest.h = el.height;

    Graphic_FillRect(dest, el.backgroundColor);

    if (el.textTexture) {
      SDL_Rect textDest = dest;
      SDL_QueryTexture(el.textTexture, NULL, NULL, &textDest.w, &textDest.h);
      Graphic_RenderCopy(el.textTexture, NULL, &textDest);
    }
  }
}

void
GUI_Init()
{
  INIT_STRUCT_FOR_DOD_FREE_LIST(_elements, MAX_GUI_ELEMENTS);
  _elements.total = 1;
  _elements.elements[GUI_ROOT].backgroundColor = 0xFF00FFFF;
  _elements.elements[GUI_ROOT].x = 0;
  _elements.elements[GUI_ROOT].y = 20;
  _elements.elements[GUI_ROOT].width = 100;
  _elements.elements[GUI_ROOT].unitInPercentFlags |= UnitInPercentFlags_Width;
  _elements.elements[GUI_ROOT].height = 20;
  _elements.elements[GUI_ROOT].parent = VOID_INDEX;
  SDL_Color color = {0xFF};
  _elements.elements[GUI_ROOT].textTexture = Graphic_CreateTextSDLTexture(
      "Lemon2092", color, NULL, NULL);

  
  _elements.elements[_elements.total].backgroundColor = 0x00FFFFFF;
  _elements.elements[_elements.total].x = 20;
  _elements.elements[_elements.total].y = 20;
  _elements.elements[_elements.total].width = 20;
  _elements.elements[_elements.total].height = 20;
  _elements.elements[_elements.total++].parent = GUI_ROOT;
}
