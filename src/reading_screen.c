#include "reading_screen.h"
#include <pebble.h>
#include "reading_controller.h"


	/// The initial position of the curr_word TextLayer.
static const GRect default_curr_word_frame = {
	{ 5,	65 },	// origin
	{ 134,	30 }	// size
};

static GBitmap *s_res_image_pause;

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_image_game_pane_white;
static GFont s_res_gothic_28;
static GBitmap *s_res_image_rewind;
static BitmapLayer *s_bitmapl_background;
static TextLayer *s_textl_curr_word;
static TextLayer *s_textl_info;
static BitmapLayer *s_bitmaplayer_rewind;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, 0);
  
  s_res_image_game_pane_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GAME_PANE_WHITE);
  s_res_gothic_28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  s_res_image_rewind = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REWIND);
  // s_bitmapl_background
  s_bitmapl_background = bitmap_layer_create(GRect(0, 0, 144, 164));
  bitmap_layer_set_bitmap(s_bitmapl_background, s_res_image_game_pane_white);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmapl_background);
  
  // s_textl_curr_word
  s_textl_curr_word = text_layer_create(GRect(26, 65, 134, 30));
  text_layer_set_background_color(s_textl_curr_word, GColorClear);
  text_layer_set_text(s_textl_curr_word, "Starting...");
  text_layer_set_font(s_textl_curr_word, s_res_gothic_28);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textl_curr_word);
  
  // s_textl_info
  s_textl_info = text_layer_create(GRect(18, 134, 100, 20));
  text_layer_set_background_color(s_textl_info, GColorClear);
  text_layer_set_text(s_textl_info, "Nothing to read.");
  text_layer_set_text_alignment(s_textl_info, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textl_info);
  
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
  bitmap_layer_destroy(s_bitmaplayer_rewind);
  gbitmap_destroy(s_res_image_game_pane_white);
  gbitmap_destroy(s_res_image_rewind);
}
// END AUTO-GENERATED UI CODE

/// Get the currently displayed word from the TextLayer.
const char * reading_get_curr_word()
{
	return text_layer_get_text(s_textl_curr_word);
}

/// Set the currently displayed word and shift it based on its length
/// to optically center it on-screen.
void reading_set_curr_word(const char *new_word)
{
	Layer *curr_word_root_layer = text_layer_get_layer(s_textl_curr_word);
	GRect new_frame = default_curr_word_frame;
	
	if (new_word) {
		// choose the new x offset based on the length of the string
		size_t word_len = strlen(new_word);
		switch (word_len) {
		case 0:
			// use default offset
			break;
		case 1:
			new_frame.origin.x = 43;
			break;
		case 2:
			new_frame.origin.x = 40;
			break;
		case 3 ... 4:
			new_frame.origin.x = 33;
			break;
		case 5 ... 6:
			new_frame.origin.x = 26;
			break;
		case 7 ... 8:
			new_frame.origin.x = 21;
			break;
		case 9 ... 10:
			new_frame.origin.x = 16;
			break;
		case 11 ... 13:
			new_frame.origin.x = 8;
			break;
		default: // 14+
			new_frame.origin.x = 4;
			break;
		}
	}
	// Set the new frame and text
	//TODO: does the order of these two lines matter? (graphically/performance-wise)
	layer_set_frame(curr_word_root_layer, new_frame);
	text_layer_set_text(s_textl_curr_word, new_word);
}

/// Set the font of the currently displayed word.
void reading_set_font(GFont font)
{
	text_layer_set_font(s_textl_curr_word, font);
}

void reading_set_paused(bool is_paused)
{
	if (is_paused) {
		bitmap_layer_set_bitmap(s_bitmaplayer_rewind, s_res_image_pause);
	} else {
		bitmap_layer_set_bitmap(s_bitmaplayer_rewind, NULL);
	}
}

void reading_set_rewinding(bool is_rewinding)
{
	if (is_rewinding) {
		bitmap_layer_set_bitmap(s_bitmaplayer_rewind, s_res_image_rewind);
	} else {
		reading_set_paused(rc_is_paused());
	}
}


static void handle_window_load(Window *window)
{
	// set up curr_word's TextLayer
	text_layer_set_overflow_mode(s_textl_curr_word, GTextOverflowModeFill);
}

static void handle_window_unload(Window* window)
{
	destroy_ui();
}

void show_reading_screen(void)
{
	s_res_image_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
	initialise_ui();
	text_layer_set_text(s_textl_info, "Nothing to read.");
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load	= handle_window_load,
		.unload	= handle_window_unload,
	});
	window_stack_push(s_window, true);
}

void hide_reading_screen(void)
{
	window_stack_remove(s_window, true);
}
