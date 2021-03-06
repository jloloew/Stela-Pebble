//#include <pebble.h>
#include "utils.h"
#include "JALAppMessage.h"
#include "JALWordList.h"


static Version phone_version = { .major = 0, .minor = 0, .patch = 0 };

// Receiving Messages
static void receive_block_size_message(Tuple *tuple) __attribute__((nonnull));
static void receive_words(DictionaryIterator *iterator) __attribute__((nonnull));
static void receive_error(Tuple *tuple) __attribute__((nonnull));
static void receive_version_number(Tuple *major, Tuple *minor, Tuple *patch) __attribute__((nonnull));
// Sending Messages
static void send_version_number(void);
// Helpers for sending messages.
	/*!
	 * Open the AppMessage outbox to prepare to send a message.
	 * 
	 * @return The DictionaryIterator to write the outgoing message into. 
	 */
static DictionaryIterator * _appmesg_send_helper1(void);
	/*!
	 * Send the message in the outbox.
	 * 
	 * @param iterator The DictionaryIterator containing the outgoing message.
	 * @param dict_result The result of adding the key-value pair to the iterator. Used for error handling.
	 */
static void _appmesg_send_helper2(DictionaryIterator *iterator,
								  const DictionaryResult dict_result) __attribute__((nonnull));
	/*!
	 * Create and send a key-value pair via AppMessage.
	 * 
	 * @param key The key to use for the value in the AppMessage.
	 * @param value The int value to send.
	 */
static void _appmesg_send_int(const int key,
							  const int32_t value);
	/*!
	 * Create and send a key-value pair via AppMessage.
	 * 
	 * @param key The key to use for the value in the AppMessage.
	 * @param value The unsigned int value to send.
	 */
static void _appmesg_send_uint(const int key,
							   const uint32_t value);
	/*!
	 * Create and send a key-value pair via AppMessage.
	 * 
	 * @param key The key to use for the value in the AppMessage.
	 * @param value The C string to send.
	 */
static void _appmesg_send_cstring(const int key,
								  const char *const value) __attribute__((nonnull));

// Callbacks
	/// callback for when a new message comes in from the phone
void appmesg_received_handler(DictionaryIterator *iterator,
							  void *context __attribute__((unused)));
	/// callback for when an error occurs while trying to receive a new message
void appmesg_not_received_handler(AppMessageResult reason,
								  void *context __attribute__((unused)));
	/// Successfully sent a message.
void appmesg_sent_handler(DictionaryIterator *iterator,
						  void *context __attribute__((unused)));
	/// An error occurred while sending a message.
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
	/*
	// calculate AppMessage inbox/outbox buffer sizes
	// Formula is from https://developer.getpebble.com/docs/c/group___dictionary.html#ga0551d069624fb5bfc066fecfa4153bde
	uint32_t inbox_size = 0, outbox_size = 0;
	unsigned int avg_word_size = 10;
	unsigned int avg_num_words = 10;
	inbox_size = 1 + // dictionary header
		// 1 Tuple per control message
		(APPMESG_FIRST_WORD * 7) + // 7 bytes for each Tuple's header
		(APPMESG_FIRST_WORD * 4) + // 4 bytes for the ints in each Tuple
		// 1 Tuple per word
		(avg_num_words * 7) + // 7 bytes for each Tuple's header
		(avg_num_words * (avg_word_size + 1)); // Add the size of each word
	outbox_size = 200;
	JL_DEBUG("Setting AppMessage inbox/outbox sizes to %d/%d", inbox_size, outbox_size);
	// open AppMessage communication
	AppMessageResult open_result = app_message_open(inbox_size, outbox_size);
	*/
	// open AppMessage communication
	AppMessageResult open_result = app_message_open(APPMESG_INBOX_SIZE, APPMESG_OUTBOX_SIZE);
	if (open_result == APP_MSG_OK) {
		JL_DEBUG("AppMessage opened successfully.");
	} else {
		JL_ERROR("AppMessage failed to open.");
		//TODO: update UI to say "not connected" or something
	}
}

void appmesg_deinit(void)
{
	app_message_deregister_callbacks();
	phone_version = (Version){ .major = 0, .minor = 0, .patch = 0 };
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

/********** CALLBACKS **********/

	// Route the received iterator to the correct function for processing.
void appmesg_received_handler(DictionaryIterator *iterator, void *context)
{
//	JL_DEBUG("Used/free memory: %d/%d", heap_bytes_used(), heap_bytes_free());
	
	// safety first
	if (!iterator) {
		JL_ERROR("Received NULL dictionary.");
		return;
	}
	
	// log it
	JL_DEBUG("Received dict of size %u", (unsigned int)dict_size(iterator));
	JL_IF_DEBUG {
		log_dictionary(iterator);
	}
	
	// check for an error
	Tuple *curr_tuple = dict_find(iterator, ERROR_KEY);
	if (curr_tuple) {
		receive_error(curr_tuple);
		return;
	}
	
	// check for a version number
	curr_tuple = dict_find(iterator, VERSION_MAJOR_KEY);
	if (curr_tuple) {
		Tuple *major = curr_tuple;
		Tuple *minor = dict_find(iterator, VERSION_MINOR_KEY);
		Tuple *patch = dict_find(iterator, VERSION_PATCH_KEY);
		receive_version_number(major, minor, patch);
		return;
	}
	
	// check for a reset command
	curr_tuple = dict_find(iterator, RESET_KEY);
	if (curr_tuple) {
		wl_reset();
		return;
	}
	
	// check for a block size request
	curr_tuple = dict_find(iterator, TEXT_BLOCK_SIZE_KEY);
	if (curr_tuple) {
		JL_DEBUG("Received a request to change or send the block size.");
		receive_block_size_message(curr_tuple);
	}
	
	// check for a message with the total number of blocks
	curr_tuple = dict_find(iterator, TOTAL_NUMBER_OF_BLOCKS_KEY);
	if (curr_tuple) {
		unsigned int total_num_blocks = curr_tuple->value->uint32;
		wl_set_total_num_blocks(total_num_blocks);
		JL_DEBUG("Set the total number of blocks to %d.", total_num_blocks);
	}
	
	// check for a message with words
	curr_tuple = dict_find(iterator, APPMESG_BLOCK_NUMBER_KEY);
	if (curr_tuple) {
		receive_words(iterator);
		return;
	}
	
	// unable to figure out what's in the message
	if (!curr_tuple) {
		curr_tuple = dict_read_first(iterator);
		uint32_t key = curr_tuple->key;
		JL_ERROR("Received message with unknown key: %lu", key);
	}
}

// An error occurred while receiving a new message.
void appmesg_not_received_handler(AppMessageResult reason, void *context)
{
	// log the error
	JL_ERROR("%s", stringify_AppMessageResult(reason));
}

// Successfully sent a message.
void appmesg_sent_handler(DictionaryIterator *iterator, void *context)
{
	JL_DEBUG("Successfully sent message.");
}

// An error occurred while sending a message.
void appmesg_not_sent_handler(DictionaryIterator *iterator,
							  AppMessageResult reason,
							  void *context)
{
	// log the error
	JL_ERROR("%s", stringify_AppMessageResult(reason));
}

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
		JL_ERROR("Requested block size is not a signed int value.");
		return;
	}
	if (tuple->length != 4) {
		JL_ERROR("Requested block size is not a 32-bit value.");
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
		JL_DEBUG("The phone's block size is %d.", -value);
	}
	
	// Send our block size to the phone, whether or not we changed ours.
	appmesg_send_block_size(wl_get_block_size());
}

static void receive_words(DictionaryIterator *iterator)
{
	int block_number = -1;
	unsigned int start_index = 0, num_words = 0, first_word_key = 0;
	Tuple *curr_tuple;
	
	// get the block number
	curr_tuple = dict_find(iterator, APPMESG_BLOCK_NUMBER_KEY);
	if (curr_tuple) {
		block_number = curr_tuple->value->int32;
	}
	// get the starting index of the word within the block
	curr_tuple = dict_find(iterator, APPMESG_WORD_START_INDEX_KEY);
	if (curr_tuple) {
		start_index = curr_tuple->value->uint32;
	}
	// get the number of words
	curr_tuple = dict_find(iterator, APPMESG_NUM_WORDS_KEY);
	if (curr_tuple) {
		num_words = curr_tuple->value->uint32;
	}
	// get the key for the first actual word in this message
	curr_tuple = dict_find(iterator, APPMESG_FIRST_WORD_KEY);
	if (curr_tuple) {
		first_word_key = curr_tuple->value->uint32;
	}
	
	// get each word, in order
	unsigned int word_num = 0;
	while (word_num < num_words) {
		curr_tuple = dict_find(iterator, first_word_key + word_num);
		
		// copy the string out of the Tuple
		uint16_t word_length = curr_tuple->length;
		if (word_length == 0) {
			word_num++;
			continue;
		}
		char *copied_word = malloc(word_length * sizeof(char));
		if (!copied_word) {
			JL_ERROR("Out of memory.");
		}
		strncpy(copied_word, curr_tuple->value->cstring, word_length);
		
		// add the word to the block
		wl_add_word(copied_word, block_number, start_index + word_num);
		
		word_num++;
	}
	
	JL_DEBUG("Added %d words.", word_num);
}

static void receive_error(Tuple *tuple)
{
	if (tuple->type == TUPLE_CSTRING) {
		const char *error_message = (const char *)&tuple->value->cstring;
		JL_ERROR("%s", error_message);
	} else {
		JL_ERROR("Received non-string error.");
	}
}

static void receive_version_number(Tuple *major, Tuple *minor, Tuple *patch)
{
	VERSION_MAJOR_KEY_t v_major = major->value->uint8;
	VERSION_MINOR_KEY_t v_minor = minor->value->uint8;
	VERSION_PATCH_KEY_t v_patch = patch->value->uint8;
	
	Version const ver = { .major = v_major, .minor = v_minor, .patch = v_patch };
	
	// If the major is 0, it's a request for our version number.
	// Else, the phone is sending its version number.
	if (ver.major == 0) {
		send_version_number();
	} else {
		phone_version = ver;
		JL_DEBUG("Phone has version %s", version_to_string(ver));
	}
}

/********** SEND HELPERS **********/

static void send_version_number(void)
{
	DictionaryIterator *iterator = _appmesg_send_helper1();
	// add the data to the outgoing message
	DictionaryResult dict_result;
	dict_result = dict_write_uint8(iterator, VERSION_MAJOR_KEY, PEBBLE_STELA_VERSION.major);
	if (dict_result == DICT_OK) {
		dict_result = dict_write_uint8(iterator, VERSION_MINOR_KEY, PEBBLE_STELA_VERSION.minor);
	}
	if (dict_result == DICT_OK) {
		dict_result = dict_write_uint8(iterator, VERSION_PATCH_KEY, PEBBLE_STELA_VERSION.patch);
	}
	_appmesg_send_helper2(iterator, dict_result);
}

static DictionaryIterator * _appmesg_send_helper1(void)
{
	DictionaryIterator *iterator = NULL;
	// open the outbox to prepare to send a message
	AppMessageResult appmesg_result = app_message_outbox_begin(&iterator);
	if (appmesg_result != APP_MSG_OK) {
		// failure
		JL_ERROR("Unable to open outbox while preparing: %s", stringify_AppMessageResult(appmesg_result));
	}
	return iterator;
}

static void _appmesg_send_helper2(DictionaryIterator *iterator,
								  const DictionaryResult dict_result)
{
	if (dict_result != DICT_OK) { // error handling
		JL_ERROR("Dict error: %s", stringify_DictResult(dict_result));
		free(iterator); // not sure if this is necessary
		return;
	}
	
	// send the message
	AppMessageResult appmesg_result = app_message_outbox_send();
	if (appmesg_result != APP_MSG_OK) {
		JL_ERROR("Error sending message: %s", stringify_AppMessageResult(appmesg_result));
	} else {
		JL_DEBUG("Successfully sent message.");
	}
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
