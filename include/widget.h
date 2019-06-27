#ifndef WIDGET_H
#define WIDGET_H

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
} Widget_Element;

typedef enum {
  Widget_OnClick,
  Widget_OnFocus,
  Widget_All
} Widget_Events;

typedef enum {
  Widget_Auto,
  Widget_Pixel,
  Widget_Percent,
} Widget_Unit;

typedef enum {
  Widget_VerticalAlignTop, Widget_VerticalAlignBottom, Widget_VerticalAlignCenter
} Widget_VerticalAlignment;

typedef enum {
  Widget_HorizontalAlignLeft, Widget_HorizontalAlignRight, Widget_HorizontalAlignCenter
} Widget_HorizontalAlignment;


typedef void (*Widget_OnHandlerFunc)(Id el, Widget_Events e);

Id Widget_Create(Id parent);
void Widget_AddEventListener(Id el, Widget_Events e, Widget_OnHandlerFunc handler);
void Widget_RemoveEventListener(Id el, Widget_Events e, Widget_OnHandlerFunc handler);
void Widget_Render();
void Widget_Init();
void Widget_SetAligments(
    Id id, 
    Widget_HorizontalAlignment horizontalAlignment, 
    Widget_VerticalAlignment verticalAlignment
);

void Widget_SetPosition(
    Id id, 
    double x, 
    double y, 
    double w, 
    double h, 
    UnitInPercentFlags flags
);

void Widget_SetText(Id id, const char * const text);
void Widget_SetImage(Id id, const char * const image);
void Widget_SetSrc(Id id, SDL_Rect src);
 
#endif
