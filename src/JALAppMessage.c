#include "JALAppMessage.h"
#include "JALWordList.h"

const char * stringify_AppMessageResult(const AppMessageResult reason)
{
	switch (reason) {
		case APP_MSG_OK:
			return "All good, operation was successful.";
		case APP_MSG_SEND_TIMEOUT:
			return "The other end did not confirm receiving the sent data with an (n)ack in time.";
		case APP_MSG_SEND_REJECTED:
			return "The other end rejected the sent data, with a \"nack\" reply.";
		case APP_MSG_NOT_CONNECTED:
			return "The other end was not connected.";
		case APP_MSG_APP_NOT_RUNNING:
			return "The local application was not running.";
		case APP_MSG_INVALID_ARGS:
			return "The function was called with invalid arguments.";
		case APP_MSG_BUSY:
			return "There are pending (in or outbound) messages that need to be processed first before new ones can be received or sent.";
		case APP_MSG_BUFFER_OVERFLOW:
			return "The buffer was too small to contain the incoming message.";
		case APP_MSG_ALREADY_RELEASED:
			return "The resource had already been released.";
		case APP_MSG_CALLBACK_ALREADY_REGISTERED:
			return "The callback node was already registered, or its ListNode has not been initialized.";
		case APP_MSG_CALLBACK_NOT_REGISTERED:
			return "The callback could not be deregistered, because it had not been registered before.";
		case APP_MSG_OUT_OF_MEMORY:
			return "The support library did not have sufficient application memory to perform the requested operation.";
		case APP_MSG_CLOSED:
			return "App message was closed.";
		case APP_MSG_INTERNAL_ERROR:
			return "An internal OS error prevented APP_MSG from completing an operation.";
		default:
			return "";
	}
}


void app_message_received_handler(DictionaryIterator *iterator, void *context)
{
	static char memory_full = 0;
	if (memory_full) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Memory full, unable to add new word.");
	}
	
	Tuple *wordPair = dict_read_first(iterator);
	if (!wordPair) { // safety check
		return;
	}
	do {
		// check the value's type
		if (wordPair->type != TUPLE_CSTRING) {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Received non-string word.");
		} else {
			// it's a string
			uint16_t dataLength = wordPair->length;
			if (dataLength == 0) {
				return; // no work to do
			}
			// copy the string out of the dictionary and add it to the linked list
//			char *receivedWord = calloc(dataLength, sizeof(char));
//			strncpy(receivedWord, (const char *)wordPair->value, dataLength);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Received '%s'", (char *)wordPair->value);
			if (!wl_add_word((const char *)wordPair->value)) {
				memory_full = 1;
			}
		}
	} while ((wordPair = dict_read_next(iterator)));
}


// An error occurred while receiving a new message.
void app_message_not_received_handler(AppMessageResult reason, void *context)
{
	// log the reason
	APP_LOG(APP_LOG_LEVEL_ERROR, "%s %s", __func__, stringify_AppMessageResult(reason));
}
