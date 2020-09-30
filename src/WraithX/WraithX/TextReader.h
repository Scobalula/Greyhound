#pragma once

#include <cstdint>
#include <string>
#include <cstdio>

// A class that handles reading and scanning a text file for data
class TextReader
{
private:
    // A handle to the file
    FILE* FileHandle;
    // The length of the file
    uint64_t FileLength;

public:
    TextReader();
    ~TextReader();

    // Open a file for reading
    bool Open(const std::string& FileName, bool Shared = false);
    // Connect this reader to an already open handle
    bool Connect(FILE* FileHandleReference);

    // Whether or not the file is still open
    bool IsOpen();

    // Close the file (If we aren't already closed)
    void Close();

    // Gets the length of the file (in bytes)
    uint64_t GetLength() const;
    // Gets the current position of the file (in bytes)
    uint64_t GetPosition() const;

    // Sets the position of the file (in bytes)
    void SetPosition(uint64_t Offset);

    // Read the next line from the file
    std::string ReadLine(bool& Success);
    // Reads the entire file to the end
    std::string ReadToEnd();
    // Parse the next line from the file
    void ParseLine(const char* Format, ...);
};