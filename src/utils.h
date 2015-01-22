#pragma once
//#include <pebble.h>
#include "keys.h"


	// The caller must free the string returned.
char * version_to_string(const Version ver);
	// The default value for each field is 0.
Version string_to_version(const char *const str) __attribute__((nonnull));
