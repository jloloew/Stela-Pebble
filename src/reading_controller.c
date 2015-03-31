#include <pebble.h>
#include "reading_controller.h"
#include "reading_screen.h"
#include "JALWordList.h"


static const unsigned int reading_speed_min = 45;
static const unsigned int reading_speed_max = 240;

	/// In words per minute.
static unsigned int reading_speed;
static uint32_t reading_frequency;
static bool is_paused = true;
static bool is_rewinding = false;


static void timer_callback(void *data __attribute__((unused)));


void rc_did_receive_first_word()
{
	show_reading_screen();
	rc_set_reading_speed(75);
	rc_start_reading();
}

void rc_start_reading()
{
	rc_set_paused(false);
	app_timer_register(reading_frequency, timer_callback, NULL);
}


void rc_increase_reading_speed()
{
	unsigned int new_speed = rc_get_reading_speed() + 5;
	if (new_speed <= reading_speed_max) {
		rc_set_reading_speed(new_speed);
	}
}

void rc_decrease_reading_speed()
{
	unsigned int new_speed = rc_get_reading_speed() - 5;
	if (new_speed >= reading_speed_min) {
		rc_set_reading_speed(new_speed);
	}
}


bool rc_is_paused()
{
	return is_paused;
}

void rc_set_paused(bool paused)
{
	is_paused = paused;
	reading_set_paused(paused);
	if (!paused) {
		rc_start_reading();
	}
}

unsigned int rc_get_reading_speed()
{
	return reading_speed;
}

void rc_set_reading_speed(unsigned int new_reading_speed)
{
	reading_speed = new_reading_speed;
	// convert words per minute into milliseconds per word
	reading_frequency = (uint32_t)(1000.0 * 1.0 / (reading_speed / 60.0));
}

bool rc_is_rewinding()
{
	return is_rewinding;
}

void rc_set_rewinding(bool rewinding)
{
	is_rewinding = rewinding;
	reading_set_rewinding(rewinding);
}

static void timer_callback(void *data __attribute__((unused)))
{
	if (is_paused) {
		return;
	}
	
	const char *new_word;
	if (is_rewinding) {
		new_word = wl_prev_word();
	} else {
		new_word = wl_next_word();
	}
	reading_set_curr_word(new_word);
	// re-run the timer to change words again soon
	app_timer_register(reading_frequency, timer_callback, NULL);
}
