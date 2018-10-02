#include "stdafx.h"

// The class we are implementing
#include "WraithX.h"
#include "WraithTheme.h"

// We need the following classes
#include "CommandLineManager.h"
#include "Strings.h"
#include "Image.h"

// The token
ULONG_PTR WraithX::GDIPlusToken = NULL;

bool WraithX::InitializeAPI(bool GUI, bool ImageAPI)
{
	// We need to setup the API, and GUI if need be, also, load settings

	// Cache command line string
	auto CommandLine = ::GetCommandLine();

	// Initialize COM
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) { return false; }

	// Initialize DirectXTex WIC
	if (ImageAPI)
		Image::InitializeImageAPI();

	// Initialize AFX if we need to
	if (GUI)
	{
		// We must setup AFX
		if (!AfxWinInit(::GetModuleHandle(NULL), NULL, CommandLine, 0)) { return false; }
		if (!AfxOleInit()){ return false; }
		if (!AfxInitRichEdit2()) { return false; }

		// We must setup GDI+
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		// Start it up
		Gdiplus::GdiplusStartup(&GDIPlusToken, &gdiplusStartupInput, NULL);

		// Setup the theme
		WraithTheme::InitializeDefaultTheme();
	}

	// Parse command line arguments
	LPWSTR* CommandArgsList;
	// The count of arguments
	int32_t CommandArgsCount;

	// Apply it
	CommandArgsList = CommandLineToArgvW(CommandLine, &CommandArgsCount);

	// If successful, parse and clean up
	if (CommandArgsList != NULL)
	{
		// A list of arguments
		std::vector<std::string> Arguments;

		// Loop and add
		for (int32_t i = 0; i < CommandArgsCount; i++)
		{
			// Add
			Arguments.emplace_back(Strings::ToNormalString(CommandArgsList[i]));
		}

		// Clean up
		LocalFree(CommandArgsList);

		// Pass the arguments over to the argument parser
		CommandLineManager::ParseCommandLine(Arguments);
	}

	// We succeeded
	return true;
}

void WraithX::ShutdownAPI(bool GUI, bool ImageAPI)
{
	// We need to shutdown the API, close down gui if need be
	if (ImageAPI)
		Image::ShutdownImageAPI();
	
	// Shutdown COM
	CoUninitialize();

	// Close down AFX
	if (GUI)
	{
		// Shutdown AFX
		AfxWinTerm();

		// Shutdown GDI+
		Gdiplus::GdiplusShutdown(GDIPlusToken);

		// Free theme resources
		WraithTheme::UnloadThemeResources();
	}
}