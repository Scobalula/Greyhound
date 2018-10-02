#include <Windows.h>

// The sphinx module definition
#include "Sphinx.h"

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	// We don't want thread calls
	DisableThreadLibraryCalls(hModule);

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Create instances
		SphinxInitialize();
		break;
	case DLL_PROCESS_DETACH:
		// Clean up resources
		SphinxShutdown();
		break;
	}

	// Success
	return TRUE;
}