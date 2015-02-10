//#include <pebble.h>
#include "utils.h"
#include "JALAppMessage.h"
#include "JALWordList.h"


static Version phone_version = { .major = 0, .minor = 0, .patch = 0 };

// Receiving Messages
static void receive_block_size_message(Tuple *tuple)	__attribute__((nonnull));
static void receive_words(DictionaryIterator *iterator)	__attribute__((nonnull));
static void receive_error(Tuple *tuple)					__attribute__((nonnull));
static void receive_version_number(Tuple *tuple)		__attribute__((nonnull));
// Sending Messages
static void send_version_number(void);
	// Helpers for sending messages.
static DictionaryIterator * _appmesg_send_helper1(void);
static void _appmesg_send_helper2(DictionaryIterator *iterator,
								  const DictionaryResult dict_result) __attribute__((nonnull));
static void _appmesg_send_int(    const int key, const int32_t     value);
static void _appmesg_send_uint(   const int key, const uint32_t    value);
static void _appmesg_send_cstring(const int key, const char *const value) __attribute__((nonnull));

// Callbacks
	// callback for when a new message comes in from the phone
void appmesg_received_handler(DictionaryIterator *iterator, void *context __attribute__((unused)));
	// callback for when an error occurs while trying to receive a new message
void appmesg_not_received_handler(AppMessageResult reason, void *context __attribute__((unused)));
	// Successfully sent a message.
void appmesg_sent_handler(DictionaryIterator *iterator, void *context __attribute__((unused)));
	// An error occurred while sending a message.
void appmesg_not_sent_handler(DictionaryIterator *iterator,
							  AppMessageResult reason,
							  void *context __attribute__((unused)));


/********** PUBLIC FUNCTIONS **********/

void appmesg_init(void)
{
	// register AppMessage handlers
	app_message_register_inbox_received(appmesg_received_handler);
	app_message_register_inbox_dropped(appmesg_not_received_handler);
	app_message_register_outbox_sent(appmesg_sent_handler);
	app_message_register_outbox_failed(appmesg_not_sent_handler);
	
	// open AppMessage communication
	AppMessageResult open_result = app_message_open(APPMESG_INBOX_SIZE, APPMESG_OUTBOX_SIZE);
	if (open_result == APP_MSG_OK) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMessage opened successfully.");
	} else {
		APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage failed to open.");
		//TODO: update UI to say "not connected" or something
	}
}

void appmesg_deinit(void)
{
	app_message_deregister_callbacks();
}

void appmesg_request_block(const unsigned int index)
{
	_appmesg_send_uint(APPMESG_BLOCK_NUMBER_KEY, index);
}

void appmesg_send_block_size(const int block_size)
{
	_appmesg_send_int(TEXT_BLOCK_SIZE_KEY, block_size);
}

void appmesg_send_error(const char *const error_msg)
{
	if (error_msg) {
		_appmesg_send_cstring(ERROR_KEY, error_msg);
	}
}

/********** PRIVATE FUNCTIONS **********/

/********** RECEIVE HELPERS **********/

	// When the phone receives a positive size, it sets the size and replies with its negated new size.
	// When the watch receives a positive size, it sets the size and replies with its negated new size.
	// When the phone receives a negative size, it does nothing.
	// When the watch receives a negative size, it replies with its negated current size.
static void receive_block_size_message(Tuple *tuple)
{
	// verify the value's type and size are correct
	TupleType value_type = tuple->type;
	if (value_type != TUPLE_INT) {
		APP_LOG(APP_LOG_LEVEL_ERROR,
				"%s: Requested block size is not a signed int value.",
				__func__);
		return;
	}
	if (tuple->length != 4) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Requested block size is not a 32-bit value.", __func__);
		return;
	}
	
	// If the requested block size is positive, it's a command to set the block size.
	// If it's negative or zero, it's a request to send our current block size.
	int value = tuple->value->int32;
	
	// If the value is a command (positive), change our block size.
	if (value > 0) {
		wl_set_block_size((unsigned int)value);
		// negate it before we send it, so it's not a command.
		value = -value;
	} else {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: The phone's block size is %d.", __func__, -value);
	}
	
	// Send our block size to the phone, whether or not we changed ours.
	appmesg_send_block_size(wl_get_block_size());
}

static void receive_words(DictionaryIterator *iterator)
{
	int block_number = -1;
	unsigned start_index = 0, num_words = 0, first_word_key = 0;
	
	// get the block number
	Tuple *curr_tuple = dict_find(iterator, (unsigned)APPMESG_BLOCK_NUMBER_KEY);
	if (curr_tuple) {
		block_number = curr_tuple->value->int32;
	}
	// get the starting index of the word within the block
	curr_tuple = dict_find(iterator, (unsigned)APPMESG_WORD_START_INDEX_KEY);
	if (curr_tuple) {
		start_index = curr_tuple->value->uint32;
	}
	// get the number of words
	curr_tuple = dict_find(iterator, (unsigned)APPMESG_NUM_WORDS_KEY);
	if (curr_tuple) {
		num_words = curr_tuple->value->uint32;
	}
	// get the key for the first actual word in this message
	curr_tuple = dict_find(iterator, (unsigned)APPMESG_FIRST_WORD_KEY);
	if (curr_tuple) {
		first_word_key = curr_tuple->value->uint32;
	}
	
	// get each word, in order
	unsigned word_num = first_word_key;
	while (word_num < num_words) {
		curr_tuple = dict_find(iterator, word_num);
		
		// copy the string out of the Tuple
		uint16_t word_length = curr_tuple->length;
		if (word_length == 0) {
			word_num++;
			continue;
		}
		char *copied_word = malloc(word_length * sizeof(char));
		if (!copied_word) {
			APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Out of memory.", __func__);
		}
		strncpy(copied_word, (const char *)&curr_tuple->value->cstring, word_length);
		
		// add the word to the block
		wl_add_word(copied_word, block_number, start_index + word_num);
		
		word_num++;
	}
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: Added %d words.", __func__, word_num);
}

static void receive_error(Tuple *tuple)
{
	if (tuple->type == TUPLE_CSTRING) {
		const char *error_message = (const char *)&tuple->value->cstring;
		APP_LOG(APP_LOG_LEVEL_ERROR, "%s: %s", __func__, error_message);
	} else {
		APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Received non-string error.", __func__);
	}
}

static void receive_version_number(Tuple *tuple)
{
	const char *ver_str = (const char *)&tuple->value->cstring;
	// parse out the version
	const Version ver = string_to_version(ver_str);
	
	// If the major is 0, it's a request for our version number.
	// Else, the phone is sending its version number.
	if (ver.major == 0) {
		send_version_number();
	} else {
		phone_version = ver;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: Phone has version %s", __func__, ver_str);
	}
}

/********** SEND HELPERS **********/

static void send_version_number(void)
{
	// stringify the version number
	char *ver_str = version_to_string(PEBBLE_STELA_VERSION);
	
	_appmesg_send_cstring(STELA_VERSION_KEY, ver_str);
	
	free(ver_str);
}

static DictionaryIterator * _appmesg_send_helper1(void)
{
	DictionaryIterator *iterator = NULL;
	
	// open the outbox to prepare to send a message
	AppMessageResult appmesg_result = app_message_outbox_begin(&iterator);
	if (appmesg_result != APP_MSG_OK) {
		// failure
		APP_LOG(APP_LOG_LEVEL_ERROR,
				"%s: Unable to open outbox while preparing: %s",
				__func__,
				stringify_AppMessageResult(appmesg_result));
	}
	
	return iterator;
}

static void _appmesg_send_helper2(DictionaryIterator *iterator, const DictionaryResult dict_result)
{
	if (dict_result != DICT_OK) { // error handling
		APP_LOG(APP_LOG_LEVEL_ERROR,
				"%s: Dict error: %s",
				__func__,
				stringify_DictResult(dict_result));
		goto fail;
	}
	
	// send the message
	AppMessageResult appmesg_result = app_message_outbox_send();
	if (appmesg_result != APP_MSG_OK) {
		APP_LOG(APP_LOG_LEVEL_ERROR,
				"%s: Error sending message: %s",
				__func__,
				stringify_AppMessageResult(appmesg_result));
	} else {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Successfully sent message.");
	}
	
fail:
	free(iterator);
	return;
}

static void _appmesg_send_int(const int key, const int32_t value)
{
	DictionaryIterator *iterator = _appmesg_send_helper1();
	// add the data to the outgoing message
	DictionaryResult dict_result = dict_write_int32(iterator,
													(uint32_t)key,
													value);
	_appmesg_send_helper2(iterator, dict_result);
}

static void _appmesg_send_uint(const int key, const uint32_t value)
{
	DictionaryIterator *iterator = _appmesg_send_helper1();
	// add the data to the outgoing message
	DictionaryResult dict_result = dict_write_uint32(iterator,
													 (uint32_t)key,
													 value);
	_appmesg_send_helper2(iterator, dict_result);
}

static void _appmesg_send_cstring(const int key, const char *const value)
{
	DictionaryIterator *iterator = _appmesg_send_helper1();
	// add the data to the outgoing message
	DictionaryResult dict_result = dict_write_cstring(iterator,
													  (uint32_t)key,
													  value);
	_appmesg_send_helper2(iterator, dict_result);
}

/********** CALLBACKS **********/

	// Route the received iterator to the correct function for processing.
void appmesg_received_handler(DictionaryIterator *iterator, void *context)
{
	// safety first
	if (!iterator) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Received NULL dictionary.", __func__);
		return;
	}
	
	// check for an error
	Tuple *curr_tuple = dict_find(iterator, (unsigned)ERROR_KEY);
	if (curr_tuple) {
		receive_error(curr_tuple);
		return;
	}
	
	// check for a version number
	curr_tuple = dict_find(iterator, (unsigned)STELA_VERSION_KEY);
	if (curr_tuple) {
		receive_version_number(curr_tuple);
		return;
	}
	
	// check for a reset command
	curr_tuple = dict_find(iterator, (unsigned)RESET_KEY);
	if (curr_tuple) {
		wl_reset();
		return;
	}
	
	// check for a block size request
	curr_tuple = dict_find(iterator, (unsigned)TEXT_BLOCK_SIZE_KEY);
	if (curr_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG,
				"%s: Received a request to change or send the block size.",
				__func__);
		receive_block_size_message(curr_tuple);
		return;
	}
	
	// check for a block number message
	curr_tuple = dict_find(iterator, (unsigned)TOTAL_NUMBER_OF_BLOCKS_KEY);
	if (curr_tuple) {
		unsigned int total_num_blocks = curr_tuple->value->uint32;
		wl_set_total_num_blocks(total_num_blocks);
		APP_LOG(APP_LOG_LEVEL_DEBUG,
				"%s: Set the total number of blocks to %d.",
				__func__,
				total_num_blocks);
		return;
	}
	
	// check for a message with words
	curr_tuple = dict_find(iterator, (unsigned)APPMESG_BLOCK_NUMBER_KEY);
	if (curr_tuple) {
		receive_words(iterator);
		return;
	}
	
	// unable to figure out what's in the message
	curr_tuple = dict_read_first(iterator);
	if (curr_tuple) {
		int32_t key = (int32_t)curr_tuple->key;
		APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Received message with unknown key: %ld", __func__, key);
	}
}

// An error occurred while receiving a new message.
void appmesg_not_received_handler(AppMessageResult reason, void *context)
{
	// log the error
	APP_LOG(APP_LOG_LEVEL_ERROR, "%s: %s", __func__, stringify_AppMessageResult(reason));
}

// Successfully sent a message.
void appmesg_sent_handler(DictionaryIterator *iterator, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: Successfully sent message.", __func__);
}

// An error occurred while sending a message.
void appmesg_not_sent_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
	// log the error
	APP_LOG(APP_LOG_LEVEL_ERROR, "%s: %s", __func__, stringify_AppMessageResult(reason));
}
