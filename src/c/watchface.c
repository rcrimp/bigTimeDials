#include <pebble.h>
  
#define DIGIT_COUNT 4
#define IMAGE_COUNT 10 
#define IMG_WIDTH 69
#define IMG_HEIGHT 69
#define DIGIT_PADDING 2

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168

static Window *s_main_window;

static TextLayer *s_date_layer;
static TextLayer *s_battery_layer; 
static GFont s_date_font;
static GFont s_battery_font;

static BitmapLayer *s_time_digits[DIGIT_COUNT];
static GBitmap *s_image_numbers[IMAGE_COUNT];

static int s_prev_digits[DIGIT_COUNT] = {-1, -1, -1, -1};
static char s_prev_date[11] = "";
static int s_prev_battery_percent = -1;

static void update_battery() {
  BatteryChargeState charge_state = battery_state_service_peek();
  if (charge_state.charge_percent == s_prev_battery_percent) return;

  s_prev_battery_percent = charge_state.charge_percent;
  static char s_battery_buffer[8];
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
  layer_mark_dirty(text_layer_get_layer(s_battery_layer));
}


static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Update date string only if changed
  static char s_buffer[11];
  strftime(s_buffer, sizeof(s_buffer), "%a %d %b", tick_time);
  if (strncmp(s_prev_date, s_buffer, sizeof(s_buffer)) != 0) {
    strcpy(s_prev_date, s_buffer);
    text_layer_set_text(s_date_layer, s_buffer);
  }

  // Extract digits
  int digits[DIGIT_COUNT] = {
    tick_time->tm_hour / 10,
    tick_time->tm_hour % 10,
    tick_time->tm_min / 10,
    tick_time->tm_min % 10
  };

  // Set only changed digits
  for (int i = 0; i < DIGIT_COUNT; ++i) {
    if (s_prev_digits[i] != digits[i]) {
      s_prev_digits[i] = digits[i];
      bitmap_layer_set_bitmap(s_time_digits[i], s_image_numbers[digits[i]]);
      layer_mark_dirty(bitmap_layer_get_layer(s_time_digits[i]));
    }
  }
}

 
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
  s_image_numbers[0] = gbitmap_create_with_resource(RESOURCE_ID_ZERO);
  s_image_numbers[1] = gbitmap_create_with_resource(RESOURCE_ID_ONE);
  s_image_numbers[2] = gbitmap_create_with_resource(RESOURCE_ID_TWO);
  s_image_numbers[3] = gbitmap_create_with_resource(RESOURCE_ID_THREE);
  s_image_numbers[4] = gbitmap_create_with_resource(RESOURCE_ID_FOUR);
  s_image_numbers[5] = gbitmap_create_with_resource(RESOURCE_ID_FIVE);
  s_image_numbers[6] = gbitmap_create_with_resource(RESOURCE_ID_SIX);
  s_image_numbers[7] = gbitmap_create_with_resource(RESOURCE_ID_SEVEN);
  s_image_numbers[8] = gbitmap_create_with_resource(RESOURCE_ID_EIGHT);
  s_image_numbers[9] = gbitmap_create_with_resource(RESOURCE_ID_NINE);


  // Create BitmapLayer to display the GBitmap
  s_time_digits[0] = bitmap_layer_create(GRect(DIGIT_PADDING, 0, IMG_WIDTH, IMG_HEIGHT));
  bitmap_layer_set_bitmap(s_time_digits[0], s_image_numbers[0]);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_time_digits[0]));

  s_time_digits[1] = bitmap_layer_create(GRect(IMG_WIDTH + DIGIT_PADDING * 2, 0, IMG_WIDTH, IMG_HEIGHT));
  bitmap_layer_set_bitmap(s_time_digits[1], s_image_numbers[0]);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_time_digits[1]));


  s_time_digits[2] = bitmap_layer_create(GRect(DIGIT_PADDING, SCREEN_HEIGHT - IMG_HEIGHT, IMG_WIDTH, IMG_HEIGHT));
  bitmap_layer_set_bitmap(s_time_digits[2], s_image_numbers[0]);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_time_digits[2]));

  s_time_digits[3] = bitmap_layer_create(GRect(IMG_WIDTH + DIGIT_PADDING * 2, SCREEN_HEIGHT - IMG_HEIGHT, IMG_WIDTH, IMG_HEIGHT));
  bitmap_layer_set_bitmap(s_time_digits[3], s_image_numbers[0]);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_time_digits[3]));

  // Create battery TextLayer
  s_battery_layer = text_layer_create(
    GRect(DIGIT_PADDING, 50, IMG_WIDTH, 14));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text(s_battery_layer, "100%");
  
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_14));

  text_layer_set_font(s_battery_layer, s_battery_font);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));


  
  
  // Create date TextLayer
  s_date_layer = text_layer_create(
    GRect(0, IMG_HEIGHT, bounds.size.w, 24));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "Sat 14 Jun");
  
  // Create GFont
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RUBIK_24));

  // Apply to TextLayer
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Make sure the time is displayed from the start
  update_time();
  update_battery();
}
 
static void main_window_unload(Window *window) {
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_battery_font);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
  
  for (int i = 0; i < DIGIT_COUNT; i++) {
    if (s_time_digits[i] != NULL) {
      bitmap_layer_destroy(s_time_digits[i]);
    }
  }
  for (int i = 0; i < IMAGE_COUNT; i++) {
    if (s_image_numbers[i] != NULL) {
      gbitmap_destroy(s_image_numbers[i]);
    }
  }
}
 
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  if (tick_time->tm_min == 0 && !quiet_time_is_active()) {
    vibes_double_pulse();
  }
  
}

static void battery_handler(BatteryChargeState charge_state) {
  update_battery();
}
  
static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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