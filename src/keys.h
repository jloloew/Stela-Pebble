#pragma once
#include "JL_DEBUG.h"


/*!
 * Keys for received AppMessages.
 * Negative values are control.
 * Zero and positive values represent the index of the block the attached word belongs in.
 */
typedef enum {
	ERROR_KEY = 0,						///< For reporting an error, as a string.
	RESET_KEY = 1,						///< Reset command. Sent by the phone when a new article will be sent.
	TEXT_BLOCK_SIZE_KEY = 2,			///< Used to set the maximum number of words in each block.
	TOTAL_NUMBER_OF_BLOCKS_KEY = 3,		///< The number of blocks in the entire article.
	APPMESG_NUM_WORDS_KEY = 4,			///< The number of words contained in just the current message.
	APPMESG_WORD_START_INDEX_KEY = 5,	///< Used to send the index of the first word within the block.
	APPMESG_BLOCK_NUMBER_KEY = 6,		///< Holds the index of the block in the current message.
	APPMESG_FIRST_WORD_KEY = 7,			///< Used to send the dictionary key of the first word within this message.
	VERSION_MAJOR_KEY = 8,				///< For sending the version of Stela.
	VERSION_MINOR_KEY = 9,				///< For sending the version of Stela.
	VERSION_PATCH_KEY = 10,				///< For sending the version of Stela.
	APPMESG_FIRST_WORD = 11				///< The value stored for this key is the first word.
										///  Subsequent words should be stored for increasing keys from this key.
										///  This value is always APPMESG_FIRST_WORD_KEY + 1.
} EAppMessage_Keys;

typedef uint8_t		ERROR_KEY_t;
typedef uint8_t		RESET_KEY_t;
typedef int16_t		TEXT_BLOCK_SIZE_KEY_t;
typedef int16_t		TOTAL_NUMBER_OF_BLOCKS_KEY_t;
typedef uint8_t		APPMESG_NUM_WORDS_KEY_t;
typedef uint16_t	APPMESG_WORD_START_INDEX_KEY_t;
typedef int16_t		APPMESG_BLOCK_NUMBER_KEY_t;
typedef uint32_t	APPMESG_FIRST_WORD_KEY_t;
typedef uint8_t		VERSION_MAJOR_KEY_t;
typedef uint8_t		VERSION_MINOR_KEY_t;
typedef uint8_t		VERSION_PATCH_KEY_t;
typedef uint32_t	APPMESG_FIRST_WORD_t;

/// The app's UI state
typedef enum {
	app_ui_state_splash_screen,
	app_ui_state_reading,
	app_ui_state_settings
} app_ui_state_t;

/// A version number of the form x.y.z. Used for AppMessage.
typedef struct {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
} Version;

/// The version of this build of the Stela app for Pebble.
extern const Version PEBBLE_STELA_VERSION;

/*!
 * How many words we have left to read in the current block
 * before we request the next block. Used in the WordList.
 */
extern const int32_t WORDS_REMAINING_FOR_BLOCK_REQUEST; // default 100

	/// The size of the AppMessage inbox, in bytes.
extern const unsigned int APPMESG_INBOX_SIZE;
	/// The size of the AppMessage outbox, in bytes.
extern const unsigned int APPMESG_OUTBOX_SIZE;
