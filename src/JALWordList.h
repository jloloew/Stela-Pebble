#pragma once

void wl_init();

void wl_deinit();

/// Make a copy of the given word and append it to the end of the list.
/// 
/// @param newWord The word to add to the list.
/// @return A pointer to the copy of the word.
const char * wl_add_word(const char *newWord);

const char * wl_next_word();

const char * wl_prev_word();
