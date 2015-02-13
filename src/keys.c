#include "keys.h"
#include <pebble.h>


const unsigned int WORDS_REMAINING_FOR_BLOCK_REQUEST = 50;

const Version PEBBLE_STELA_VERSION = { .major = 1, .minor = 0, .patch = 1 };

const unsigned int APPMESG_INBOX_SIZE  = 128;
const unsigned int APPMESG_OUTBOX_SIZE = 128;
