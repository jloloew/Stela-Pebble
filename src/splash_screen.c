#include "splash_screen.h"
#include <pebble.h>


// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static TextLayer *s_textl_status;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, true);
  
  // s_textl_status
  s_textl_status = text_layer_create(GRect(0, 130, 144, 20));
  text_layer_set_text(s_textl_status, "Waiting for Device...");
  text_layer_set_text_alignment(s_textl_status, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textl_status);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textl_status);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_splash_screen(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_splash_screen(void) {
  window_stack_remove(s_window, true);
}
