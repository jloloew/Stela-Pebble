#include <pebble.h>
#include "utils.h"
#include "JALAppMessage.h"
#include "JALWordList.h"


//TODO: fix this, make it nice
extern void change_to_book(void);

typedef struct {
	const char **words;		///< An array of strings, one for each word.
	int32_t block_index;	///< The index of the Block.
} Block __attribute__((aligned));

	/// The number of words each block holds.
static uint32_t block_size		= 200;
	/// The total number of blocks that make up the entire text of what we're reading.
static int32_t total_num_blocks	= 0;

static Block *curr_block;	///< The block we're currently reading from.
static Block *alt_block;	///< This block is used as a buffer.
static int32_t curr_word_index = -1; ///< -1 if the index isn't valid

static bool wl_should_request_next_block(const bool is_rewinding) __attribute__((pure));
static bool wl_should_swap_blocks(const bool is_rewinding) __attribute__((pure));
static Block * block_create(void);
static void block_destroy(Block *target) __attribute__((nonnull));
static void swap_blocks(void);


/********** PUBLIC FUNCTIONS **********/

void wl_init(void)
{
	curr_block = block_create();
	alt_block  = block_create();
}

void wl_deinit(void)
{
	block_destroy(alt_block);
	block_destroy(curr_block);
	alt_block = curr_block = NULL;
}

void wl_reset(void)
{
	// destroy the old blocks
	block_destroy(curr_block);
	block_destroy(alt_block);
	// create new, empty blocks
	curr_block = block_create();
	alt_block  = block_create();
	// reset state
	curr_word_index = -1;
	wl_set_total_num_blocks(0);
	// keep the current block_size
}

	// does NOT make a copy the word
	// returns true on success, false otherwise
void wl_add_word(const char *new_word,
				 const uint32_t block_index,
				 const uint32_t word_index)
{
	JL_DEBUG("reached.");
	
	// safety checks
	if (word_index >= block_size || !new_word) {
		return;
	}
	
	// Start reading the first time this method is called.
	static bool change_to_book_done = false;
	if (!change_to_book_done) {
		change_to_book_done = true;
		change_to_book();
	}
	
	JL_DEBUG("reached.");
	
	// Check if the block for this word is the curr_block.
	// If it doesn't go in the current block, put it into the alt_block.
	Block *dest_block; // the block to write the word into
	// check the index of the blocks we have
	if ((signed)block_index == curr_block->block_index) {
		dest_block = curr_block;
	} else if ((signed)block_index == alt_block->block_index) {
		dest_block = alt_block;
	} else {
		JL_DEBUG("reached.");
		
		// we have no data for this saved block. Wipe the alt_block clean and use that.
		block_destroy(alt_block);
		
		JL_DEBUG("reached.");
		
		alt_block = block_create();
		if (!alt_block) { // safety check
			JL_ERROR("Unable to allocate new alt_block.");
			return;
		}
		// set up the new alt_block
		alt_block->block_index = block_index;
		dest_block = alt_block;
	}
	
	// actually add the word to the block
	free((char *)dest_block->words[word_index]);
	dest_block->words[word_index] = new_word;
}

	// returns NULL on error
const char * wl_next_word(void)
{
	int32_t next_block_index = curr_block->block_index + 1;
	int32_t next_word_index = curr_word_index + 1;
	
	bool is_rewinding = false;
	if (wl_should_request_next_block(is_rewinding)) {
		appmesg_request_block(next_block_index);
	}
	if (wl_should_swap_blocks(is_rewinding)) {
		swap_blocks();
		next_word_index = 0;
	}
	
	// make sure next_word_index is valid
	if (next_word_index < (signed)block_size) {
		// return the next word and set the current word index
		const char *next_word = curr_block->words[next_word_index];
		if (next_word) {
			curr_word_index = next_word_index;
			return next_word;
		}
	}
	// unable to get a valid word
	return NULL;
}

	// returns NULL on error
const char * wl_prev_word(void)
{
	// swap out blocks if we reach the beginning of the current block
	if (curr_word_index - 1 < 0) {
		if (alt_block->block_index < 0) {
			JL_DEBUG("Unable to switch to previous block.");
			return NULL; // no previous block exists
		}
		if (alt_block->block_index == curr_block->block_index - 1) {
			swap_blocks();
			// set the current word index to the end of the new current block
			curr_word_index = block_size;
		} else {
			JL_DEBUG("Unable to switch to previous block.");
			return NULL; // no previous word exists
		}
	}
	
	// request the previous block, if necessary
	if (wl_should_request_next_block(true)) {
		if (curr_block->block_index > 0) {
			appmesg_request_block(curr_block->block_index - 1);
		}
	}
	
	// return the previous word and decrement the current word index
	const char *word = curr_block->words[curr_word_index - 1];
	if (word) {
		curr_word_index--;
	}
	return word;
}

uint32_t wl_get_block_size(void)
{
	return block_size;
}

void wl_set_block_size(const uint32_t new_block_size)
{
	JL_DEBUG("Old size: %u, new size: %u",
			 (unsigned int)block_size, (unsigned int)new_block_size);
	
	if (new_block_size == block_size) {
		return; // nothing to do
	}
	
	/*
	// calculate the absolute index of the current word
	int abs_word_index;
	if (curr_word_index < 0 || !curr_block || curr_block->block_index < 0) {
		abs_word_index = -1;
	} else {
		abs_word_index = block_size * curr_block->block_index;
		abs_word_index += curr_word_index;
	}
	// calculate the block index of the current word
	int new_block_index;
	if (abs_word_index < 0) {
		new_block_index = -1;
	} else {
		new_block_index = abs_word_index / new_block_size;
		// set the curr_word_index and pause
		curr_word_index = abs_word_index % new_block_size;
		//TODO: pause
	}
	*/
	
	// destroy the old blocks
	block_destroy(curr_block);
	block_destroy(alt_block);
	// set the new size
	block_size = new_block_size;
	// create new, empty blocks
	curr_block = block_create();
	alt_block  = block_create();
	
	/*
	// request the new current block
	if (new_block_index >= 0) {
		appmesg_request_block((unsigned)new_block_index);
	}
	*/
}

int32_t wl_get_total_num_blocks(void)
{
	return total_num_blocks;
}

void wl_set_total_num_blocks(const int32_t new_total)
{
	if (new_total == total_num_blocks) {
		return; // nothing to do
	}
	
	total_num_blocks = new_total;
	
	// if we have a block past the max number, delete it
	if (alt_block->block_index >= new_total) {
		block_destroy(alt_block);
		alt_block = block_create();
	}
	if (curr_block->block_index >= new_total) {
		// if alt_block was still valid, make it the current block
		block_destroy(curr_block);
		curr_block = alt_block;
		alt_block  = block_create();
	}
}

/********* PRIVATE FUNCTIONS **********/

static bool wl_should_request_next_block(const bool is_rewinding)
{
	int words_remaining;
	bool next_block_exists, next_block_is_possessed, block_size_too_small;
	if (is_rewinding) {
		words_remaining = curr_word_index;
		next_block_exists = (curr_block->block_index - 1 >= 0);
		next_block_is_possessed = (alt_block->block_index == curr_block->block_index - 1);
		block_size_too_small = (((int32_t)block_size <= WORDS_REMAINING_FOR_BLOCK_REQUEST)
								&& (curr_word_index == (int32_t)block_size - 1));
	} else {
		words_remaining = block_size - (curr_word_index + 1);
		next_block_exists = (curr_block->block_index + 1 < total_num_blocks);
		next_block_is_possessed = (alt_block->block_index == curr_block->block_index + 1);
		block_size_too_small = (((int32_t)block_size <= WORDS_REMAINING_FOR_BLOCK_REQUEST)
								&& (curr_word_index == 0));
	}
	bool words_remaining_at_cutoff = (words_remaining == (signed)WORDS_REMAINING_FOR_BLOCK_REQUEST);
	
	bool word_count_trigger = block_size_too_small || words_remaining_at_cutoff;
	bool should_request = next_block_exists && !next_block_is_possessed && word_count_trigger;
	
	return should_request;
}

static bool wl_should_swap_blocks(const bool is_rewinding)
{
	int32_t next_block_index;
	bool next_block_exists, reached_end_of_curr_block;
	if (is_rewinding) {
		next_block_index = curr_block->block_index - 1;
		next_block_exists = (next_block_index >= 0); // prevent matches when alt_block->block_index == -1
		reached_end_of_curr_block = (curr_word_index == 0);
	} else {
		next_block_index = curr_block->block_index + 1;
		next_block_exists = (next_block_index < total_num_blocks);
		reached_end_of_curr_block = (curr_word_index == (int32_t)block_size - 1);
	}
	bool next_block_is_possessed = next_block_exists && (alt_block->block_index == next_block_index);
	bool should_swap = next_block_is_possessed && reached_end_of_curr_block;

	return should_swap;
}

static Block * block_create(void)
{
	JL_DEBUG("Creating new block.");
	
	Block *new_block = (Block *)calloc(1, sizeof(Block));
	char **words = (char **)calloc(block_size, sizeof(char *));
	if (!new_block || !words) { // safety first
		goto error;
	}
	new_block->words = (const char **)words;
	new_block->block_index = -1;
	return new_block;
error:
	free(new_block);
	free(words);
	return NULL;
}

static void block_destroy(Block *target)
{
	JL_DEBUG("Destroying block %p", target);
	if (target == curr_block) {
		JL_DEBUG("Destroying curr_block");
		curr_block = NULL;
	}
	if (target == alt_block) {
		JL_DEBUG("Destroying alt_block");
		alt_block = NULL;
	}
	
	if (!target) { // safety check
		return;
	}
	
	for (unsigned int i = 0; i < ARRAY_SIZE(target->words); i++) {
		char *word = (char *)target->words[i];
		free(word);
		target->words[i] = NULL;
	}
	free(target->words);
	target->words = NULL;
	free(target);
}

	// The caller should reset curr_word_index.
static void swap_blocks(void)
{
	// swap the curr and alt blocks
	Block *temp = curr_block;
	curr_block  = alt_block;
	alt_block   = temp;
	
	JL_DEBUG("Swapped blocks. Curr is now block %d and alt is block %d.",
			 (int)curr_block->block_index, (int)alt_block->block_index);
}
