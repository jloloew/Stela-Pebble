#include <pebble.h>
#include "keys.h"
#include "JALAppMessage.h"
#include "JALWordList.h"


//TODO: fix this, make it nice
extern void change_to_book(void);

typedef struct {
	const char **words; // an array of strings, one for each word
	unsigned int block_index; // the index of the Block
} Block __attribute__((aligned));

static unsigned int block_size       = 300;
static unsigned int total_num_blocks = 0;

static Block *curr_block;
static Block *alt_block;
static int curr_word_index = -1; // -1 if the index isn't valid

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
				 const unsigned int block_index,
				 const unsigned int word_index)
{
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
	
	// Check if the block for this word is the curr_block.
	// If it doesn't go in the current block, put it into the alt_block.
	Block *dest_block; // the block to write the word into
	// check the index of the blocks we have
	if (block_index == curr_block->block_index) {
		dest_block = curr_block;
	} else if (block_index == alt_block->block_index) {
		dest_block = alt_block;
	} else {
		// we have no data for this saved block. Wipe the alt_block clean and use that.
		block_destroy(alt_block);
		alt_block = block_create();
		if (!alt_block) { // safety check
			APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Unable to allocate new alt_block.", __func__);
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
	// swap out blocks if we reach the end of the current block
	if (curr_word_index >= (signed)block_size) {
		// check if the alt_block is the next block
		if (alt_block->block_index == curr_block->block_index + 1) {
			swap_blocks(); // also sets curr_word_index to -1
			APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: Swapped blocks.", __func__);
		} else {
			return NULL; // no next word exists
		}
	}
	
	// request the next block, if necessary
	int words_remaining = block_size - (curr_word_index + 1);
	bool should_request = (words_remaining == (signed)WORDS_REMAINING_FOR_BLOCK_REQUEST);
	should_request = should_request || (block_size <= WORDS_REMAINING_FOR_BLOCK_REQUEST
										&& curr_word_index == 0);
	should_request = should_request && (alt_block->block_index != curr_block->block_index + 1);
	if (should_request) {
		appmesg_request_block(curr_block->block_index + 1);
	}
	
	// return the next word and increment the current word index
	const char *word = curr_block->words[curr_word_index + 1];
	if (word) {
		curr_word_index++;
	}
	return word;
}

	// returns NULL on error
const char * wl_prev_word(void)
{
	if (curr_word_index < 0) {
		return NULL;
	}
	
	// swap out blocks if we reach the beginning of the current block
	if (curr_word_index == 0) {
		if (curr_block->block_index == 0) {
			return NULL; // no previous block exists
		}
		if (alt_block->block_index == curr_block->block_index - 1) {
			swap_blocks();
			APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: Swapped blocks.", __func__);
			// set the current word index to the end of the new current block
			curr_word_index = block_size;
		} else {
			return NULL; // no previous word exists
		}
	}
	
	// safety check
	if (curr_word_index - 1 < 0) {
		return NULL;
	}
	
	// request the previous block, if necessary
	int words_remaining = block_size - (curr_word_index + 1);
	bool should_request = (words_remaining == (signed)WORDS_REMAINING_FOR_BLOCK_REQUEST);
	should_request = should_request || (block_size <= WORDS_REMAINING_FOR_BLOCK_REQUEST
										&& curr_word_index == (signed)block_size);
	should_request = should_request && (alt_block->block_index != curr_block->block_index - 1);
	if (should_request) {
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

unsigned int wl_get_block_size(void)
{
	return block_size;
}

void wl_set_block_size(const unsigned int num_words)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: Old size: %d, new size: %d", __func__, block_size, num_words);
	
	if (num_words == block_size) {
		return; // nothing to do
	}
	
	// calculate the absolute index of the current word
	int abs_word_index;
	if (curr_word_index < 0 || !curr_block) {
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
		new_block_index = abs_word_index / num_words;
		// set the curr_word_index and pause
		curr_word_index = abs_word_index % num_words;
		//TODO: pause
	}
	
	// destroy the old blocks
	block_destroy(curr_block);
	block_destroy(alt_block);
	// set the new size
	block_size = num_words;
	// create new, empty blocks
	curr_block = block_create();
	alt_block  = block_create();
	// request the new current block
	if (new_block_index >= 0) {
		appmesg_request_block((unsigned)new_block_index);
	}
}

unsigned int wl_get_total_num_blocks(void)
{
	return total_num_blocks;
}

void wl_set_total_num_blocks(const unsigned int new_total)
{
	total_num_blocks = new_total;
	
	// if we have a block past the max number, delete it
	if (alt_block->block_index >= new_total) {
		block_destroy(alt_block);
		alt_block = block_create();
	}
	if (curr_block->block_index >= new_total) {
		block_destroy(curr_block);
		curr_block = alt_block;
		alt_block  = block_create();
	}
}

/********* PRIVATE FUNCTIONS **********/

static Block * block_create(void)
{
	Block *new_block = (Block *)calloc(1, sizeof(Block));
	char **words = (char **)calloc(block_size, sizeof(char *));
	if (!new_block || !words) { // safety first
		free(new_block);
		free(words);
		return NULL;
	}
	new_block->words = (const char **)words;
	new_block->block_index = 0;
	return new_block;
}

static void block_destroy(Block *target)
{
	if (!target) { // safety check
		return;
	}
	
	for (unsigned int i = 0; i < block_size; i++) {
		char *word = (char *)(target->words[i]);
		free(word);
		target->words[i] = NULL;
	}
	free(target->words);
	target->words = NULL;
	free(target);
}

static void swap_blocks(void)
{
	// swap the curr and alt blocks
	Block *temp = curr_block;
	curr_block  = alt_block;
	alt_block   = temp;
	
	// update the current word index
	curr_word_index = -1;
}
