#pragma once
#include <pebble.h>

typedef struct {
    Layer *layer; // Layer to draw the widget
    
    // color theme
    GColor bg_color;
    GColor fg_color;
    
    // radial properties
    int line_thickness;
    bool clockwise;
    float progress;
    
    // text properties
    TextLayer *text_layer;
    int line_height;
    GFont font;
} RadialWidget;

RadialWidget *widget_radial_create(
    GRect bounds,
    GColor bg_color,
    GColor fg_color,
    int line_thickness,
    bool clockwise,
    GFont font,
    int line_height
);
void widget_radial_destroy(RadialWidget *widget);
void widget_radial_update(Layer *layer, GContext *ctx);

void widget_radial_set(RadialWidget *widget, const char *text, float progress);