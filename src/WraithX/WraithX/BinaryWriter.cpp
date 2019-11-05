#include "stdafx.h"

// The class we are implementing
#include "BinaryWriter.h"

// We require the patterns utility functions
#include "Patterns.h"

BinaryWriter::BinaryWriter()
{
    // Init the handle
    FileHandle = nullptr;
}

BinaryWriter::~BinaryWriter()
{
    // Close if possible
    Close();
}

bool BinaryWriter::Open(const std::string& FileName, bool ReadWrite)
{
    // Close if we are open
    if (FileHandle != nullptr)
    {
        // Close
        Close();
    }
    // Open the file for writing (append mode, since we have create for new)
    fopen_s(&FileHandle, FileName.c_str(), (ReadWrite) ? "ab+" : "ab");
    // Check
    if (FileHandle)
    {
        // Worked
        return true;
    }
    // Failed
    return false;
}

bool BinaryWriter::Create(const std::string& FileName, bool ReadWrite)
{
    // Close if we are open
    if (FileHandle != nullptr)
    {
        // Close
        Close();
    }
    // Open the file for writing (create or overwrite)
    fopen_s(&FileHandle, FileName.c_str(), (ReadWrite) ? "wb+" : "wb");
    // Check
    if (FileHandle)
    {
        // Worked
        return true;
    }
    // Failed
    return false;
}

bool BinaryWriter::Connect(FILE* FileHandleReference)
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
        // Worked
        return true;
    }
    // Failed
    return false;
}

void BinaryWriter::SetWriteBuffer(uint32_t Size)
{
    // Set a buffer size for this writer
    if (FileHandle != nullptr)
    {
        setvbuf(FileHandle, NULL, _IOFBF, (size_t)Size);
    }
}

bool BinaryWriter::IsOpen()
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

void BinaryWriter::Close()
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

uint64_t BinaryWriter::GetPosition()
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

void BinaryWriter::SetPosition(uint64_t Offset)
{
    // Set the position
    if (FileHandle != nullptr)
    {
        // Set it
        _fseeki64(FileHandle, Offset, SEEK_SET);
    }
}

void BinaryWriter::WriteNullTerminatedString(const std::string& Value)
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // A null char
        auto NullChar = 0;
        // Write the string as a null-terminated string
        fwrite(Value.c_str(), sizeof(char), Value.size(), FileHandle);
        // Write null char
        fwrite(&NullChar, 1, 1, FileHandle);
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

void BinaryWriter::Write(const int8_t* Buffer, uint32_t Size)
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Write it
        fwrite(Buffer, 1, Size, FileHandle);
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

void BinaryWriter::Write(const uint8_t* Buffer, uint32_t Size)
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Write it
        fwrite(Buffer, 1, Size, FileHandle);
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

void BinaryWriter::Write(const std::shared_ptr<int8_t[]>& Buffer, uint32_t Size)
{
    // Make sure we have a file
    if (FileHandle != nullptr)
    {
        // Write it
        fwrite(Buffer.get(), 1, Size, FileHandle);
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