/**
 * @file JL_DEBUG.h
 * 
 * This custom logging framework is a little rough around the edges, but it's
 * a cinch to disable logging application-wide.
 * 
 * @author Justin Loew
 * @date 3/17/2015
 */

#pragma once
#include <pebble.h>


#ifndef JL_DEBUG_DEFINED
#define JL_DEBUG_DEFINED

	/// Enable or disable this whole system
#define JL_DEBUG_ENABLED 1

	// What to add to the logging messages
#define JL_ADD_FILE_TO_LOGS	0
#define JL_ADD_FUNC_TO_LOGS	1
#define JL_ADD_LINE_TO_LOGS	1

	// Select log levels to enable
#define JL_ENABLE_LOG_LEVEL_ALL				1
#define JL_ENABLE_LOG_LEVEL_ERROR			(1 && JL_ENABLE_LOG_LEVEL_ALL)
#define JL_ENABLE_LOG_LEVEL_WARNING			(1 && JL_ENABLE_LOG_LEVEL_ERROR)
#define JL_ENABLE_LOG_LEVEL_INFO			(1 && JL_ENABLE_LOG_LEVEL_WARNING)
#define JL_ENABLE_LOG_LEVEL_DEBUG			(1 && JL_ENABLE_LOG_LEVEL_INFO)
#define JL_ENABLE_LOG_LEVEL_VERBOSE			(0 && JL_ENABLE_LOG_LEVEL_DEBUG)


	/// Execute the next line or block only in the debug configuration.
#define JL_IF_DEBUG if (JL_DEBUG_ENABLED)


	/// Output an actual log statement.
#define JL_FINALIZE_LOG(level, fmt, ...)					\
	do {													\
		JL_IF_DEBUG											\
			APP_LOG(level, fmt , ##__VA_ARGS__);			\
	} while (0)												\


	// Prepend the file name.
#if JL_ADD_FILE_TO_LOGS
	#define _JL_ADD_FILE(level, fmt, ...) JL_FINALIZE_LOG(level, "%s:" fmt, __FILE__ , ##__VA_ARGS__)
#else
	#define _JL_ADD_FILE(level, fmt, ...) JL_FINALIZE_LOG(level, fmt , ##__VA_ARGS__)
#endif
#define JL_ADD_FILE(level, fmt, ...) _JL_ADD_FILE(level, fmt , ##__VA_ARGS__)

	// Prepend the function name.
#if JL_ADD_FUNC_TO_LOGS
	#define _JL_ADD_FUNC(level, fmt, ...) JL_ADD_FILE(level, "%s:" fmt, __func__ , ##__VA_ARGS__)
#else
	#define _JL_ADD_FUNC(level, fmt, ...) JL_ADD_FILE(level, fmt , ##__VA_ARGS__)
#endif
#define JL_ADD_FUNC(level, fmt, ...) _JL_ADD_FUNC(level, fmt , ##__VA_ARGS__)

	// Prepend the line number.
#if JL_ADD_LINE_TO_LOGS
	#define _JL_ADD_LINE(level, fmt, ...) JL_ADD_FUNC(level, "%d:" fmt, __LINE__ , ##__VA_ARGS__)
#else
	#define _JL_ADD_LINE(level, fmt, ...) JL_ADD_FUNC(level, fmt , ##__VA_ARGS__)
#endif
#define JL_ADD_LINE(level, fmt, ...) _JL_ADD_LINE(level, fmt , ##__VA_ARGS__)

	// Add a space to the beginning of the format string if we're going to be adding any extra information
#if JL_ADD_FILE_TO_LOGS || JL_ADD_FUNC_TO_LOGS || JL_ADD_LINE_TO_LOGS
	#define _JL_AUGMENT_FORMAT_STRING(level, fmt, ...) JL_ADD_LINE(level, " " fmt , ##__VA_ARGS__)
#else
	#define _JL_AUGMENT_FORMAT_STRING(level, fmt, ...) JL_ADD_LINE(level, fmt , ##__VA_ARGS__)
#endif
#define JL_AUGMENT_FORMAT_STRING(level, fmt, ...) _JL_AUGMENT_FORMAT_STRING(level, fmt , ##__VA_ARGS__)

/**************** Actual Logging Macros ****************/

	// Log errors.
#if JL_ENABLE_LOG_LEVEL_ERROR
	#define _JL_ERROR(fmt, ...) JL_AUGMENT_FORMAT_STRING(APP_LOG_LEVEL_ERROR, fmt , ##__VA_ARGS__)
#else
	#define _JL_ERROR(fmt, ...)
#endif
#define JL_ERROR(fmt, ...) _JL_ERROR(fmt , ##__VA_ARGS__)

	// Log warnings.
#if JL_ENABLE_LOG_LEVEL_WARNING
	#define _JL_WARNING(fmt, ...) JL_AUGMENT_FORMAT_STRING(APP_LOG_LEVEL_WARNING, fmt , ##__VA_ARGS__)
#else
	#define _JL_WARNING(fmt, ...)
#endif
#define JL_WARNING(fmt, ...) _JL_WARNING(fmt , ##__VA_ARGS__)

	// Log information.
#if JL_ENABLE_LOG_LEVEL_INFO
	#define _JL_INFO(fmt, ...) JL_AUGMENT_FORMAT_STRING(APP_LOG_LEVEL_INFO, fmt , ##__VA_ARGS__)
#else
	#define _JL_INFO(fmt, ...)
#endif
#define JL_INFO(fmt, ...) _JL_INFO(fmt , ##__VA_ARGS__)

	// Log debug messages.
#if JL_ENABLE_LOG_LEVEL_DEBUG
	#define _JL_DEBUG(fmt, ...) JL_AUGMENT_FORMAT_STRING(APP_LOG_LEVEL_DEBUG, fmt , ##__VA_ARGS__)
#else
	#define _JL_DEBUG(fmt, ...)
#endif
#define JL_DEBUG(fmt, ...) _JL_DEBUG(fmt , ##__VA_ARGS__)

	// Log verbose debug messages.
#if JL_ENABLE_LOG_LEVEL_VERBOSE
	#define _JL_VERBOSE(fmt, ...) JL_AUGMENT_FORMAT_STRING(APP_LOG_LEVEL_DEBUG_VERBOSE, fmt , ##__VA_ARGS__)
#else
	#define _JL_VERBOSE(fmt, ...)
#endif
#define JL_VERBOSE(fmt, ...) _JL_VERBOSE(fmt , ##__VA_ARGS__)


#endif // JL_DEBUG_DEFINED
