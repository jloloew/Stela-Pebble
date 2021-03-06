#include <pebble.h>
#include "keys.h"
#include "JALAppMessage.h"
#include "JALWordList.h"
#include "settings_screen.h"
#include "reading_controller.h"


#define ACCEL_STEP_MS     50
#define MAX_READING_SPEED 95
#define MIN_READING_SPEED 20


static Window *window;
static TextLayer *curr_reading_word_layer;
static TextLayer *info_text_layer;
static BitmapLayer *rewind_layer;
static BitmapLayer *image_layer;
Layer *window_layer;
static AppTimer *timer;
GBitmap *image;
GBitmap *font_banner;
GBitmap *game_bg;
GBitmap *rewind_icon;
GBitmap *pause_icon;
GBitmap *nothing_icon;
const char *fonts[3] = {
	FONT_KEY_GOTHIC_28,
	FONT_KEY_GOTHIC_28_BOLD,
	FONT_KEY_ROBOTO_CONDENSED_21
};
GFont disp_font;
int font_id = 0;
enum {
	MENU = 1,
	BOOK,
	SETTINGS
} frame;
int text_x       = -20;
int text_y       = 70;
static int16_t text_margin = 3; // the minimum spacing between the edge of the screen and the on-screen text.
int speed        = 40;
int con_timer    = 0;
int hold         = 5; // some sort of timer
int push_x       = 0;
bool started     = false;
bool pause       = true;
bool fast_rewind = false;
bool end         = false;
bool menu        = true;
	// The word currently being displayed on-screen.
const char *word = NULL;
	// The size of the word being displayed.
int size_of_word = 256;
	// The horizontal resolution of the Pebble's screen.
static int16_t pebble_window_width = 144;


	// type:
	// 0, 2 = forward
	// 1 = reverse
void redraw_text(int type)
{
	if (type == 0) {
		hold--;
	}
	
	if (hold < 0 || type > 0) {
		GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
		GSize text_size = { .w = 0, .h = 0 };
		
		if (type == 0) {
			word = wl_next_word() ?: "";
			size_of_word = strlen(word);
		} else if (type == 1) {
			word = wl_prev_word() ?: word;
			size_of_word = strlen(word);
		}
		
		if (end) {
			hold = 1000000;
		} else {
			hold = (125 / speed) + (size_of_word * 4 / speed);
			
			int shift_x;
			if (size_of_word < 0) {
				// error of some sort, or no word
				shift_x = 0;
			} else if (size_of_word <= 2) {
				shift_x = 1;
			} else if (size_of_word <= 5) {
				shift_x = 2;
			} else if (size_of_word <= 8) {
				shift_x = 3;
			} else if (size_of_word <= 12) {
				shift_x = 4;
			} else if (size_of_word <= 14) {
				shift_x = 5;
			} else if (size_of_word <= 16) {
				shift_x = 6;
			} else if (size_of_word <= 20) {
				shift_x = 7;
			} else { // size_of_word > 20
				shift_x = 8;
			}
			shift_x *= 4;
			
			text_size = graphics_text_layout_get_content_size(
				word, disp_font, move_pos2, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter
			);
			int16_t size_x = text_size.w;
			JL_DEBUG("Content size: (%d, %d)", text_size.w, text_size.h);
			
			push_x = (size_x / 2) - 2*shift_x + 5;
		}
		
		move_pos2 = (GRect) { .origin = { text_x + push_x - 5, text_y }, .size = { 180, 180 } };
		// constrain move_pos2 to the bounds of the window
		static int16_t max_text_width = 0;
		if (max_text_width == 0) {
			max_text_width = pebble_window_width - 2 * text_margin;
		}
		
		if (text_size.w > max_text_width) {
			JL_ERROR("Word is too wide for the screen! Max width is %d, word width is %d, word is %s",
					(int)max_text_width, (int)text_size.w, word);
		} else {
			int16_t center_x = move_pos2.origin.x + (move_pos2.size.w / 2);
			int16_t right_overshoot = (center_x + (text_size.w / 2)) - pebble_window_width - text_margin;
			if (right_overshoot > 0) {
				move_pos2.origin.x -= right_overshoot;
				JL_DEBUG("Corrected right overshoot of %d px.", (int)right_overshoot);
				// recalculate center
				center_x = move_pos2.origin.x + (move_pos2.size.w / 2);
			}
			int16_t left_overshoot = text_margin - (center_x - (text_size.w / 2));
			// move the word farther right if it's going off the left side of the screen
			if (left_overshoot > 0) {
				move_pos2.origin.x += left_overshoot;
				JL_DEBUG("Corrected left overshoot of %d px.", (int)left_overshoot);
				// recalculate center
				center_x = move_pos2.origin.x + (move_pos2.size.w / 2);
			}
		}
		
		text_layer_set_text(curr_reading_word_layer, word);
		
		layer_set_frame(text_layer_get_layer(curr_reading_word_layer), move_pos2);
		
		JL_DEBUG("Frame: origin = (%d, %d), size = (%d, %d)",
				 move_pos2.origin.x, move_pos2.origin.y,
				 move_pos2.size.w, move_pos2.size.h);
	}
}


// This is like our app's main loop.
// It gets the next word every so often.
static void timer_callback(void *data __attribute__((unused)))
{
	JL_DEBUG("timer_callback() called");
	
	if (!pause) {
		redraw_text(0);
	} else {
		if (fast_rewind) {
			redraw_text(1);
		}
	}
	
	if (con_timer > 0) {
		con_timer--;
		if (con_timer == 49) {
			layer_add_child(window_layer, text_layer_get_layer(info_text_layer));
		}
	} else {
		// only hide the info text if there's something useful to read
		if (word && strcmp(word, "") != 0) {
			layer_remove_from_parent(text_layer_get_layer(info_text_layer));
		}
	}
	
	// animation actionEvent
	timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}


static void change_to_menu()
{
	frame = MENU;
	text_layer_set_text(info_text_layer, "Waiting for Device...");
	text_layer_set_text(curr_reading_word_layer, "");
	
	// reading word
	GRect move_pos2 = (GRect) { .origin = { -15, 105 }, .size = { 180, 180 } };
	layer_set_frame(text_layer_get_layer(curr_reading_word_layer), move_pos2);
	// connection status text
	GRect move_pos3 = (GRect) { .origin = { -15, 130 }, .size = { 180, 180 } };
	layer_set_frame(text_layer_get_layer(info_text_layer), move_pos3);
	
	// stela logo
	GRect move_pos4 = (GRect) { .origin = { -18, -15 }, .size = { 180, 180 } };
	layer_set_frame(bitmap_layer_get_layer(image_layer), move_pos4);
	
	bitmap_layer_set_bitmap(image_layer, image);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
}


static void change_to_settings()
{
	frame = SETTINGS;
	text_layer_set_text(info_text_layer, "");
	text_layer_set_text(curr_reading_word_layer, "FONT");
	GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
	layer_set_frame(text_layer_get_layer(curr_reading_word_layer), move_pos2);
	GRect move_pos4 = (GRect) { .origin = { -18, -80 }, .size = { 180, 180 } };
	layer_set_frame(bitmap_layer_get_layer(image_layer), move_pos4);
	
	bitmap_layer_set_compositing_mode(image_layer, GCompOpAssign);
	bitmap_layer_set_bitmap(image_layer, font_banner);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
	
}


void change_to_book()
{
	frame = BOOK;
	started = true;
	pause = false;
	if (!strcmp(fonts[font_id], FONT_KEY_GOTHIC_28)) {
		text_x = -40;
		text_y = 70;
	} else if (!strcmp(fonts[font_id], FONT_KEY_GOTHIC_28_BOLD)) {
		text_x = -40;
		text_y = 70;
	} else if (!strcmp(fonts[font_id], FONT_KEY_ROBOTO_CONDENSED_21)) {
		text_x = -40;
		text_y = 73;
	} else {
		JL_ERROR("Bad font_id: %d", font_id);
	}
	
	// we haven't received any text to read yet, let the user know
	if (word) {
		text_layer_set_text(curr_reading_word_layer, "Starting...");
	} else {
		text_layer_set_text(info_text_layer, "Nothing to read.");
	}
	
	
	GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
	layer_set_frame(text_layer_get_layer(curr_reading_word_layer), move_pos2);
	
	GRect move_pos4 = (GRect) { .origin = { -18, 0 }, .size = { 180, 180 } };
	layer_set_frame(bitmap_layer_get_layer(image_layer), move_pos4);
	
	bitmap_layer_set_compositing_mode(image_layer, GCompOpClear);
	
	bitmap_layer_set_bitmap(image_layer, game_bg);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
	
#if READING_CONTROLLER_ENABLE
	rc_did_receive_first_word();
	
	
#else
	// start periodic updates to update the word that's currently on-screen
	timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
#endif // READING_CONTROLLER_ENABLE
}


void up_click_handler(ClickRecognizerRef recognizer, void *context __attribute__((unused)))
{
	switch (frame) {
	case MENU:
		change_to_settings();
		break;
	case SETTINGS:
		if (started) {
			change_to_book();
		} else {
			change_to_menu();
		}
		break;
	case BOOK:
#if READING_CONTROLLER_ENABLE
		rc_increase_reading_speed();
		break;
#else
		if (pause) {
			// do nothing
		} else {
			// increase reading speed
			if(speed < MAX_READING_SPEED) {
				speed += 5;
				con_timer = 50;
				// update the text label
				char label[10];
				snprintf(label, sizeof(label), "%d WPM", speed);
				text_layer_set_text(info_text_layer, label);
				//TODO: update the timer's fire interval
			}
		}
		break;
#endif // READING_CONTROLLER_ENABLE
	}
}


void middle_click_handler(ClickRecognizerRef recognizer, void *context __attribute__((unused)))
{
	switch (frame) {
		case MENU:
			change_to_book();
			break;
		case SETTINGS:
			font_id++;
			if (font_id > 2) {
				font_id = 0;
			}
			if (font_id == 0) {
				text_x = -20;
				text_y = 70;
			} else if (font_id == 1) {
				text_x = -20;
				text_y = 70;
			} else if (font_id == 2) {
				text_x = -20;
				text_y = 73;
			}
			
			disp_font = fonts_get_system_font(fonts[font_id]);
			GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
			layer_set_frame(text_layer_get_layer(curr_reading_word_layer), move_pos2);
			text_layer_set_font(curr_reading_word_layer, disp_font);
			break;
		case BOOK:
#if READING_CONTROLLER_ENABLE
			rc_set_paused(!rc_is_paused());
			break;
#else
			pause = !pause;
			if (pause == true) {
				bitmap_layer_set_bitmap(rewind_layer, pause_icon);
			} else {
				bitmap_layer_set_bitmap(rewind_layer, nothing_icon);
			}
			break;
#endif // READING_CONTROLLER_ENABLE
	}
}


void down_click_handler(ClickRecognizerRef recognizer, void *context __attribute__((unused)))
{
	switch (frame) {
	case MENU:
		// do nothing
		break;
	case SETTINGS:
		// do nothing
		break;
	case BOOK:
#if READING_CONTROLLER_ENABLE
		if (rc_is_paused()) {
			rc_set_rewinding(true);
		} else {
			rc_decrease_reading_speed();
		}
		break;
#else
		if (pause) {
			// start rewinding
			end = false;
			hold = 0;
			redraw_text(1);
			fast_rewind = true;
			bitmap_layer_set_bitmap(rewind_layer, rewind_icon);
		} else {
			// decrease reading speed
			if (speed > MIN_READING_SPEED) {
				speed -= 5;
				con_timer = 50;
				// update the text label
			char label[10];
			snprintf(label, sizeof(label), "%d WPM", speed);
			text_layer_set_text(info_text_layer, label);
				//TODO: update the timer's fire interval
			}
		}
		break;
#endif // READING_CONTROLLER_ENABLE
	}
}


void down_up_click_handler(ClickRecognizerRef recognizer, void *context __attribute__((unused)))
{
#if READING_CONTROLLER_ENABLE
	rc_set_rewinding(false);
#else
	fast_rewind = false;
	if (pause) {
		bitmap_layer_set_bitmap(rewind_layer, pause_icon);
	}
#endif // READING_CONTROLLER_ENABLE
}


void config_provider(void *context __attribute__((unused)))
{
	window_single_click_subscribe(BUTTON_ID_SELECT,   middle_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP,       up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN,     down_click_handler);
	window_long_click_subscribe(  BUTTON_ID_DOWN, 50, down_click_handler, down_up_click_handler);
}


static void init()
{
	// Set up the UI
	
	window = window_create();
	window_set_fullscreen(window, true);
	window_stack_push(window, true /* animated */);
	window_set_click_config_provider(window, config_provider);
	window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	curr_reading_word_layer	= text_layer_create(bounds);
	info_text_layer			= text_layer_create(bounds);
	image_layer				= bitmap_layer_create(bounds);
	rewind_layer			= bitmap_layer_create(bounds);
	
	image			= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STELA_ICON);
	font_banner		= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FONT_BANNER);
	game_bg			= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GAME_PANE_BLACK);
	rewind_icon		= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REWIND);
	pause_icon		= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
	nothing_icon	= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOTHING);
	
	disp_font = fonts_get_system_font(fonts[0]);
	text_layer_set_font(curr_reading_word_layer, disp_font);
	
	bitmap_layer_set_bitmap(rewind_layer, nothing_icon);
	bitmap_layer_set_alignment(rewind_layer, GAlignCenter);
	GRect move_pos2 = (GRect) { .origin = { -20, -60 }, .size = { 180, 180 } };
	layer_set_frame(bitmap_layer_get_layer(rewind_layer), move_pos2);
	
	change_to_menu();
	
	text_layer_set_text_alignment(curr_reading_word_layer,	GTextAlignmentCenter);
	text_layer_set_text_alignment(info_text_layer,			GTextAlignmentCenter);
	text_layer_set_overflow_mode(info_text_layer,			GTextOverflowModeWordWrap);
	
	layer_add_child(window_layer, text_layer_get_layer(curr_reading_word_layer));
	layer_add_child(window_layer, text_layer_get_layer(info_text_layer));
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
	layer_add_child(window_layer, bitmap_layer_get_layer(rewind_layer));
	
	// Set up the word list
	wl_init();
	
	// set up AppMessage
	appmesg_init();
	
	JL_DEBUG("Heap bytes used/free: %d/%d", heap_bytes_used(), heap_bytes_free());
}


static void deinit()
{
	appmesg_deinit();
	
	// Delete the word list
	wl_deinit();
	
	gbitmap_destroy(image);
	gbitmap_destroy(game_bg);
	gbitmap_destroy(font_banner);
	gbitmap_destroy(rewind_icon);
	gbitmap_destroy(pause_icon);
	gbitmap_destroy(nothing_icon);
	
	bitmap_layer_destroy(image_layer);
	bitmap_layer_destroy(rewind_layer);
	text_layer_destroy(curr_reading_word_layer);
	text_layer_destroy(info_text_layer);
	window_destroy(window);
}


int main(void)
{
	init();
	app_event_loop();
	deinit();
}
