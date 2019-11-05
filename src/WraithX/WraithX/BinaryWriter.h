#pragma once

#include <cstdint>
#include <string>
#include <cstdio>
#include <memory>

// A class that handles writing a binary file
class BinaryWriter
{
private:
    // A handle to the file
    FILE* FileHandle;

public:
    BinaryWriter();
    ~BinaryWriter();

    // Open a file for writing
    bool Open(const std::string& FileName, bool ReadWrite = false);
    // Create a new file for writing
    bool Create(const std::string& FileName, bool ReadWrite = false);
    // Connect this writer to an already open handle
    bool Connect(FILE* FileHandleReference);

    // Set write buffer
    void SetWriteBuffer(uint32_t Size);

    // Whether or not the file is still open
    bool IsOpen();

    // Close the file (If we aren't already closed)
    void Close();

    // Gets the current position of the file
    uint64_t GetPosition();

    // Sets the position of the file
    void SetPosition(uint64_t Offset);

    // Write a null-terminated string
    void WriteNullTerminatedString(const std::string& Value);
    
    template<class T>
    // Write a value based on the given type
    void Write(T Value)
    {
        // Make sure we have a file
        if (FileHandle != nullptr)
        {
            // Prepare to write it
            fwrite(&Value, sizeof(T), 1, FileHandle);
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

    template<uint32_t Size>
    // Write a char array with the given size
    void Write(const char(&Value)[Size])
    {
        // Make sure we have a file
        if (FileHandle != nullptr)
        {
            // Prepare to write it
            fwrite(&Value, 1, Size, FileHandle);
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

    template<uint32_t Size>
    // Write a unsigned char array with the given size
    void Write(const uint8_t(&Value)[Size])
    {
        // Make sure we have a file
        if (FileHandle != nullptr)
        {
            // Prepare to write it
            fwrite(&Value, 1, Size, FileHandle);
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

    // Write a buffer to the file
    void Write(const int8_t* Buffer, uint32_t Size);
    // Write a buffer to the file
    void Write(const uint8_t* Buffer, uint32_t Size);
    // Write a safe buffer to the file
    void Write(const std::shared_ptr<int8_t[]>& Buffer, uint32_t Size);
};
