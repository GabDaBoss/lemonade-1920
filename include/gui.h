#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>
#include "utils.h"

typedef struct {
  SDL_Rect src, dest;
  SDL_Texture* texture;
  int parent;
} GUI_Element;

typedef enum {
  GUI_OnClick,
  GUI_OnFocus,
  GUI_All
} GUI_Events;

#define GUI_ROOT (Id) 0

typedef void (*GUI_OnHandlerFunc)(Id el, GUI_Events e);

Id GUI_AddElement(Id parent);
void GUI_AddEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler);
void GUI_RemoveEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler);
 
#endif
