#pragma once
#include <pebble.h>


	// Get and set the word currently displayed on-screen.
const char * reading_get_curr_word() __attribute__((pure));
void reading_set_curr_word(const char *new_word);

void reading_set_font(GFont font);

void reading_set_paused(bool is_paused);
void reading_set_rewinding(bool is_rewinding);

void show_reading_screen(void);
void hide_reading_screen(void);
