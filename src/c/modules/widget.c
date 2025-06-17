#include <pebble.h>
#include <stdlib.h>
#include <string.h>
#include "widget.h"

void widget_update(Layer *layer, GContext *ctx) {
    Widget *widget = *(Widget **)layer_get_data(layer);
    GRect bounds = layer_get_bounds(layer);

    graphics_context_set_fill_color(ctx, widget->bg_color);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    graphics_context_set_fill_color(ctx, widget->fg_color);
    
    if (widget->clockwise) {
        int start_angle = DEG_TO_TRIGANGLE(0);
        int end_angle = DEG_TO_TRIGANGLE(360 * widget->progress);
        graphics_fill_radial(
            ctx, bounds, GOvalScaleModeFitCircle,
            widget->line_thickness, start_angle, end_angle);
    } else {
        int start_angle = DEG_TO_TRIGANGLE(360 * (1.0 - widget->progress));
        int end_angle = DEG_TO_TRIGANGLE(359);
        graphics_fill_radial(
            ctx, bounds, GOvalScaleModeFitCircle,
            widget->line_thickness, start_angle, end_angle);
    }
}

Widget *widget_create(
    GRect bounds,
    GColor bg_color,
    GColor fg_color,
    int line_thickness,
    bool clockwise,
    GFont font,
    int line_height
) {
    Widget *widget = malloc(sizeof(Widget));
    if (!widget) return NULL;

    widget->layer = layer_create_with_data(bounds, sizeof(Widget *));
    if (!widget->layer) {
        free(widget);
        return NULL;
    }

    // Attach user data
    *(Widget **)layer_get_data(widget->layer) = widget;
    layer_set_update_proc(widget->layer, widget_update);

    widget->line_thickness = line_thickness;
    widget->bg_color = bg_color;
    widget->fg_color = fg_color; 
    widget->font = font;
    widget->line_height = line_height;
    widget->progress = 0.0f; // Default progress
    widget->clockwise = clockwise;

    // Create and add text layer
    int text_top = (bounds.size.h - line_height) / 2;
    widget->text_layer = text_layer_create(GRect(0, text_top, bounds.size.w, bounds.size.h));
    text_layer_set_background_color(widget->text_layer, GColorClear);
    text_layer_set_text_color(widget->text_layer, fg_color);
    text_layer_set_font(widget->text_layer, widget->font);
    text_layer_set_text_alignment(widget->text_layer, GTextAlignmentCenter);
    
    layer_add_child(widget->layer, text_layer_get_layer(widget->text_layer));

    return widget;
}

void widget_set_data(Widget *widget, const char *text, float progress) {
    if (widget && widget->text_layer) {
        text_layer_set_text(widget->text_layer, text);
        widget->progress = progress;
        layer_mark_dirty(widget->layer);
    }
}

void widget_destroy(Widget *widget) {
    if (!widget) return;
    if (widget->text_layer) {
        text_layer_destroy(widget->text_layer);
    }
    if (widget->layer) {
        layer_destroy(widget->layer);
    }
    free(widget);
}
