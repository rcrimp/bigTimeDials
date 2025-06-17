#include <pebble.h>
#include "modules/radial.h"
#include "modules/big_digit.h"
#include "modules/border.h"

static Window *s_main_window;

static GFont s_small_font;
static GFont s_medium_font;
static GFont s_large_fount;

static BorderWidget *s_border_widget;
static RadialWidget *s_radial_seconds;
static RadialWidget *s_radial_battery;
static BigDigitWidget *s_digit_hour_tens;
static BigDigitWidget *s_digit_hour_ones;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;
static TextLayer *s_hour_layer;

// widget update handlers
static void seconds_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    // Update border seconds widget
    if (s_border_widget) {
        widget_border_set_progress(s_border_widget, (float)(tick_time->tm_min) / MINUTES_PER_HOUR);
    }
  
    static char s_buffer[16];
    
    if (s_radial_seconds) {
        strftime(s_buffer, sizeof(s_buffer), "%M", tick_time);
        widget_radial_set(s_radial_seconds, s_buffer, (float)(tick_time->tm_min) / MINUTES_PER_HOUR);
    }
    if (s_digit_hour_tens) {
        widget_big_digit_set(s_digit_hour_tens, tick_time->tm_min / 10); // Display tens of seconds
    }
    if (s_digit_hour_ones) {
        widget_big_digit_set(s_digit_hour_ones, tick_time->tm_min % 10); // Display last digit of seconds
    }
    
    // Update date layer
    static char buffer[16];
    if (s_date_layer) {
      strftime(buffer, sizeof(buffer), "%y %m %d", tick_time);
      text_layer_set_text(s_date_layer, buffer);
    }

    // Update day layer
    static char day_buffer[16];
    if (s_day_layer) {
      strftime(day_buffer, sizeof(day_buffer), "%A", tick_time);
      text_layer_set_text(s_day_layer, day_buffer);
    }

    // Update hour layer
    static char hour_buffer[16];
    if (s_hour_layer) {
      strftime(hour_buffer, sizeof(hour_buffer), "%H", tick_time);
      text_layer_set_text(s_hour_layer, hour_buffer);
    }
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

  s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_18));
  s_medium_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_24));
  s_large_fount = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_48));

  s_border_widget = widget_border_create(
    GRect(0, 0, bounds.size.w, bounds.size.h),
    1
  );
  layer_add_child(window_layer, s_border_widget->layer);

  int date_height = 24; // Height of the date layer

  s_day_layer = text_layer_create(
    GRect(0, (bounds.size.h + IMG_HEIGHT) / 2 - 5, bounds.size.w, date_height + 5)
  );
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_font(s_day_layer, s_medium_font);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  text_layer_set_text(s_day_layer, "Wednesday");
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));

  s_date_layer = text_layer_create(
    GRect(0, (bounds.size.h + IMG_HEIGHT) / 2 + date_height - 5, bounds.size.w, date_height)
  );
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_medium_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "44 44 44");
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_hour_layer = text_layer_create(
    GRect(2, -10, bounds.size.w / 2, 48)
  );
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorWhite);
  text_layer_set_font(s_hour_layer, s_large_fount);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_text(s_hour_layer, "00");
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));

  // radial seconds widget
  s_radial_seconds = widget_radial_create(
    GRect(bounds.size.w/2, 6, 32, 32),
    GColorBlack,
    GColorWhite,
    3, // line thickness
    true, // clockwise
    s_small_font,
    18 * 1.3 // text line_height
  );
  layer_add_child(window_layer, s_radial_seconds->layer);
  
  // radial battery layer
  s_radial_battery = widget_radial_create(
    GRect(bounds.size.w/2 + 37, 6, 32, 32),
    GColorBlack,
    GColorWhite,
    3, // line thickness
    false, // anti-clockwise
    s_small_font,
    18 * 1.3 // text line_height
  );
  layer_add_child(window_layer, s_radial_battery->layer);

  // big digit test widget
  s_digit_hour_tens = widget_big_digit_create(
    GPoint(bounds.size.w / 2 - IMG_WIDTH, (bounds.size.h - IMG_HEIGHT) / 2 - 5),
    5 // number to display
  );
  layer_add_child(window_layer, s_digit_hour_tens->layer);
  s_digit_hour_ones = widget_big_digit_create(
    GPoint(bounds.size.w / 2, (bounds.size.h - IMG_HEIGHT) / 2 - 5),
    3 // number to display
  );
  layer_add_child(window_layer, s_digit_hour_ones->layer);

  // placeholder
  seconds_tick_handler(localtime(&(time_t){time(NULL)}), SECOND_UNIT);
  battery_handler(battery_state_service_peek());
  
  layer_mark_dirty(s_digit_hour_tens->layer);
}

// widget destruction
static void main_window_unload(Window *window) {
  if (s_border_widget) widget_border_destroy(s_border_widget);
  if (s_date_layer) text_layer_destroy(s_date_layer);
  if (s_radial_seconds) widget_radial_destroy(s_radial_seconds);
  if (s_radial_battery) widget_radial_destroy(s_radial_battery);
  if (s_digit_hour_tens) widget_big_digit_destroy(s_digit_hour_tens);
  if (s_digit_hour_ones) widget_big_digit_destroy(s_digit_hour_ones);
  if (s_small_font) fonts_unload_custom_font(s_small_font);
  if (s_medium_font) fonts_unload_custom_font(s_medium_font);
  if (s_large_fount) fonts_unload_custom_font(s_large_fount);

  widget_big_digit_unload_images();
}

void hour_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (!quiet_time_is_active()) {
    vibes_double_pulse();
  }
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, seconds_tick_handler);
  // tick_timer_service_subscribe(HOUR_UNIT, hour_tick_handler);
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