#pragma once


void rc_did_receive_first_word();
void rc_start_reading();

void rc_increase_reading_speed();
void rc_decrease_reading_speed();


bool rc_is_paused() __attribute__((pure));
void rc_set_paused(bool paused);

unsigned int rc_get_reading_speed() __attribute__((pure));
void rc_set_reading_speed(unsigned int new_reading_speed);

bool rc_is_rewinding() __attribute__((pure));
void rc_set_rewinding(bool rewinding);
