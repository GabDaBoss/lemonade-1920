#include "gui.h"
#include "graphic.h"

#define MAX_GUI_ELEMENTS 1000

typedef struct {
  double value;
  GUI_Unit unit;
} Size;

typedef struct {
  Size left, right, top, bottom;
} Square;

static struct {
  Square margins[MAX_GUI_ELEMENTS], 
         paddings[MAX_GUI_ELEMENTS], 
         borders[MAX_GUI_ELEMENTS];
  Index  parents[MAX_GUI_ELEMENTS];
  Size widths[MAX_GUI_ELEMENTS], heights[MAX_GUI_ELEMENTS];
  Uint32 backgroundColors[MAX_GUI_ELEMENTS];
  Uint32 bordersColors[MAX_GUI_ELEMENTS];
  SET_STRUCT_FOR_DOD(Id, MAX_GUI_ELEMENTS);
} _elements;

Id 
GUI_AddElement(Id parent)
{
  Index index;
  Id id;
  GET_NEXT_ID(_elements, id, index, MAX_GUI_ELEMENTS);

  _elements.parents[index] = parent;

  return id;
}

static double 
_toPixels(Size size, double outerSizeInPixels)
{
  if (size.unit == GUI_Pixel) {
    return size.value;
  } else if (size.unit == GUI_Percent) {
    return outerSizeInPixels * size.unit / 100;
  }

  return 0;
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
//  Square margin = _elements.margins[GUI_ROOT];
//
//  if (_elements.widths[GUI_ROOT].unit == GUI_Auto) {
//    Graphic_QueryWindowSize(&dest.w, NULL);
//    dest.w -=  _toPixels(margin.left, dest.w) + _toPixels(margin.right, dest.w);
//  } else {
//    dest.w = _elements.widths[GUI_ROOT].value;
//  }
//  if (_elements.widths[GUI_ROOT].unit == GUI_Auto || 
//      _elements.widths[GUI_ROOT].unit == GUI_Percent) {
//    Graphic_QueryWindowSize(NULL, &dest.h);
//    dest.h -=  _toPixels(margin.top, dest.h) + _toPixels(margin.right, dest.h);
//  }


  for (Index i = 0; i < _elements.total; i++) {
    printf("render gui: %d\n", i);
    SDL_Rect dest = {0}, parentDest = {0};

    if (_elements.parents[i] == VOID_INDEX) {
      Graphic_QueryWindowSize(&parentDest.w, &parentDest.h);
    }

    dest.w = _elements.widths[i].value;
    dest.h = _elements.heights[i].value;

    Graphic_FillRect(dest, _elements.backgroundColors[i]);
  }
}
void
GUI_Init()
{
  INIT_STRUCT_FOR_DOD_FREE_LIST(_elements, MAX_GUI_ELEMENTS);
  _elements.total = 1;
  _elements.backgroundColors[GUI_ROOT] = 0xFF00FFFF;
  _elements.widths[GUI_ROOT].unit = GUI_Pixel;
  _elements.widths[GUI_ROOT].value = 20;
  _elements.heights[GUI_ROOT].unit = GUI_Pixel;
  _elements.heights[GUI_ROOT].value = 20;
  _elements.parents[GUI_ROOT] = VOID_INDEX;
}
