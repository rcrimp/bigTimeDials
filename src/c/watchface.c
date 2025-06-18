#include <pebble.h>
#include "modules/radial.h"
#include "modules/big_digit.h"
#include "modules/border.h"

static Window *s_main_window;

static GFont s_small_font;
static GFont s_medium_font;
static GFont s_large_font;

static RadialWidget *s_radial_battery;
static RadialWidget *s_radial_year;
static RadialWidget *s_radial_month;
static RadialWidget *s_radial_day;
static RadialWidget *s_radial_hour;
static RadialWidget *s_radial_minute;

static TextLayer *s_date_layer;
static Layer *s_week_layer;

static int s_month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // Days in each month
const char *ordinals[] = {"1st", "2nd", "3rd", "4th", "5th",
                          "6th", "7th", "8th", "9th", "10th",
                          "11th", "12th", "13th", "14th", "15th", 
                          "16th", "17th", "18th", "19th", "20th", 
                          "21st", "22nd", "23rd", "24th", "25th", 
                          "26th", "27th", "28th", "29th", "30th", 
                          "31st"};
const char *days[] = {"1", "2", "3", "4", "5", "6", "7",
                    "8", "9", "10", "11", "12", "13", "14",
                    "15", "16", "17", "18", "19", "20",
                    "21", "22", "23", "24", "25", "26",
                    "27", "28", "29", "30", "31"};
const char *day_letter[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

static const char *DAY_LETTERS[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

static GFont get_font(bool is_bold) {
  return fonts_get_system_font(is_bold ? FONT_KEY_GOTHIC_14_BOLD : FONT_KEY_GOTHIC_14);
}

static void draw_day_box(GContext *ctx, int x, int width, const char *label, GFont font, int y_offset) {
  GRect box = GRect(x, y_offset, width, 14);
  graphics_draw_text(ctx, label, font, box, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void draw_highlight_box(GContext *ctx, int x, int width, int height) {
  GRect highlight = GRect(x, 0, width, height);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, highlight, 1, GCornersAll);
}

void week_layer_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *today = localtime(&now);
  GRect bounds = layer_get_bounds(layer);
  const int gutter = 1;
  const int cell_width = 19;
  const int highlight_height = 27;

  for (int i = 0; i < 7; i++) {
    // Calculate column position
    int x = (i + 1) * gutter + i * cell_width;

    // Calculate each day's actual date
    time_t day_offset_time = now + (i - 1) * 86400; // -1 to +5 relative to today
    struct tm *day_info = localtime(&day_offset_time);

    // Check if this is today
    bool is_today = (day_info->tm_mday == today->tm_mday &&
                     day_info->tm_mon  == today->tm_mon &&
                     day_info->tm_year == today->tm_year);

    GFont font = get_font(is_today);
    GFont bold = get_font(true);

    // Draw highlight background for today
    if (is_today) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_context_set_text_color(ctx, GColorBlack);
      draw_highlight_box(ctx, x, cell_width, highlight_height);
    } else {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_context_set_text_color(ctx, GColorWhite);
    }

    // Draw weekday label (Su-Mo)
    draw_day_box(ctx, x, cell_width, DAY_LETTERS[i], is_today ? bold : font, -3);

    // Draw today’s date
    char day_text[3];
    snprintf(day_text, sizeof(day_text), "%d", day_info->tm_mday);
    draw_day_box(ctx, x, cell_width, day_text, is_today ? bold : font, 10);

    // Draw 6 days ahead (for visual cue?)
    time_t future_time = now + (i + 6) * 86400;
    struct tm *future_info = localtime(&future_time);
    snprintf(day_text, sizeof(day_text), "%d", future_info->tm_mday);
    draw_day_box(ctx, x, cell_width, day_text, font, 24);
  }
}

// widget update handlers
static void seconds_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  static char s_year[4]; // Year in two digits
  static char s_month[3]; // Month in two digits
  // static char s_day[3]; // Day in two digits
  static char s_hour[3]; // Hour in two digits
  static char s_minute[3]; // Minute in two digits
  static char s_date[12];

  strftime(s_year, sizeof(s_year), "'%y", tick_time);
  strftime(s_month, sizeof(s_month), "%m", tick_time);
  // strftime(s_day, sizeof(s_day), "%d", tick_time);
  char* s_day = (char*)days[tick_time->tm_mday - 1]; // Get ordinal day
  strftime(s_hour, sizeof(s_hour), "%H", tick_time);
  strftime(s_minute, sizeof(s_minute), "%M", tick_time); 

  strftime(s_date, sizeof(s_date), "%B", tick_time);         
  text_layer_set_text(s_date_layer, s_date);

  // days out of 365
  float year_progress = (tick_time->tm_yday + 1) / 365.0f;
  // days in the month (ignore leap years for simplicity)
  float month_progress = (tick_time->tm_mday + 1) / (s_month_days[tick_time->tm_mon] + 1.0f);
  // hours in the day (24 )
  float day_progress = (tick_time->tm_hour + 1) / 24.0f;
  // minutes in he hour (60)
  float hour_progress = (tick_time->tm_min + 1) / 60.0f;
  // seconds in the minute (60)
  float minute_progress = (tick_time->tm_sec + 1) / 60.0f;

  
  widget_radial_set(s_radial_year, s_year, year_progress);
  widget_radial_set(s_radial_month, s_month, month_progress);
  widget_radial_set(s_radial_day, s_day, day_progress);
  widget_radial_set(s_radial_hour, s_hour, hour_progress);
  widget_radial_set(s_radial_minute, s_minute, minute_progress);
  layer_mark_dirty(s_week_layer);
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
  s_large_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_48));

  // radial year
  s_radial_year = widget_radial_create(
    GRect(0, 0, 36, 36),
    GColorBlack, GColorWhite,
    3, true, s_small_font, 18 * 1.3 
  );
  layer_add_child(window_layer, s_radial_year->layer);

  // radial month
  s_radial_month = widget_radial_create(
    GRect(36, 0, 36, 36),
    GColorBlack, GColorWhite,
    3, true, s_small_font, 18 * 1.3
  );
  layer_add_child(window_layer, s_radial_month->layer);

  // radial day
  s_radial_day = widget_radial_create(
    GRect(72, 0, 36, 36),
    GColorBlack, GColorWhite,
    3, true, s_small_font, 18 * 1.3
  );
  layer_add_child(window_layer, s_radial_day->layer);

  // radial hour
  s_radial_hour = widget_radial_create(
    GRect(0, 36, 72, 72),
    GColorBlack, GColorLightGray,
    5, true, s_large_font, 48 * 1.3
  );
  layer_add_child(window_layer, s_radial_hour->layer);

  // radial minute
  s_radial_minute = widget_radial_create(
    GRect(72, 36, 72, 72),
    GColorBlack, GColorLightGray,
    5, true, s_large_font, 48 * 1.3
  );
  layer_add_child(window_layer, s_radial_minute->layer);

  // radial battery layer
  // int x = (bounds.size.w - 32) / 2;
  s_radial_battery = widget_radial_create(
    GRect(108, 0, 36, 36),
    GColorClear,
    GColorWhite,
    3, // line thickness
    false, // anti-clockwise
    s_small_font,
    18 * 1.3 // text line_height
  );
  layer_add_child(window_layer, s_radial_battery->layer);

  // date layer
  s_date_layer = text_layer_create(
    GRect(0, bounds.size.w - 40, bounds.size.w, 28 * 1.3)
  );
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_small_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "Wed, Jun");
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_week_layer = layer_create(
    GRect(0, bounds.size.h - 43, bounds.size.w, 42)
  );
  layer_add_child(window_layer, s_week_layer);
  layer_set_update_proc(s_week_layer, week_layer_proc);

  // initial values
  seconds_tick_handler(localtime(&(time_t){time(NULL)}), SECOND_UNIT);
  battery_handler(battery_state_service_peek());
}

// widget destruction
static void main_window_unload(Window *window) {
  if (s_date_layer) text_layer_destroy(s_date_layer);
  if (s_week_layer) layer_destroy(s_week_layer);

  if (s_radial_battery) widget_radial_destroy(s_radial_battery);
  if (s_radial_year) widget_radial_destroy(s_radial_year);
  if (s_radial_month) widget_radial_destroy(s_radial_month);
  if (s_radial_day) widget_radial_destroy(s_radial_day);
  if (s_radial_hour) widget_radial_destroy(s_radial_hour);
  if (s_radial_minute) widget_radial_destroy(s_radial_minute);
  
  if (s_small_font) fonts_unload_custom_font(s_small_font);
  if (s_medium_font) fonts_unload_custom_font(s_medium_font);
  if (s_large_font) fonts_unload_custom_font(s_large_font);
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