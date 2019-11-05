#pragma once

#include <cstdint>
#include <string>
#include <afxwin.h>

// We require the strings utility
#include "Strings.h"

// A class that handles single instance applications and talking to the main one
class Instance
{
private:

    // The handle to the current instance
    static HANDLE AppInstance;

public:
    // -- Instancing functions

    // Begins a single instance, if successful, it returns true, otherwise, it attempts to bring forth the main window
    static bool BeginSingleInstance(const std::string& MainWindowTitle);
    // Ends a single instance
    static void EndSingleInstance();
};