#pragma once

#include <cstdint>

class Compression
{
public:
	// -- LZ4 Functions

	// Decompress a block of LZ4 compressed data
	static uint32_t DecompressLZ4Block(const int8_t* CompressedBuffer, int8_t* DecompressedBuffer, int32_t CompressedSize, int32_t DecompressedSize);
	// Compress a block of data with LZ4
	static uint32_t CompressLZ4Block(const int8_t* Buffer, int8_t* CompressedBuffer, int32_t BufferSize, int32_t CompressedSize);
	// Returns the recommended buffer size for decompressing LZ4
	static uint32_t DecompressionSizeLZ4();
	// Returns the maximum compressed size for compressing to LZ4
	static uint32_t CompressionSizeLZ4(uint32_t Size);

	// -- LZO1X Functions

	// Decompress a block of LZO1X compressed data
	static uint32_t DecompressLZO1XBlock(const int8_t* CompressedBuffer, int8_t* DecompressedBuffer, int32_t CompressedSize, int32_t DecompressedSize);
	// Compress a block of data with LZO1X
	static uint32_t CompressLZO1XBlock(const int8_t* Buffer, int8_t* CompressedBuffer, int32_t BufferSize, int32_t CompressedSize);
	// Returns the recommended buffer size for decompressing LZO1X
	static uint32_t DecompressionSizeLZO1X();
	// Returns the maximum compressed size for compressing to LZO1X
	static uint32_t CompressionSizeLZO1X(uint32_t Size);

	// -- ZLib Functions

	// Decompress a block of ZLib compressed data
	static uint32_t DecompressZLibBlock(const int8_t* CompressedBuffer, int8_t* DecompressedBuffer, int32_t CompressedSize, int32_t DecompressedSize);
	// Compress a block of data with ZLib
	static uint32_t CompressZLibBlock(const int8_t* Buffer, int8_t* CompressedBuffer, int32_t BufferSize, int32_t CompressedSize);
	// Returns the recommended buffer size for decompressing ZLib
	static uint32_t DecompressionSizeZLib();
	// Returns the maximum compressed size for compressing to ZLib
	static uint32_t CompressionSizeZLib(uint32_t Size);

	// -- Deflate Functions

	// Decompress a block of Deflate compressed data
	static uint32_t DecompressDeflateBlock(const int8_t* CompressedBuffer, int8_t* DecompressedBuffer, int32_t CompressedSize, int32_t DecompressedSize);
	// Compress a block of data with Deflate
	static uint32_t CompressDeflateBlock(const int8_t* Buffer, int8_t* CompressedBuffer, int32_t BufferSize, int32_t CompressedSize);
	// Returns the recommended buffer size for decompressing Deflate
	static uint32_t DecompressionSizeDeflate();
	// Returns the maximum compressed size for compressing to Deflate
	static uint32_t CompressionSizeDeflate(uint32_t Size);
};