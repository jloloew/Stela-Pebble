#pragma once
//#include <pebble.h>


	/// Initialize the AppMessage subsystems.
void appmesg_init();

void appmesg_deinit();

	/// Send a request to the phone for the block with the given index.
	/// 
	/// @param index The index of the desired block.
void appmesg_request_block(const unsigned int index);

	/// Tell the phone to switch to a new block size.
	/// 
	/// @param block_size The desired block size to request.
void appmesg_send_block_size(const int block_size);

void appmesg_send_error(const char *const error_msg) __attribute__((nonnull));
