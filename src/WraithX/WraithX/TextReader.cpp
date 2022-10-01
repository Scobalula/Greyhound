#include "stdafx.h"

// The class we are implementing
#include "TextReader.h"

// We requrie the string utilities
#include "Strings.h"
#include "Systems.h"

TextReader::TextReader()
{
    // Init the handle
    FileHandle = nullptr;
    // Init the length
    FileLength = 0;
}

TextReader::~TextReader()
{
    // Close if possible
    Close();
}

bool TextReader::Open(const std::string& FileName, bool Shared)
{
    // Close if we are open
    if (FileHandle != nullptr)
    {
        // Close
        Close();
    }

    // Open the file for reading
    if (Shared)
        FileHandle = Systems::OpenFileShared(FileName, "rb");
    else
        fopen_s(&FileHandle, FileName.c_str(), "rb");

    // Check
    if (FileHandle)
    {
        // Go to end
        _fseeki64(FileHandle, 0, SEEK_END);
        // Set
        FileLength = (uint64_t)_ftelli64(FileHandle);
        // Go back
        _fseeki64(FileHandle, 0, SEEK_SET);
        // Worked
        return true;
    }
    // Failed
    return false;
}

bool TextReader::Connect(FILE* FileHandleReference)
{
    // Close if we are open
    if (FileHandle != nullptr)
    {
        // Close
        Close();
    }
    // Setit
    FileHandle = FileHandleReference;
    // Check and get length
    if (FileHandle)
    {
        // Grab place
        auto CurrentPosition = _ftelli64(FileHandle);
        // Go to end
        _fseeki64(FileHandle, 0, SEEK_END);
        // Set
        FileLength = _ftelli64(FileHandle);
        // Go back
        _fseeki64(FileHandle, CurrentPosition, SEEK_SET);
        // Worked
        return true;
    }
    // Failed
    return false;
}

bool TextReader::IsOpen()
{
    // Make sure we aren't closed
    if (FileHandle != nullptr)
    {
        // We are open, not possible to be closed
        return true;
    }
    // Closed
    return false;
}

void TextReader::Close()
{
    // Make sure we aren't closed
    if (FileHandle != nullptr)
    {
        // We can close it
        fclose(FileHandle);
        // Set the values
        FileHandle = nullptr;
        FileLength = 0;
    }
}

uint64_t TextReader::GetLength() const
{
    // Return the loaded length
    if (FileHandle != nullptr)
    {
        // Result
        return FileLength;
    }
    // Failed
    return 0;
}

uint64_t TextReader::GetPosition() const
{
    // Return the position
    if (FileHandle != nullptr)
    {
        // Result
        return _ftelli64(FileHandle);
    }
    // Failed
    return 0;
}

void TextReader::SetPosition(uint64_t Offset)
{
    // Set the position
    if (FileHandle != nullptr)
    {
        // Set it
        _fseeki64(FileHandle, Offset, SEEK_SET);
    }
}

std::string TextReader::ReadLine(bool& Success)
{
    Success = true;
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Get the next line from the stream
        char LineBuffer[0x2000];
        // Read
        if(fgets(LineBuffer, 0x2000, FileHandle) == NULL)
            Success = false;
        // Return
        return Strings::EndTrim(std::string(LineBuffer));
    }
    // Failed
#ifdef _DEBUG
    throw new std::exception("No file is open");
#else
    throw new std::exception("");
#endif
}

std::string TextReader::ReadToEnd()
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // A buffer for the reader
        std::stringstream BufferStream;
        // Get the next line from the stream
        char LineBuffer[0x2000];
        // Clean
        std::memset(LineBuffer, 0, 0x2000);
        // Loop
        while (fread(LineBuffer, 1, 0x2000, FileHandle))
        {
            // Set
            BufferStream << LineBuffer;
            // Clear
            std::memset(LineBuffer, 0, 0x2000);
        }
        // Return
        return BufferStream.str();
    }
    // Failed
#ifdef _DEBUG
    throw new std::exception("No file is open");
#else
    throw new std::exception("");
#endif
}

void TextReader::ParseLine(const char* Format, ...)
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Get the next line from the stream
        char LineBuffer[0x2000];
        // Read
        fgets(LineBuffer, 0x2000, FileHandle);
        // Parse
        va_list VarArgs;
        // Start
        va_start(VarArgs, Format);
        // Prepare to set them
        vsscanf(LineBuffer, Format, VarArgs);
        // End
        va_end(VarArgs);
    }
    else
    {
        // Failed
#ifdef _DEBUG
        throw new std::exception("No file is open");
#else
        throw new std::exception("");
#endif
    }
}