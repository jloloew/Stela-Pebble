#include "splash_screen.h"
#include <pebble.h>


// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, 0);
}

static void destroy_ui(void) {
  window_destroy(s_window);
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
