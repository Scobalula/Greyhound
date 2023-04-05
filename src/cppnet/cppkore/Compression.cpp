#include "stdafx.h"

// The class we are implementing
#include "Compression.h"

// We need to include external libraries for compression
#include "..\cppkore_incl\LZ4_XXHash\lz4.h"
#include "..\cppkore_incl\LZ4_XXHash\lz4hc.h"

#include "..\cppkore_incl\LZO1X\minilzo.h"
#include "..\cppkore_incl\ZLib\miniz.h"

uint32_t WraithCompression::DecompressLZ4Block(const int8_t* CompressedBuffer, int8_t* DecompressedBuffer, int32_t CompressedSize, int32_t DecompressedSize)
{
    // Compressed buffer is already allocated with (hopefully) LZ4 data.
    if (DecompressedSize == 0) { DecompressedSize = DecompressionSizeLZ4(); }

    // Next, pass off the data to the LZ4 decompress safe function
    auto Result = LZ4_decompress_safe((const char*)CompressedBuffer, (char*)DecompressedBuffer, CompressedSize, DecompressedSize);

    // If we have a result > 0, we were successful
    if (Result > 0) { return (uint32_t)Result; }

    // We failed to decompress
    return 0;
}

uint32_t WraithCompression::CompressLZ4Block(const int8_t* Buffer, int8_t* CompressedBuffer, int32_t BufferSize, int32_t CompressedSize)
{
    // Buffer is already allocated, check that we have a size, if not, set to compressed size
    if (CompressedSize == 0) { CompressedSize = CompressionSizeLZ4(BufferSize); }

    // Next, pass off the data to the LZ4
    auto Result = LZ4_compress_default((const char*)Buffer, (char*)CompressedBuffer, BufferSize, CompressedSize);

    // If we have a result > 0, we did it
    if (Result > 0) { return (uint32_t)Result; }

    // We failed to compress
    return 0;
}

uint32_t WraithCompression::DecompressionSizeLZ4()
{
    // Return value
    return 0x900000;
}

uint32_t WraithCompression::CompressionSizeLZ4(uint32_t Size)
{
    // Return value
    return LZ4_compressBound((int32_t)Size);
}

uint32_t WraithCompression::DecompressLZO1XBlock(const int8_t* CompressedBuffer, int8_t* DecompressedBuffer, int32_t CompressedSize, int32_t DecompressedSize)
{
    // Compressed buffer is already allocated with (hopefully) LZO1X data.
    if (DecompressedSize == 0) { DecompressedSize = DecompressionSizeLZO1X(); }

    // Resulting size
    lzo_uint ResultSize = DecompressedSize;

    // Next, pass off the data to the LZO1X decompress function
    auto Result = lzo1x_decompress_safe((const uint8_t*)CompressedBuffer, CompressedSize, (uint8_t*)DecompressedBuffer, &ResultSize, NULL);

    // If we have a result == OK, we were successful
    if (Result == LZO_E_OK) { return (uint32_t)ResultSize; }

    // We failed to decompress
    return 0;
}

uint32_t WraithCompression::CompressLZO1XBlock(const int8_t* Buffer, int8_t* CompressedBuffer, int32_t BufferSize, int32_t CompressedSize)
{
    // Buffer is already allocated, check that we have a size, if not, set to compressed size
    if (CompressedSize == 0) { CompressedSize = CompressionSizeLZO1X(BufferSize); }

    // Resulting size
    lzo_uint ResultSize = 0;

    // Working memory
    auto WorkingMem = malloc(LZO1X_1_MEM_COMPRESS);

    // Next, pass off the data to the LZO1X
    auto Result = lzo1x_1_compress((const uint8_t*)Buffer, BufferSize, (uint8_t*)CompressedBuffer, &ResultSize, WorkingMem);

    // Free working memory
    if (WorkingMem != NULL) { free(WorkingMem); }

    // If we have a result == OK, we did it
    if (Result == LZO_E_OK) { return (uint32_t)ResultSize; }

    // We failed to compress
    return 0;
}

uint32_t WraithCompression::DecompressionSizeLZO1X()
{
    // Return value
    return 0x900000;
}

uint32_t WraithCompression::CompressionSizeLZO1X(uint32_t Size)
{
    // Return value
    return (Size + Size / 16 + 64 + 3);
}

uint32_t WraithCompression::DecompressZLibBlock(const int8_t* CompressedBuffer, int8_t* DecompressedBuffer, int32_t CompressedSize, int32_t DecompressedSize)
{
    // Compressed buffer is already allocated with (hopefully) ZLib data.
    if (DecompressedSize == 0) { DecompressedSize = DecompressionSizeZLib(); }

    // Resulting size
    mz_ulong ResultSize = DecompressedSize;

    // Next, pass off the data to the ZLib decompressor
    auto Result = uncompress((uint8_t*)DecompressedBuffer, &ResultSize, (const uint8_t*)CompressedBuffer, CompressedSize);

    // If we have result == Z_OK we are done
    if (Result == Z_OK) { return (uint32_t)ResultSize; }

    // We failed to decompress
    return 0;
}

uint32_t WraithCompression::CompressZLibBlock(const int8_t* Buffer, int8_t* CompressedBuffer, int32_t BufferSize, int32_t CompressedSize)
{
    // Buffer is already allocated, check that we have a size, if not, set to compressed size
    if (CompressedSize == 0) { CompressedSize = CompressionSizeZLib(BufferSize); }

    // Resulting size
    mz_ulong ResultSize = CompressedSize;

    // Next, pass off the data to the ZLib
    auto Result = compress((uint8_t*)CompressedBuffer, &ResultSize, (const uint8_t*)Buffer, BufferSize);

    // If we have result == Z_OK we are done
    if (Result == Z_OK) { return (uint32_t)ResultSize; }

    // We failed to compress
    return 0;
}

uint32_t WraithCompression::DecompressionSizeZLib()
{
    // Return value
    return 0x900000;
}

uint32_t WraithCompression::CompressionSizeZLib(uint32_t Size)
{
    // Return value
    return (uint32_t)compressBound(Size);
}

uint32_t WraithCompression::DecompressDeflateBlock(const int8_t* CompressedBuffer, int8_t* DecompressedBuffer, int32_t CompressedSize, int32_t DecompressedSize)
{
    // Compressed buffer is already allocated with (hopefully) Deflate data.
    if (DecompressedSize == 0) { DecompressedSize = DecompressionSizeDeflate(); }

    // Allocate and prepare stream
    z_stream InflateStream;
    // Set it
    std::memset(&InflateStream, 0, sizeof(InflateStream));

    // Initialize
    if (inflateInit2(&InflateStream, -MZ_DEFAULT_WINDOW_BITS))
    {
        // Failed
        return 0;
    }

    // Set
    InflateStream.avail_in = CompressedSize;
    InflateStream.avail_out = DecompressedSize;
    InflateStream.next_in = (const uint8_t*)CompressedBuffer;
    InflateStream.next_out = (uint8_t*)DecompressedBuffer;

    // Inflate it
    auto Result = inflate(&InflateStream, Z_SYNC_FLUSH);

    // Shutdown
    inflateEnd(&InflateStream);

    // Check result
    if (Result == Z_OK || (InflateStream.total_out == DecompressedSize))
    {
        // Set result
        return (uint32_t)InflateStream.total_out;
    }

    // We failed to decompress
    return 0;
}

uint32_t WraithCompression::CompressDeflateBlock(const int8_t* Buffer, int8_t* CompressedBuffer, int32_t BufferSize, int32_t CompressedSize)
{
    // Buffer is already allocated, check that we have a size, if not, set to compressed size
    if (CompressedSize == 0) { CompressedSize = CompressionSizeDeflate(BufferSize); }

    // Allocate and prepare stream
    z_stream DeflateStream;
    // Set it
    std::memset(&DeflateStream, 0, sizeof(DeflateStream));

    // Initialize
    if (deflateInit2(&DeflateStream, MZ_DEFAULT_LEVEL, MZ_DEFLATED, -MZ_DEFAULT_WINDOW_BITS, 9, MZ_DEFAULT_STRATEGY))
    {
        // Failed
        return 0;
    }

    // Set
    DeflateStream.avail_in = BufferSize;
    DeflateStream.avail_out = CompressedSize;
    DeflateStream.next_in = (const uint8_t*)Buffer;
    DeflateStream.next_out = (uint8_t*)CompressedBuffer;

    // Deflate it
    auto Result = deflate(&DeflateStream, Z_SYNC_FLUSH);

    // Shutdown
    inflateEnd(&DeflateStream);

    // Check result
    if (Result == Z_OK)
    {
        // Set result
        return (uint32_t)DeflateStream.total_out;
    }

    // We failed to compress
    return 0;
}

uint32_t WraithCompression::DecompressionSizeDeflate()
{
    // Return value
    return 0x900000;
}

uint32_t WraithCompression::CompressionSizeDeflate(uint32_t Size)
{
    // Return value
    return (uint32_t)compressBound(Size);
}