#include "gui.h"

#define MAX_GUI_ELEMENTS 1000

typedef enum {
  Pixel,
  Percent,
} Unit;

typedef struct {
  double value;
  Unit unit;
} Size;

static struct {
  struct {
    Size left, right, top, bottom;
  } margins[MAX_GUI_ELEMENTS], 
    paddings[MAX_GUI_ELEMENTS], 
    borders[MAX_GUI_ELEMENTS];
  Index parents[MAX_GUI_ELEMENTS];
  Uint32 backgroundColor;
  Uint32 bordersColor;
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

void 
GUI_AddEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler)
{

}

void 
GUI_RemoveEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler)
{

}
