#include <pebble.h>
#include "border.h"

BorderWidget *widget_border_create(GRect bounds, int thickness) {
  BorderWidget *widget = malloc(sizeof(BorderWidget));
  if (!widget) return NULL;
  
  widget->layer = layer_create_with_data(bounds, sizeof(BorderWidget *));
  if (!widget->layer) {
    free(widget);
    return NULL;
  }

  // Attach user data
  *(BorderWidget **)layer_get_data(widget->layer) = widget;
  layer_set_update_proc(widget->layer, widget_border_update);

  widget->progress = 0.0f;
  widget->thickness = thickness;

  layer_set_update_proc(widget->layer, widget_border_update);
  return widget;
}

void widget_border_update(Layer *layer, GContext *ctx) {
    BorderWidget *widget = *(BorderWidget **)layer_get_data(layer);
    GRect bounds = layer_get_bounds(layer);
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    
  const int W = bounds.size.w;
  const int H = bounds.size.h;
  const int T = widget->thickness;

  const int px_top_half  = W / 2;
  const int px_right     = H - T;
  const int px_bottom    = W - T;
  const int px_left      = H - T;
  const int px_top_left  = (W / 2) - T;

  const int perimeter = px_top_half + px_right + px_bottom + px_left + px_top_left;

  int dur_top_half  = (60 * px_top_half) / perimeter;
  int dur_right     = (60 * px_right)    / perimeter;
  int dur_bottom    = (60 * px_bottom)   / perimeter;
  int dur_left      = (60 * px_left)     / perimeter;
  int dur_top_left  = (60 * px_top_left) / perimeter;

  int dur_total = dur_top_half + dur_right + dur_bottom + dur_left + dur_top_left;
  if (dur_total < 60) dur_top_left += 60 - dur_total; // absorb rounding

  int m = localtime(&(time_t){time(NULL)})->tm_min;
  int elapsed = m;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "BorderWidget: elapsed %d seconds", elapsed);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);

  if (elapsed < dur_top_half) {
    // Top-right (left → right), full width, no vertical inset
    int px = (W / 2) * elapsed / dur_top_half;
    graphics_fill_rect(ctx, GRect(W / 2, 0, px, T), 0, GCornerNone);

  } else if ((elapsed -= dur_top_half) < dur_right) {
    // Top-right done
    graphics_fill_rect(ctx, GRect(W / 2, 0, W / 2, T), 0, GCornerNone);

    // Right edge (top → bottom), inset by T
    int px = (H - T) * elapsed / dur_right;
    graphics_fill_rect(ctx, GRect(W - T, T, T, px), 0, GCornerNone);

  } else if ((elapsed -= dur_right) < dur_bottom) {
    // Top-right + right full
    graphics_fill_rect(ctx, GRect(W / 2, 0, W / 2, T), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(W - T, T, T, H - T), 0, GCornerNone);

    // Bottom (right → left), inset by T from bottom and sides
    int px = (W - T) * elapsed / dur_bottom;
    graphics_fill_rect(ctx, GRect(W - px - T, H - T, px, T), 0, GCornerNone);

  } else if ((elapsed -= dur_bottom) < dur_left) {
    // Top + right + bottom full
    graphics_fill_rect(ctx, GRect(W / 2, 0, W / 2, T), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(W - T, T, T, H - T), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(0, H - T, W - T, T), 0, GCornerNone);

    // Left (bottom → top), inset from all edges
    int px = (H - T) * elapsed / dur_left;
    graphics_fill_rect(ctx, GRect(0, H - T - px, T, px), 0, GCornerNone);

  } else {
    // All previous full
    graphics_fill_rect(ctx, GRect(W / 2, 0, W / 2, T), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(W - T, T, T, H - T), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(0, H - T, W - T, T), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(0, 0, T, H - T), 0, GCornerNone);

    elapsed -= dur_left;
    // Top-left (left → right), inset by T from top and sides
    int px = (W / 2 - T) * elapsed / dur_top_left;
    graphics_fill_rect(ctx, GRect(T, 0, px, T), 0, GCornerNone);
  }
}

void widget_border_set_progress(BorderWidget *widget, float progress) {
  if (widget) {
    widget->progress = progress;
    layer_mark_dirty(widget->layer);
  }
}

void widget_border_destroy(BorderWidget *widget) {
  if (widget) {
    layer_destroy(widget->layer);
    free(widget);
  }
}
