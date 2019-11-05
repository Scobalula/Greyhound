#pragma once

#include <cstdint>
#include <string>
#include <cstdio>

// A class that handles writing a text file
class TextWriter
{
private:
    // A handle to the file
    FILE* FileHandle;

public:
    TextWriter();
    ~TextWriter();

    // Open a file for writing
    bool Open(const std::string& FileName);
    // Create a new file for writing
    bool Create(const std::string& FileName);
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

    // Write a string to the file
    void Write(const std::string& Value);
    // Write a formatted string to the file
    void WriteFmt(const char* Format, ...);
    // Write a string to the file and end the line
    void WriteLine(const std::string& Value);
    // Write a formatted string to the file and end the line
    void WriteLineFmt(const char* Format, ...);

    // Starts a new line in the file
    void NewLine();
};
