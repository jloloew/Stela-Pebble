#pragma once


/*!
 * Keys for received AppMessages.
 * Negative values are control.
 * Zero and positive values represent the index of the block the attached word belongs in.
 */
typedef enum {
	ERROR_KEY = 0,					///< For reporting an error, as a string.
	STELA_VERSION_KEY,				///< For sending the version of Stela as a string.
	RESET_KEY,						///< Reset command. Sent by the phone when a new article will be sent.
	TEXT_BLOCK_SIZE_KEY,			///< Used to set the maximum number of words in each block.
	TOTAL_NUMBER_OF_BLOCKS_KEY,		///< The number of blocks in the entire article.
	APPMESG_NUM_WORDS_KEY,			///< The number of words contained in just the current message.
	APPMESG_WORD_START_INDEX_KEY,	///< Used to send the index of the first word within the block.
	APPMESG_BLOCK_NUMBER_KEY,		///< Holds the index of the block in the current message.
	APPMESG_FIRST_WORD_KEY,			///< Used to send the dictionary key of the first word within this message.
	APPMESG_FIRST_WORD				///< The value stored for this key is the first word.
									///  Subsequent words should be stored for increasing keys from this key.
									///  This value is always APPMESG_FIRST_WORD_KEY + 1.
} EAppMessage_Keys;

/// The app's UI state
typedef enum {
	app_ui_state_splash_screen,
	app_ui_state_reading,
	app_ui_state_settings
} app_ui_state_t;

/// A version number of the form x.y.z. Used for AppMessage.
typedef struct {
	unsigned char major;
	unsigned char minor;
	unsigned char patch;
} Version;

/// The version of this build of the Stela app for Pebble.
extern const Version PEBBLE_STELA_VERSION;

/*!
 * How many words we have left to read in the current block
 * before we request the next block. Used in the WordList.
 */
extern const unsigned int WORDS_REMAINING_FOR_BLOCK_REQUEST; // default 100

	/// The size of the AppMessage inbox, in bytes.
extern const unsigned int APPMESG_INBOX_SIZE;
	/// The size of the AppMessage outbox, in bytes.
extern const unsigned int APPMESG_OUTBOX_SIZE;
