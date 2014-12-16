#pragma once
#include <pebble.h>

void app_message_received_handler(DictionaryIterator *iterator, void *context);

void app_message_not_received_handler(AppMessageResult reason, void *context);

const char * stringify_AppMessageResult(const AppMessageResult reason);
