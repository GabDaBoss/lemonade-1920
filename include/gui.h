#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>
#include "utils.h"

#define GUI_ROOT 0

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

typedef enum {
  GUI_Auto,
  GUI_Pixel,
  GUI_Percent,
} GUI_Unit;


typedef void (*GUI_OnHandlerFunc)(Id el, GUI_Events e);

Id GUI_AddElement(Id parent);
void GUI_AddEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler);
void GUI_RemoveEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler);
void GUI_Render();
void GUI_Init();
 
#endif
