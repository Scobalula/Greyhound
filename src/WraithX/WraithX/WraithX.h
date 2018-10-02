#pragma once

#include <Windows.h>

// Information on the current WraithX API
#define WRAITHX_BUILD_MAJOR 1
#define WRAITHX_BUILD_MINOR 0
#define WRAITHX_BUILD_EXPONENT 102

// A class that manages WraithX's API
class WraithX
{
private:
	// The GDI+ instance
	static ULONG_PTR GDIPlusToken;

public:
	// Setup everything required for WraithX to function, GUI if specified, load command line args (old entry point)
	static bool InitializeAPI(bool GUI, bool ImageAPI = true);
	// Close down the API
	static void ShutdownAPI(bool GUI, bool ImageAPI = true);
};