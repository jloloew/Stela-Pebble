#pragma once

	
void wl_init(void);

void wl_deinit(void);

void wl_reset(void);

/*!
 * Add the given word to the end of the specified block.
 * 
 * @param new_word The word to add to the list.
 * @param block_index The index of the block the word belongs to.
 * @param word_index The index of the word within the block.
 */
void wl_add_word(const char *new_word,
				 const uint32_t block_index,
				 const uint32_t word_index);

/*!
 * Get the next word, swapping out blocks if necessary.
 * Automatically request the next block from the phone.
 * 
 * @return NULL if there is no next word
 */
const char * wl_next_word(void);

/*!
 * Get the previous word, swapping out blocks if necessary.
 * Automatically request the previous block from the phone.
 * 
 * @return NULL if there is no next word
 */
const char * wl_prev_word(void);

uint32_t wl_get_block_size(void) __attribute__((__pure__));
void wl_set_block_size(const uint32_t new_block_size);

int32_t wl_get_total_num_blocks(void) __attribute__((__pure__));
void wl_set_total_num_blocks(const int32_t new_total);
