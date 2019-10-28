#pragma once
#include "stdafx.h"
#include "CascLib.h"
#include "CascCommon.h"
class CASCFileReader
{
private:
    // A pointer to the File Handle
    HANDLE FileHandle;
public:
    CASCFileReader(HANDLE Storage, std::string fileName);
    ~CASCFileReader();

    // Gets the length of the file
    uint64_t GetLength() const;
    // Gets the current position of the file
    uint64_t GetPosition() const;

    // Sets the position of the file
    void SetPosition(uint64_t Offset);
    // Advances the position of the file
    void Advance(uint64_t Length);

    template <class T>
    // Read a block of data from the file with the given type
    T Read()
    {
        // Ensure the file is open
        if (FileHandle != nullptr)
        {
            // We must read the data based on type.
            T ResultValue;
            // Zero out the memory
            std::memset(&ResultValue, 0, sizeof(ResultValue));
            // Read the value from the process
            CascReadFile(FileHandle, &ResultValue, sizeof(ResultValue), NULL);
            // Return the result
            return ResultValue;
        }
        // Failed to perform read
#ifdef _DEBUG
        throw new std::exception("No file is open");
#else
        throw new std::exception("");
#endif
    }
    // Read a null-terminated string from the file
    std::string ReadNullTerminatedString();
    // Read a block of data from the file with a given length
    std::unique_ptr<uint8_t[]> Read(uint64_t Length, uint64_t& Result);
    // Read a block of data from the file with a given length to the buffer
    void Read(uint8_t* Buffer, uint64_t Length, uint64_t& Result);
};

