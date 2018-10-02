#pragma once

#include <Windows.h>
#include <cstdint>
#include <tchar.h>

// Initialization results
enum class SirenStatus
{
	// Siren has successfully loaded
	Success,
	// Siren was unable to locate the Dll file
	MissingDll,
	// Siren tried to load a Dll of the wrong arch
	InvalidArch,
	// Siren tried to load a Dll that was outdated or invalid
	OutdatedOrInvalid
};

// Various compression formats and their IDs (This does not corrospond to the Oodle Stream Header)
enum class SirenFormat : uint32_t
{
	LZH = 0,
	LZHLW = 1,
	LZNIB = 2,
	None = 3,
	LZB16 = 4,
	LZBLW = 5,
	LZA = 6,
	LZNA = 7,
	Kraken = 8,
	Mermaid = 9,
	BitKnit = 10,
	Selkie = 11,
	Hydra = 12,
	Leviathan = 13
};

// The level of compression to use
enum class SirenCompressionLevel : uint32_t
{
	None = 0,
	SuperFast = 1,
	VeryFast = 2,
	Fast = 3,
	Normal = 4,
	Optimal1 = 5,
	Optimal2 = 6,
	Optimal3 = 7,
	Optimal4 = 8,
	Optimal5 = 9
};

// A class that wraps the necessary functions required for manipulating Oodle bitstreams...
class Siren
{
public:

	// Initialize Siren library loading the latest version of the Oodle dll ('oo2core_5_win64.dll')
	// DllPath may be set if the library is not located locally to the program (Direct path)
	static SirenStatus Initialize(const TCHAR* DllPath = nullptr);
	// Shuts down the Siren library, cleaning up all allocated resources, and unloading the dll
	static void Shutdown();

	// Compress the input buffer to an Oodle bitstream (With the specified settings)
	static uintptr_t Compress(const uint8_t* Buffer, const uintptr_t BufferSize, uint8_t* OutputBuffer, const uintptr_t OutputBufferSize, SirenFormat Format, SirenCompressionLevel Level);
	// Decompress the Oodle bitstream to the output buffer
	static uintptr_t Decompress(const uint8_t* Buffer, const uintptr_t BufferSize, uint8_t* OutputBuffer, const uintptr_t OutputBufferSize);

	// Calculates the required buffer size for compressed data
	static uintptr_t CompressBound(const uintptr_t BufferSize);

private:

	// Universal function pointers
	static uintptr_t OodleLZ_Compress;
	static uintptr_t OodleLZ_Decompress;

	// The Dll handle if loaded
	static HMODULE OodleHandle;
};