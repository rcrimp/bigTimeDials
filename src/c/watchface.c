#include <pebble.h>
#include "modules/radial.h"
#include "modules/big_digit.h"

static Window *s_main_window;

static GFont s_small_font;

static RadialWidget *s_radial_seconds;
static RadialWidget *s_radial_battery;
static BigDigitWidget *s_digit_hour_tens;
static BigDigitWidget *s_digit_hour_ones;

// widget update handlers
static void seconds_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    static char buffer[16];
    strftime(buffer, sizeof(buffer), "%S", tick_time);

    widget_radial_set(s_radial_seconds, buffer, (float)(tick_time->tm_sec) / SECONDS_PER_MINUTE);
    widget_big_digit_set(s_digit_hour_tens, tick_time->tm_sec / 10); // Display last digit of seconds
    widget_big_digit_set(s_digit_hour_ones, tick_time->tm_sec % 10); // Display tens of seconds
  }
static void battery_handler(BatteryChargeState charge_state) {
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", charge_state.charge_percent);

    widget_radial_set(s_radial_battery, buffer, (float)(charge_state.charge_percent) / 100.0f);
}

// widget creation
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_14));
  
  // radial seconds widget
  s_radial_seconds = widget_radial_create(
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
  s_radial_battery = widget_radial_create(
    GRect(32, 0, 32, 32),
    GColorBlack,
    GColorWhite,
    4, // line thickness
    false, // anti-clockwise
    s_small_font,
    19 // text line_height
  );
  layer_add_child(window_layer, s_radial_battery->layer);

  // big digit test widget
  s_digit_hour_tens = widget_big_digit_create(
    GPoint(0, (bounds.size.h - IMG_HEIGHT) / 2),
    5 // number to display
  );
  layer_add_child(window_layer, s_digit_hour_tens->layer);
  s_digit_hour_ones = widget_big_digit_create(
    GPoint(IMG_WIDTH, (bounds.size.h - IMG_HEIGHT) / 2),
    3 // number to display
  );
  layer_add_child(window_layer, s_digit_hour_ones->layer);

  // placeholder
  seconds_tick_handler(localtime(&(time_t){time(NULL)}), SECOND_UNIT);
  battery_handler(battery_state_service_peek());
  
  // layer_mark_dirty(s_digit_hour_tens->layer);
}

// widget destruction
static void main_window_unload(Window *window) {
  widget_radial_destroy(s_radial_seconds);
  widget_radial_destroy(s_radial_battery);
  widget_big_digit_destroy(s_digit_hour_tens);
  widget_big_digit_destroy(s_digit_hour_ones);

  widget_big_digit_unload_images();
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