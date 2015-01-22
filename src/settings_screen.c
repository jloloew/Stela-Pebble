#include "settings_screen.h"
#include <pebble.h>


// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_image_font_banner;
static GBitmap *s_res_droid_serif_28_bold_preview_white;
static GBitmap *s_res_gothic_28_preview_white;
static GBitmap *s_res_gothic_28_bold_preview_white;
static GFont s_res_gothic_28_bold;
static BitmapLayer *s_bmpl_font_banner;
static ActionBarLayer *s_actionbarl_font_choice;
static InverterLayer *s_inverterlayer_1;
static TextLayer *s_textl_font_example_text;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, true);
  
  s_res_image_font_banner = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FONT_BANNER);
  s_res_droid_serif_28_bold_preview_white = gbitmap_create_with_resource(RESOURCE_ID_DROID_SERIF_28_BOLD_PREVIEW_WHITE);
  s_res_gothic_28_preview_white = gbitmap_create_with_resource(RESOURCE_ID_GOTHIC_28_PREVIEW_WHITE);
  s_res_gothic_28_bold_preview_white = gbitmap_create_with_resource(RESOURCE_ID_GOTHIC_28_BOLD_PREVIEW_WHITE);
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  // s_bmpl_font_banner
  s_bmpl_font_banner = bitmap_layer_create(GRect(0, 0, 120, 23));
  bitmap_layer_set_bitmap(s_bmpl_font_banner, s_res_image_font_banner);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bmpl_font_banner);
  
  // s_actionbarl_font_choice
  s_actionbarl_font_choice = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actionbarl_font_choice, s_window);
  action_bar_layer_set_background_color(s_actionbarl_font_choice, GColorBlack);
  action_bar_layer_set_icon(s_actionbarl_font_choice, BUTTON_ID_UP, s_res_droid_serif_28_bold_preview_white);
  action_bar_layer_set_icon(s_actionbarl_font_choice, BUTTON_ID_SELECT, s_res_gothic_28_preview_white);
  action_bar_layer_set_icon(s_actionbarl_font_choice, BUTTON_ID_DOWN, s_res_gothic_28_bold_preview_white);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_actionbarl_font_choice);
  
  // s_inverterlayer_1
  s_inverterlayer_1 = inverter_layer_create(GRect(0, 0, 120, 23));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_inverterlayer_1);
  
  // s_textl_font_example_text
  s_textl_font_example_text = text_layer_create(GRect(0, 76, 124, 28));
  text_layer_set_text(s_textl_font_example_text, "A   F   S");
  text_layer_set_text_alignment(s_textl_font_example_text, GTextAlignmentCenter);
  text_layer_set_font(s_textl_font_example_text, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textl_font_example_text);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  bitmap_layer_destroy(s_bmpl_font_banner);
  action_bar_layer_destroy(s_actionbarl_font_choice);
  inverter_layer_destroy(s_inverterlayer_1);
  text_layer_destroy(s_textl_font_example_text);
  gbitmap_destroy(s_res_image_font_banner);
  gbitmap_destroy(s_res_droid_serif_28_bold_preview_white);
  gbitmap_destroy(s_res_gothic_28_preview_white);
  gbitmap_destroy(s_res_gothic_28_bold_preview_white);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_settings_screen(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_settings_screen(void) {
  window_stack_remove(s_window, true);
}
