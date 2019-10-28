#include "stdafx.h"
#include "siren.h"

// Oodle function definitions

// Oodle compression routine definition, verified for Oodle v5 (x32) (All callbacks are unmarked and aren't needed)
typedef int32_t(*OodleLZ32_CompressDef)(int32_t Format, unsigned char *Buffer, int32_t BufferSize, unsigned char *OutputBuffer, int32_t Level, void *a, void *b, void *c);
// Oodle compression routine definition, verified for Oodle v5 (x64) (All callbacks are unmarked and aren't needed)
typedef int64_t(*OodleLZ64_CompressDef)(int32_t Format, unsigned char *Buffer, int64_t BufferSize, unsigned char *OutputBuffer, int64_t Level, void *a, void *b, void *c);

// Oodle decompression routine definition, verified for Oodle v5 (x32) (All callbacks are unmarked and aren't needed)
typedef int32_t(*OodleLZ32_DecompressDef)(unsigned char *Buffer, int32_t BufferSize, unsigned char *OutputBuffer, int32_t OutputBufferSize, int32_t a, int32_t b, int32_t c, void *d, void *e, void *f, void *g, void *h, void *i, int32_t ThreadModule);
// Oodle decompression routine definition, verified for Oodle v5 (x64) (All callbacks are unmarked and aren't needed)
typedef int64_t(*OodleLZ64_DecompressDef)(unsigned char *Buffer, int64_t BufferSize, unsigned char *OutputBuffer, int64_t OutputBufferSize, int32_t a, int32_t b, int64_t c, void *d, void *e, void *f, void *g, void *h, void *i, int32_t ThreadModule);

// Default properties
HMODULE Siren::OodleHandle = NULL;

// Default functions
uintptr_t Siren::OodleLZ_Compress = NULL;
uintptr_t Siren::OodleLZ_Decompress = NULL;

SirenStatus Siren::Initialize(const TCHAR* DllPath)
{
    // Don't load if already loaded
    if (OodleHandle != NULL && OodleLZ_Compress != NULL && OodleLZ_Decompress != NULL)
        return SirenStatus::Success;

    // Prepare to load the oodle dll, also, perform verification
    OodleHandle = NULL;

    // Check if a path was provided
    if (DllPath == nullptr)
    {
        // Load based on arch
        if (sizeof(uintptr_t) == 4)
        {
            OodleHandle = LoadLibrary(L"oo2core_5_win32.dll");
        }
        else if (sizeof(uintptr_t) == 8)
        {
            OodleHandle = LoadLibrary(L"oo2core_5_win64.dll");
        }
    }
    else
    {
        // Attempt to load from this path
        OodleHandle = LoadLibrary(DllPath);
    }

    // Check if we were successful
    if (OodleHandle == NULL)
    {
        // Convert to our status
        switch (GetLastError())
        {
        case ERROR_MOD_NOT_FOUND:
            return SirenStatus::MissingDll;
        case ERROR_BAD_FORMAT:
            return SirenStatus::InvalidArch;
        }
    }

    // Load success, attempt to find the functions
    if (sizeof(uintptr_t) == 4)
    {
        // Load 32bit functions
        OodleLZ_Compress = (uintptr_t)GetProcAddress(OodleHandle, "_OodleLZ_Compress@32");
        OodleLZ_Decompress = (uintptr_t)GetProcAddress(OodleHandle, "_OodleLZ_Decompress@56");

        // Verify
        if (OodleLZ_Compress == NULL || OodleLZ_Decompress == NULL) { return SirenStatus::OutdatedOrInvalid; }
    }
    else if (sizeof(uintptr_t) == 8)
    {
        // Load 64bit functions
        OodleLZ_Compress = (uintptr_t)GetProcAddress(OodleHandle, "OodleLZ_Compress");
        OodleLZ_Decompress = (uintptr_t)GetProcAddress(OodleHandle, "OodleLZ_Decompress");

        // Verify
        if (OodleLZ_Compress == NULL || OodleLZ_Decompress == NULL) { return SirenStatus::OutdatedOrInvalid; }
    }

    // If we made it, success
    return SirenStatus::Success;
}

void Siren::Shutdown()
{
    // Shutdown if loaded
    if (OodleHandle != NULL)
    {
        // Unload and reset
        FreeLibrary(OodleHandle);
        OodleHandle = NULL;
    }

    // Reset
    OodleLZ_Compress = NULL;
    OodleLZ_Decompress = NULL;
}

uintptr_t Siren::Decompress(const uint8_t* Buffer, const uintptr_t BufferSize, uint8_t* OutputBuffer, const uintptr_t OutputBufferSize)
{
    // Safety checks
    if (OodleLZ_Decompress != NULL && Buffer != nullptr && BufferSize > 0 && OutputBuffer != nullptr && OutputBufferSize > 0)
    {
        // Result size
        intptr_t Result = 0;

        // Compile time arch check
        if (sizeof(uintptr_t) == 4)
        {
            Result = (intptr_t)((OodleLZ32_DecompressDef)OodleLZ_Decompress)((uint8_t*)Buffer, (int32_t)BufferSize, OutputBuffer, (int32_t)OutputBufferSize, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 3);
        }
        else if (sizeof(uintptr_t) == 8)
        {
            Result = (intptr_t)((OodleLZ64_DecompressDef)OodleLZ_Decompress)((uint8_t*)Buffer, (int64_t)BufferSize, OutputBuffer, (int64_t)OutputBufferSize, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 3);
        }

        // Determine success
        if (Result > 0) { return Result; }
    }

    // Failed
    return 0;
}

uintptr_t Siren::Compress(const uint8_t* Buffer, const uintptr_t BufferSize, uint8_t* OutputBuffer, const uintptr_t OutputBufferSize, SirenFormat Format, SirenCompressionLevel Level)
{
    // Safety checks
    if (OodleLZ_Compress != NULL && Buffer != nullptr && BufferSize > 0 && OutputBuffer != nullptr && OutputBufferSize > 0)
    {
        // Result size
        intptr_t Result = 0;

        // Compile time arch check
        if (sizeof(uintptr_t) == 4)
        {
            Result = (intptr_t)((OodleLZ32_CompressDef)OodleLZ_Compress)((int32_t)Format, (uint8_t*)Buffer, (int32_t)BufferSize, OutputBuffer, (int32_t)Level, NULL, NULL, NULL);
        }
        else if (sizeof(uintptr_t) == 8)
        {
            Result = (intptr_t)((OodleLZ64_CompressDef)OodleLZ_Compress)((int32_t)Format, (uint8_t*)Buffer, (int64_t)BufferSize, OutputBuffer, (int64_t)Level, NULL, NULL, NULL);
        }

        // Determine success
        if (Result > 0) { return Result; }
    }

    // Failed
    return 0;
}

uintptr_t Siren::CompressBound(const uintptr_t BufferSize)
{
    // Calculate it
    return (uintptr_t)(BufferSize + 274 * ((BufferSize + 0x3FFFF) / 0x40000));
}