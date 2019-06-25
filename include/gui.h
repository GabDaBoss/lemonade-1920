#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>
#include "utils.h"

typedef enum {
  UnitInPercentFlags_Width = 1 << 1,
  UnitInPercentFlags_Height = 1 << 2,
  UnitInPercentFlags_X = 1 << 3,
  UnitInPercentFlags_Y = 1 << 4,
} UnitInPercentFlags;

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

typedef enum {
  GUI_VerticalAlignTop, GUI_VerticalAlignBottom, GUI_VerticalAlignCenter
} GUI_VerticalAlignment;

typedef enum {
  GUI_HorizontalAlignLeft, GUI_HorizontalAlignRight, GUI_HorizontalAlignCenter
} GUI_HorizontalAlignment;


typedef void (*GUI_OnHandlerFunc)(Id el, GUI_Events e);

Id GUI_CreateWidget(Id parent);
void GUI_AddEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler);
void GUI_RemoveEventListener(Id el, GUI_Events e, GUI_OnHandlerFunc handler);
void GUI_Render();
void GUI_Init();
void GUI_SetWidgetTextAligments(
    Id id, 
    GUI_HorizontalAlignment horizontalAlignment, 
    GUI_VerticalAlignment verticalAlignment
);

void GUI_SetWidgetPosition(
    Id id, 
    double x, 
    double y, 
    double w, 
    double h, 
    UnitInPercentFlags flags
);

void GUI_SetWidgetText(Id id, const char * const text);
 
#endif
