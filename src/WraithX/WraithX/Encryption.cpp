#include "stdafx.h"

// The class we are implementing
#include "Encryption.h"

// We need the following classes
#include "BinaryWriter.h"

uint32_t Encryption::Salsa20Block(int8_t* Buffer, uint32_t BufferSize, const Salsa20Key& Key)
{
	// Make sure we have a valid key
	if (Key.IV == nullptr || Key.Key == nullptr || Key.KeyLength == 0) { return 0; }

	// Get key length
	auto KeyLength = (Key.KeyLength == 128) ? S20_KEYLEN_128 : S20_KEYLEN_256;

	// We can now pass the data off to the Salsa20
	auto Result = s20_crypt(Key.Key, KeyLength, Key.IV, 0, (uint8_t*)Buffer, BufferSize);

	// Check if result == S20_SUCCESS, if so, we worked
	if (Result == S20_SUCCESS) { return BufferSize; }

	// Failed
	return 0;
}