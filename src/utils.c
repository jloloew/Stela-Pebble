#include "utils.h"
#include <pebble.h>


	// Turns an AppMessageResult returned by an AppMessage function into its description. For debugging purposes.
const char * stringify_AppMessageResult(const AppMessageResult reason);
	// Turns a DictionaryResult returned by a Dictionary function into its description. For debugging purposes.
const char * stringify_DictResult(const DictionaryResult reason);


	// The caller must free the string returned.
char * version_to_string(const Version ver)
{
	char *ver_str = malloc(sizeof("255.255.255")); // longest possible version number
	if (!ver_str) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Out of memory", __func__);
		return NULL;
	}
	snprintf(ver_str, sizeof(ver_str), "%d.%d.%d", ver.major, ver.minor, ver.patch);
	return ver_str;
}

Version string_to_version(const char *const str)
{
	Version ver = { .major = 0, .minor = 0, .patch = 0 };
	if (str) {
		sscanf(str, "%hhu.%hhu.%hhu", &ver.major, &ver.minor, &ver.patch);
	}
	return ver;
}

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
		return NULL;
	}
}

const char * stringify_DictResult(const DictionaryResult reason)
{
	switch (reason) {
	case DICT_OK:
		return "The operation returned successfully.";
	case DICT_NOT_ENOUGH_STORAGE:
		return "There was not enough backing storage to complete the operation.";
	case DICT_INVALID_ARGS:
		return "One or more arguments were invalid or uninitialized.";
	case DICT_INTERNAL_INCONSISTENCY:
		return "The lengths and/or count of the dictionary its tuples are inconsistent.";
	case DICT_MALLOC_FAILED:
		return "A requested operation required additional memory to be allocated, but the allocation failed, likely due to insufficient remaining heap memory.";
	default:
		return NULL;
	}
}
