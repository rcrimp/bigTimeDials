#include <pebble.h>
  
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS  1
  
static Window *s_main_window;
static TextLayer *s_time_layer;
 
static GFont s_time_font;
 
static BitmapLayer *s_background_hour_0;

static GBitmap *s_image_numbers[10];
 
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
 
  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
 
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
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
  s_background_hour_0 = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_background_hour_0, s_image_numbers[0]);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_hour_0));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(
    GRect(0, 0, bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INTER_20));
 
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  
  // Make sure the time is displayed from the start
  update_time();
}
 
static void main_window_unload(Window *window) {
  fonts_unload_custom_font(s_time_font);
  
  gbitmap_destroy(s_image_numbers);
 
  bitmap_layer_destroy(s_background_hour_0);
  text_layer_destroy(s_time_layer);
}
 
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorWhite);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}
 
static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}
 
int main(void) {
  init();
  app_event_loop();
  deinit();
}