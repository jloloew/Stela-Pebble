#include "reading_screen.h"
#include <pebble.h>

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_image_game_pane_white;
static GFont s_res_gothic_28;
static GBitmap *s_res_image_pause;
static GBitmap *s_res_image_rewind;
static BitmapLayer *s_bitmapl_background;
static TextLayer *s_textl_curr_word;
static TextLayer *s_textl_info;
static BitmapLayer *s_bitmaplayer_pause;
static BitmapLayer *s_bitmaplayer_rewind;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, 0);
  
  s_res_image_game_pane_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GAME_PANE_WHITE);
  s_res_gothic_28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  s_res_image_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
  s_res_image_rewind = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REWIND);
  // s_bitmapl_background
  s_bitmapl_background = bitmap_layer_create(GRect(0, 0, 144, 164));
  bitmap_layer_set_bitmap(s_bitmapl_background, s_res_image_game_pane_white);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmapl_background);
  
  // s_textl_curr_word
  s_textl_curr_word = text_layer_create(GRect(-40, 70, 180, 180));
  text_layer_set_background_color(s_textl_curr_word, GColorClear);
  text_layer_set_text(s_textl_curr_word, "Starting...");
  text_layer_set_text_alignment(s_textl_curr_word, GTextAlignmentCenter);
  text_layer_set_font(s_textl_curr_word, s_res_gothic_28);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textl_curr_word);
  
  // s_textl_info
  s_textl_info = text_layer_create(GRect(18, 134, 100, 20));
  text_layer_set_background_color(s_textl_info, GColorClear);
  text_layer_set_text(s_textl_info, "Nothing to read.");
  text_layer_set_text_alignment(s_textl_info, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textl_info);
  
  // s_bitmaplayer_pause
  s_bitmaplayer_pause = bitmap_layer_create(GRect(59, 8, 26, 23));
  bitmap_layer_set_bitmap(s_bitmaplayer_pause, s_res_image_pause);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_pause);
  
  // s_bitmaplayer_rewind
  s_bitmaplayer_rewind = bitmap_layer_create(GRect(59, 20, 26, 23));
  bitmap_layer_set_bitmap(s_bitmaplayer_rewind, s_res_image_rewind);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_rewind);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  bitmap_layer_destroy(s_bitmapl_background);
  text_layer_destroy(s_textl_curr_word);
  text_layer_destroy(s_textl_info);
  bitmap_layer_destroy(s_bitmaplayer_pause);
  bitmap_layer_destroy(s_bitmaplayer_rewind);
  gbitmap_destroy(s_res_image_game_pane_white);
  gbitmap_destroy(s_res_image_pause);
  gbitmap_destroy(s_res_image_rewind);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_reading_screen(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_reading_screen(void) {
  window_stack_remove(s_window, true);
}
