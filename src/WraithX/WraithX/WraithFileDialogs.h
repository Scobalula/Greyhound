#pragma once

#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

#include <afxwin.h>
#include <string>
#include <vector>

// A class that handles dialog popups
class WraithFileDialogs
{
private:
    // Dialog helpers

    // Converts a usable filter to a dialog ready buffer
    static std::string MakeFilterBuffer(const std::string& Value);

public:
    // -- Dialog utility functions

    // Shows a file dialog so the user can open a file
    static std::string OpenFileDialog(const std::string& Title, const std::string& BasePath = "", const std::string& Filter = "All Files (*.*)|*.*", HWND Owner = NULL);
    // Shows a file dialog so the user can open multiple files
    static std::vector<std::string> OpenMultiFileDialog(const std::string& Title, const std::string& BasePath = "", const std::string& Filter = "All Files (*.*)|*.*", HWND Owner = NULL);

    // Shows a file dialog so the user can save a file
    static std::string SaveFileDialog(const std::string& Title, const std::string& BasePath = "", const std::string& Filter = "All Files (*.*)|*.*", HWND Owner = NULL);
};