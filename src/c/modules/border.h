#pragma once
#include <pebble.h>

typedef struct {
    Layer *layer;
    float progress;
    int thickness;
} BorderWidget;

BorderWidget *widget_border_create(GRect bounds, int thickness);
void widget_border_destroy(BorderWidget *widget);
void widget_border_update(Layer *layer, GContext *ctx);
void widget_border_set_progress(BorderWidget *widget, float progress);