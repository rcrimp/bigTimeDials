#include <pebble.h>
#include "modules/radial.h"
#include "modules/big_digit.h"
#include "modules/border.h"

static Window *s_main_window;

static GFont s_tiny_font;
static GFont s_tiny_font_bold;
static GFont s_small_font;
static GFont s_medium_font;
static GFont s_large_font;

static RadialWidget *s_radial_battery;
static RadialWidget *s_radial_minute;

static BigDigitWidget *s_big_digit_hour_tens;
static BigDigitWidget *s_big_digit_hour_ones;
static TextLayer *s_minute_layer;

static TextLayer *s_date_layer;
static Layer *s_calendar_layer;

static const char *DAY_LETTERS[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
static int s_month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // Days in each month

static void draw_day_box(GContext *ctx, int x, int width, const char *label, GFont font, int y_offset)
{
  GRect box = GRect(x, y_offset, width, 14);
  graphics_draw_text(ctx, label, font, box, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void draw_highlight_box(GContext *ctx, int x, int width, int height)
{
  GRect highlight = GRect(x, 0, width, height);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, highlight, 1, GCornersAll);
}

void week_layer_proc(Layer *layer, GContext *ctx)
{
  time_t now = time(NULL);
  struct tm *today = localtime(&now);

  GRect bounds = layer_get_bounds(layer);
  const int gutter = 1;
  const int cell_width = 19;
  const int highlight_height = 26;

  // month date
  int today_month_day = today->tm_mday;                 // Get the day of the month (1-31)                // Get the month (0-11, so add 1 for 1-12)
  int today_month_length = s_month_days[today->tm_mon]; // Get the number of days in the month

  int today_weekday = today_month_day % 7;
  int last_monday_weekday = today_month_day - (today_month_day + 6) % 7;

  // Draw line to divide weekdays from weekend (Sat Sun)
  int x = (cell_width + gutter) * 5;
  GPoint start = GPoint(x, 0);
  GPoint end = GPoint(x, bounds.size.h);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, start, end);

  for (int i = 0; i < 7; i++)
  {
    // Calculate column position
    int x = (i + 1) * gutter + i * cell_width;

    int day_of_the_month = (last_monday_weekday + i + 1) % today_month_length;
    int next_day_of_the_month = (last_monday_weekday + i + 7) % today_month_length + 1;

    bool is_today = today_month_day == day_of_the_month;

    // Draw highlight background for today
    if (is_today)
    {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_context_set_text_color(ctx, GColorBlack);
      draw_highlight_box(ctx, x, cell_width, highlight_height);
    }
    else
    {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_context_set_text_color(ctx, GColorWhite);
    }

    // Draw weekday label (Su-Mo)
    draw_day_box(ctx, x, cell_width, DAY_LETTERS[(i + 1) % 7], is_today ? s_tiny_font_bold : s_tiny_font, -3);

    // Draw today’s date
    char day_text[3];
    snprintf(day_text, sizeof(day_text), "%d", day_of_the_month);
    draw_day_box(ctx, x, cell_width, day_text, is_today ? s_tiny_font_bold : s_tiny_font, 10);

    // Draw 6 days ahead (for visual cue?)
    // time_t future_time = now + (i + 6) * 86400;
    // struct tm *future_info = localtime(&future_time);
    snprintf(day_text, sizeof(day_text), "%d", next_day_of_the_month);
    graphics_context_set_text_color(ctx, GColorWhite);

    draw_day_box(ctx, x, cell_width, day_text, s_tiny_font, 24);
  }
}

// widget update handlers
static void seconds_tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  static int prev_minute = -1;
  static int prev_hour = -1;
  static int prev_day = -1;

  static char s_hour[3];   // 21
  static char s_minute[3]; // 34
  static char s_day[3];    // 01
  static char s_date[16];  // "Jun  23-06-01"

  if (prev_minute != tick_time->tm_min)
  {
    strftime(s_minute, sizeof(s_minute), "%M", tick_time);
    text_layer_set_text(s_minute_layer, s_minute);

    float hour_progress = (tick_time->tm_min + 1) / 60.0f;
    widget_radial_set(s_radial_minute, s_hour, hour_progress);

    prev_minute = tick_time->tm_min;
  }
  if (prev_hour != tick_time->tm_hour)
  {
    strftime(s_hour, sizeof(s_hour), "%H", tick_time);
    prev_hour = tick_time->tm_hour;
    widget_big_digit_set(s_big_digit_hour_tens, tick_time->tm_hour / 10);
    widget_big_digit_set(s_big_digit_hour_ones, tick_time->tm_hour % 10);
  }
  if (prev_day != tick_time->tm_mday)
  {
    strftime(s_day, sizeof(s_day), "%d", tick_time);
    strftime(s_date, sizeof(s_date), "%b  %y-%m-%d", tick_time);
    text_layer_set_text(s_date_layer, s_date);

    layer_mark_dirty(s_calendar_layer);
    prev_day = tick_time->tm_mday;
  }
}
static void battery_handler(BatteryChargeState charge_state)
{
  static char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d", charge_state.charge_percent);
  widget_radial_set(s_radial_battery, buffer, (float)(charge_state.charge_percent) / 100.0f);
}

// widget creation
static void main_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_18));
  s_medium_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_24));
  s_large_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_48));

  s_tiny_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_tiny_font_bold = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

  s_big_digit_hour_tens = widget_big_digit_create(GPoint(0, 0), 0);
  layer_add_child(window_layer, s_big_digit_hour_tens->layer);
  s_big_digit_hour_ones = widget_big_digit_create(GPoint(bounds.size.w - IMG_WIDTH, 0), 0);
  layer_add_child(window_layer, s_big_digit_hour_ones->layer);

  // // radial minute
  s_radial_minute = widget_radial_create(
      GRect(0, 69, 36, 36),
      GColorClear, GColorWhite,
      3, true, s_small_font, 18 * 1.3);
  layer_add_child(window_layer, s_radial_minute->layer);

  // radial battery layer
  // int x = (bounds.size.w - 32) / 2;
  s_radial_battery = widget_radial_create(
      GRect(bounds.size.w - 36, 69, 36, 36),
      GColorClear,
      GColorLightGray,
      3,     // line thickness
      false, // anti-clockwise
      s_small_font,
      18 * 1.3 // text line_height
  );
  layer_add_child(window_layer, s_radial_battery->layer);

  // date layer
  s_date_layer = text_layer_create(
      GRect(0, bounds.size.w - 40, bounds.size.w, 28 * 1.3));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_small_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "June");
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // minute layer
  s_minute_layer = text_layer_create(
      GRect((bounds.size.w - IMG_WIDTH) / 2, IMG_HEIGHT - 12, IMG_WIDTH, 48));
  text_layer_set_background_color(s_minute_layer, GColorClear);
  text_layer_set_text_color(s_minute_layer, GColorWhite);
  text_layer_set_font(s_minute_layer, s_large_font);
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);
  text_layer_set_text(s_minute_layer, "00");
  layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));

  s_calendar_layer = layer_create(
      GRect(0, bounds.size.h - 43, bounds.size.w, 42));
  layer_add_child(window_layer, s_calendar_layer);
  layer_set_update_proc(s_calendar_layer, week_layer_proc);

  // initial values
  seconds_tick_handler(localtime(&(time_t){time(NULL)}), SECOND_UNIT);
  battery_handler(battery_state_service_peek());
}

// widget destruction
static void main_window_unload(Window *window)
{
  if (s_minute_layer)
    text_layer_destroy(s_minute_layer);
  if (s_date_layer)
    text_layer_destroy(s_date_layer);
  if (s_calendar_layer)
    layer_destroy(s_calendar_layer);

  if (s_radial_battery)
    widget_radial_destroy(s_radial_battery);
  if (s_radial_minute)
    widget_radial_destroy(s_radial_minute);

  if (s_small_font)
    fonts_unload_custom_font(s_small_font);
  if (s_medium_font)
    fonts_unload_custom_font(s_medium_font);
  if (s_large_font)
    fonts_unload_custom_font(s_large_font);

  if (s_big_digit_hour_tens)
    widget_big_digit_destroy(s_big_digit_hour_tens);
  if (s_big_digit_hour_ones)
    widget_big_digit_destroy(s_big_digit_hour_ones);
}

void hour_tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  if (!quiet_time_is_active())
  {
    vibes_double_pulse();
  }
}

static void init()
{
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers){
                                                .load = main_window_load,
                                                .unload = main_window_unload});
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, seconds_tick_handler);
  battery_state_service_subscribe(battery_handler);
}

static void deinit()
{
  window_destroy(s_main_window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}