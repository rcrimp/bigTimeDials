#include <pebble.h>
#include "modules/widget.h"

static Window *s_main_window;

static GFont s_small_font;

static Widget *s_radial_seconds;
static Widget *s_radial_battery;

// widget update handlers
static void seconds_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    static char buffer[16];
    strftime(buffer, sizeof(buffer), "%S", tick_time);

    widget_set_data(s_radial_seconds, buffer, (float)(tick_time->tm_sec) / SECONDS_PER_MINUTE);
}
static void battery_handler(BatteryChargeState charge_state) {
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", charge_state.charge_percent);

    widget_set_data(s_radial_battery, buffer, (float)(charge_state.charge_percent) / 100.0f);
}

// widget creation
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_14));
  
  // radial seconds widget
  s_radial_seconds = widget_create(
    GRect(0, 0, 32, 32),
    GColorBlack,
    GColorWhite,
    4, // line thickness
    true, // clockwise
    s_small_font,
    19 // text line_height
  );
  layer_add_child(window_layer, s_radial_seconds->layer);

  // radial battery layer
  s_radial_battery = widget_create(
    GRect(32, 0, 32, 32),
    GColorBlack,
    GColorWhite,
    4, // line thickness
    false, // anti-clockwise
    s_small_font,
    19 // text line_height
  );
  layer_add_child(window_layer, s_radial_battery->layer);

  // placeholder
  seconds_tick_handler(localtime(&(time_t){time(NULL)}), SECOND_UNIT);
  battery_handler(battery_state_service_peek());

}

// widget destruction
static void main_window_unload(Window *window) {
  widget_destroy(s_radial_seconds);
  widget_destroy(s_radial_battery);
  fonts_unload_custom_font(s_small_font);
}
  
static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(SECOND_UNIT, seconds_tick_handler);
  battery_state_service_subscribe(battery_handler);
}
 
static void deinit() {
  window_destroy(s_main_window);
}
 
int main(void) {
  init();
  app_event_loop();
  deinit();
}