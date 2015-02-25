#pragma once
#include "keys.h"
#include <pebble.h>


/// Gets the size of an array.
#ifndef ARRAY_SIZE
	#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#endif


/*!
 * Create a string representation of a Version number.
 * @note The caller must free the returned string.
 * 
 * @param ver The Version struct to stringify.
 * @return The string form of the Version.
 */
char * version_to_string(const Version ver);

/*!
 * Parse a version string into a Version struct.
 * The string should be of the form "x.y.z" where x, y, and z are
 * each decimal integers in the range 0-255.
 * 
 * @param str The string to parse.
 * @return The parsed Version struct, or the Version corresponding to 0.0.0 if an error occurred.
 */
	// The default value for each field is 0.
Version string_to_version(const char * const str) __attribute__((nonnull));

/*!
 * Turns an AppMessageResult returned by an AppMessage function into its description. For debugging purposes.
 * 
 * @param reason The return code to stringify.
 * @return A string describing the error, suitable for debugging.
 */
const char * stringify_AppMessageResult(const AppMessageResult reason);

/*!
 * Turns a DictionaryResult returned by a Dictionary function into its description. For debugging purposes.
 * 
 * @param reason The return code to stringify.
 * @return A string describing the error, suitable for debugging.
 */
const char * stringify_DictResult(const DictionaryResult reason);
