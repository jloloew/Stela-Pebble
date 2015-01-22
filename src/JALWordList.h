#pragma once

	
void wl_init();

void wl_deinit();

void wl_reset();

/// Add the given word to the end of the specified block.
/// 
/// @param newWord The word to add to the list.
/// @param block_index The index of the block the word belongs to.
/// @return 0 on success; otherwise, an error code.
void wl_add_word(const char *new_word,
				 const unsigned int block_index,
				 const unsigned int word_index);

/// Get the next word, swapping out blocks if necessary.
/// Automatically request the next block from the phone.
/// 
/// @return NULL if there is no next word
const char * wl_next_word();

/// Get the previous word, swapping out blocks if necessary.
/// Automatically request the previous block from the phone.
/// 
/// @return NULL if there is no next word
const char * wl_prev_word();

unsigned int wl_get_block_size() __attribute__((__pure__));
void wl_set_block_size(const unsigned int num_words);

unsigned int wl_get_total_num_blocks() __attribute__((__pure__));
void wl_set_total_num_blocks(const unsigned int new_total);
