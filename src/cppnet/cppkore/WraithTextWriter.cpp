#include "stdafx.h"

// The class we are implementing
#include "WraithTextWriter.h"

// We require the patterns utility functions
#include "Patterns.h"

TextWriter::TextWriter()
{
    // Init the handle
    FileHandle = nullptr;
}

TextWriter::~TextWriter()
{
    // Close if possible
    Close();
}

bool TextWriter::Open(const std::string& FileName)
{
    // Close if we are open
    if (FileHandle != nullptr)
    {
        // Close
        Close();
    }
    // Open the file for writing (append mode, since we have create for new)
    fopen_s(&FileHandle, FileName.c_str(), "a");
    // Check
    if (FileHandle)
    {
        // Worked
        return true;
    }
    // Failed
    return false;
}

bool TextWriter::Create(const std::string& FileName)
{
    // Close if we are open
    if (FileHandle != nullptr)
    {
        // Close
        Close();
    }
    // Open the file for writing (create or overwrite)
    fopen_s(&FileHandle, FileName.c_str(), "w");
    // Check
    if (FileHandle)
    {
        // Worked
        return true;
    }
    // Failed
    return false;
}

bool TextWriter::Connect(FILE* FileHandleReference)
{
    // Close if we are open
    if (FileHandle != nullptr)
    {
        // Close
        Close();
    }
    // Set it
    FileHandle = FileHandleReference;
    // Check and get length
    if (FileHandle)
    {
        // Worked
        return true;
    }
    // Failed
    return false;
}

void TextWriter::SetWriteBuffer(uint32_t Size)
{
    // Set a buffer size for this writer
    if (FileHandle != nullptr)
    {
        setvbuf(FileHandle, NULL, _IOFBF, (size_t)Size);
    }
}

bool TextWriter::IsOpen()
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

void TextWriter::Close()
{
    // Make sure we aren't closed
    if (FileHandle != nullptr)
    {
        // We can close it
        fclose(FileHandle);
        // Set the values
        FileHandle = nullptr;
    }
}

uint64_t TextWriter::GetPosition()
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

void TextWriter::SetPosition(uint64_t Offset)
{
    // Set the position
    if (FileHandle != nullptr)
    {
        // Set it
        _fseeki64(FileHandle, Offset, SEEK_SET);
    }
}

void TextWriter::Write(const std::string& Value)
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Write the string itself to the buffer
        fputs(Value.c_str(), FileHandle);
    }
    else
    {
        // Failed to perform write
#ifdef _DEBUG
        throw new std::exception("No file is open");
#else
        throw new std::exception("");
#endif
    }
}

void TextWriter::WriteFmt(const char* Format, ...)
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Parse
        va_list VarArgs;
        // Start
        va_start(VarArgs, Format);
        // Write it
        vfprintf_s(FileHandle, Format, VarArgs);
        // End
        va_end(VarArgs);
    }
    else
    {
        // Failed to perform write
#ifdef _DEBUG
        throw new std::exception("No file is open");
#else
        throw new std::exception("");
#endif
    }
}

void TextWriter::WriteLine(const std::string& Value)
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Write the string itself to the buffer + new line
        fputs((Value + "\n").c_str(), FileHandle);
    }
    else
    {
        // Failed to perform write
#ifdef _DEBUG
        throw new std::exception("No file is open");
#else
        throw new std::exception("");
#endif
    }
}

void TextWriter::WriteLineFmt(const char* Format, ...)
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Parse
        va_list VarArgs;
        // Start
        va_start(VarArgs, Format);
        // Write it
        vfprintf_s(FileHandle, Format, VarArgs);
        // End
        va_end(VarArgs);
        // Write a new line as well
        fprintf(FileHandle, "\n");
    }
    else
    {
        // Failed to perform write
#ifdef _DEBUG
        throw new std::exception("No file is open");
#else
        throw new std::exception("");
#endif
    }
}

void TextWriter::NewLine()
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Write the new line char
        fprintf(FileHandle, "\n");
    }
    else
    {
        // Failed to perform write
#ifdef _DEBUG
        throw new std::exception("No file is open");
#else
        throw new std::exception("");
#endif
    }
}