#include "pch.h"
#include "Kore.h"
#include "WraithMain.h"
#include "ExportManager.h"
#include "UIXTheme.h"
#include "GreyhoundTheme.h"
#include "CommandLine.h"
#include <CoDAssets.h>


#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace System;

// Cleanup old patch files before launch...
static void CleanupFilesystem()
{
    // Attempt to clean up the existing old files and folders
    auto CurrentPath = System::Environment::GetApplicationPath();

    // Attempt to delete the file
    if(IO::File::Exists(IO::Path::Combine(CurrentPath, "exhalelib.dll")))
        IO::File::Delete(IO::Path::Combine(CurrentPath, "exhalelib.dll"));
    if (IO::File::Exists(IO::Path::Combine(CurrentPath, "latest_log.txt")))
        IO::File::Delete(IO::Path::Combine(CurrentPath, "latest_log.txt"));
    if (IO::File::Exists(IO::Path::Combine(CurrentPath, "settings.dat")))
        IO::File::Delete(IO::Path::Combine(CurrentPath, "settings.dat"));
    if (IO::File::Exists(IO::Path::Combine(CurrentPath, "Wraith.exe")))
        IO::File::Delete(IO::Path::Combine(CurrentPath, "Wraith.exe"));

    // Attempt to delete old cache
    if(IO::Directory::Exists(IO::Path::Combine(CurrentPath, "data")))
        IO::Directory::Delete(IO::Path::Combine(CurrentPath, "data"));
    // Attempt to delete old update cache
    if (IO::Directory::Exists(IO::Path::Combine(CurrentPath, "Temp")))
        IO::Directory::Delete(IO::Path::Combine(CurrentPath, "Temp"));
}

LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo)
{
    MessageBoxA(NULL, "Greyhound has encountered a fatal error and must close.\n\nA dump file will be written to where Greyhound+'s exe is located, please provide this and as much information as you can when reporting this crash.", "Greyhound+", MB_OK | MB_ICONERROR);

    HANDLE hFile = CreateFile(
        L"crash_dump.dmp",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    MINIDUMP_EXCEPTION_INFORMATION mei{};
    mei.ThreadId = GetCurrentThreadId();
    mei.ClientPointers = TRUE;
    mei.ExceptionPointers = ExceptionInfo;
    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpNormal,
        &mei,
        NULL,
        NULL);

    return EXCEPTION_EXECUTE_HANDLER;
}

#if _DEBUG
int main(int argc, char** argv)
#else
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    // Assign filter before anything
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

    Forms::Application::EnableVisualStyles();
    UIX::UIXTheme::InitializeRenderer(new Themes::GreyhoundTheme());
    ExportManager::InitializeExporter();




#if _DEBUG
    SetConsoleTitleA("Greyhound | Console");
#else
    int argc = __argc;
    char** argv = __argv;
#endif

    // Handle CLI 
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "verifiedhashes") == 0)
        {
            CoDAssets::VerifiedHashes = true;
        }
    }

    // Clean up files
    CleanupFilesystem();
    WraithMain* main = new WraithMain();
    Forms::Application::Run(main);
    UIX::UIXTheme::ShutdownRenderer();
    // Tell the asset cache to clean up (Prevents crash on async cache loading)
    CoDAssets::CleanUpGame();

    return 0;
}