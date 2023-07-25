#include "stdafx.h"

// The class we are implementing
#include "WraithFileDialogs.h"

// We need the following classes
#include "Strings.h"
#include "FileSystems.h"

std::string WraithFileDialogs::MakeFilterBuffer(const std::string& Value)
{
    // The working value
    std::string WorkingValue = Value;
    // Check
    if (Strings::IsNullOrWhiteSpace(WorkingValue))
    {
        // Default
        WorkingValue = " |*.*";
    }
    // Allocate a buffer
    auto Buffer = std::make_unique<int8_t[]>(WorkingValue.length() + 2);
    // Copy it
    std::memcpy(Buffer.get(), WorkingValue.c_str(), WorkingValue.length());
    // Loop
    for (int32_t i = 0; i < (int32_t)WorkingValue.length(); i++)
    {
        // Check
        if (Buffer.get()[i] == '|')
        {
            // Set
            Buffer.get()[i] = (int8_t)0;
        }
    }
    // Set end
    Buffer.get()[WorkingValue.length() + 1] = (int8_t)0;
    // Return
    return std::string((char*)Buffer.get(), (char*)(Buffer.get() + WorkingValue.length() + 1));
}

std::string WraithFileDialogs::OpenFileDialog(const std::string& Title, const std::string& BasePath, const std::string& Filter, HWND Owner)
{
    // A buffer
    char Buffer[MAX_PATH];
    // Clean
    ZeroMemory(&Buffer, sizeof(Buffer));

    // Convert
    auto ResultFilter = MakeFilterBuffer(Filter);

    // Setup struct
    OPENFILENAMEA OpenFileBuffer;
    // Clear
    ZeroMemory(&OpenFileBuffer, sizeof(OpenFileBuffer));
    // Set properties
    OpenFileBuffer.lStructSize = sizeof(OpenFileBuffer);
    OpenFileBuffer.hwndOwner = Owner;
    OpenFileBuffer.lpstrFilter = ResultFilter.c_str();
    OpenFileBuffer.lpstrFile = Buffer;
    OpenFileBuffer.nMaxFile = MAX_PATH;
    OpenFileBuffer.lpstrTitle = Title.c_str();
    OpenFileBuffer.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;

    // Fetch it
    if (GetOpenFileNameA(&OpenFileBuffer))
    {
        // We got it, add it
        return std::string(Buffer);
    }

    // Return it
    return "";
}

std::vector<std::string> WraithFileDialogs::OpenMultiFileDialog(const std::string& Title, const std::string& BasePath, const std::string& Filter, HWND Owner)
{
    // The result
    std::vector<std::string> Result;

    // A buffer for the files
    char Buffer[0x2000];
    // Clean
    ZeroMemory(&Buffer, sizeof(Buffer));

    // Convert
    auto ResultFilter = MakeFilterBuffer(Filter);

    // Setup struct
    OPENFILENAMEA OpenFileBuffer;
    // Clear
    ZeroMemory(&OpenFileBuffer, sizeof(OpenFileBuffer));
    // Set properties
    OpenFileBuffer.lStructSize = sizeof(OpenFileBuffer);
    OpenFileBuffer.hwndOwner = Owner;
    OpenFileBuffer.lpstrFilter = ResultFilter.c_str();
    OpenFileBuffer.lpstrFile = Buffer;
    OpenFileBuffer.nMaxFile = sizeof(Buffer);
    OpenFileBuffer.lpstrTitle = Title.c_str();
    OpenFileBuffer.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_HIDEREADONLY;

    // Fetch it
    if (GetOpenFileNameA(&OpenFileBuffer))
    {
        // We got it, prepare to parse result
        char* PathBuffer = OpenFileBuffer.lpstrFile;
        // Get the directory
        std::string Directory = std::string(PathBuffer, (size_t)OpenFileBuffer.nFileOffset - 1);
        // Advance
        PathBuffer += OpenFileBuffer.nFileOffset;
        // Loop
        while (*PathBuffer)
        {
            // Load file name
            std::string FileName = std::string(PathBuffer);

            // Combine and add
            Result.push_back(FileSystems::CombinePath(Directory, FileName));

            // Advance
            PathBuffer += (FileName.length() + 1);
        }
    }

    // Return it
    return Result;
}

std::string WraithFileDialogs::SaveFileDialog(const std::string& Title, const std::string& BasePath, const std::string& Filter, HWND Owner)
{
    // A buffer
    char Buffer[MAX_PATH];
    // Clean
    ZeroMemory(&Buffer, sizeof(Buffer));

    // Convert
    auto ResultFilter = MakeFilterBuffer(Filter);

    // Setup struct
    OPENFILENAMEA OpenFileBuffer;
    // Clear
    ZeroMemory(&OpenFileBuffer, sizeof(OpenFileBuffer));
    // Set properties
    OpenFileBuffer.lStructSize = sizeof(OpenFileBuffer);
    OpenFileBuffer.hwndOwner = Owner;
    OpenFileBuffer.lpstrFilter = ResultFilter.c_str();
    OpenFileBuffer.lpstrFile = Buffer;
    OpenFileBuffer.nMaxFile = MAX_PATH;
    OpenFileBuffer.lpstrTitle = Title.c_str();

    // Fetch it
    if (GetSaveFileNameA(&OpenFileBuffer))
    {
        // We got it, add it
        return std::string(Buffer);
    }

    // Return it
    return "";
}