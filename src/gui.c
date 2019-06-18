#include "gui.h"
#include "graphic.h"

#define MAX_GUI_ELEMENTS 1000

typedef struct {
  double width, height, x, y;
} Position;

static struct {
  Index  parents[MAX_GUI_ELEMENTS];
  Position positions[MAX_GUI_ELEMENTS];
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

    for (Index j = i; j != VOID_INDEX; j = _elements.parents[j]){
      dest.x += _elements.positions[j].x;
      dest.y += _elements.positions[j].y;
    }


    dest.w = _elements.positions[i].width;
    dest.h = _elements.positions[i].height;

    Graphic_FillRect(dest, _elements.backgroundColors[i]);
  }
}
void
GUI_Init()
{
  INIT_STRUCT_FOR_DOD_FREE_LIST(_elements, MAX_GUI_ELEMENTS);
  _elements.total = 1;
  _elements.backgroundColors[GUI_ROOT] = 0xFF00FFFF;
  _elements.positions[GUI_ROOT].x = 20;
  _elements.positions[GUI_ROOT].y = 20;
  _elements.positions[GUI_ROOT].width = 20;
  _elements.positions[GUI_ROOT].height = 20;
  _elements.parents[GUI_ROOT] = VOID_INDEX;

  
  _elements.backgroundColors[_elements.total] = 0x00FFFFFF;
  _elements.positions[_elements.total].x = 20;
  _elements.positions[_elements.total].y = 20;
  _elements.positions[_elements.total].width = 20;
  _elements.positions[_elements.total].height = 20;
  _elements.parents[_elements.total++] = GUI_ROOT;
}
