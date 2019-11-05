#include "stdafx.h"

// The class we are implementing
#include "Instance.h"

// We need the hashing class
#include "Hashing.h"

// Set it up
HANDLE Instance::AppInstance = nullptr;

bool Instance::BeginSingleInstance(const std::string& MainWindowTitle)
{
    // The window hash
    auto WindowHash = Strings::Format("%X", Hashing::HashXXHashString(MainWindowTitle));

    // Attempt to open an *existing* mutex
    HANDLE AppInstance = OpenMutexA(MUTEX_ALL_ACCESS, 0, WindowHash.c_str());
    // Check it
    if (!AppInstance && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        // We can make it
        AppInstance = CreateMutexA(NULL, FALSE, WindowHash.c_str());
        // Done
        return true;
    }
    else
    {
        // We need to at least attempt to show the main window
        auto WindowHandle = FindWindowA(NULL, MainWindowTitle.c_str());
        // Ensure we got one
        if (WindowHandle != NULL)
        {
            // Lets make it top most
            SetForegroundWindow(WindowHandle);
        }
    }

    // Failed
    return false;
}

void Instance::EndSingleInstance()
{
    // Shutdown the instance
    if (AppInstance != nullptr)
    {
        // Close
        ReleaseMutex(AppInstance);
        // Reset
        AppInstance = nullptr;
    }
}