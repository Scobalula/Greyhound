#pragma once

#include <cstdint>
#include <string>
#include <cstdio>

// A class that handles writing binary data to memory
class MemoryWriter
{
private:
    // A pointer to the data
    int8_t* DataPointer;
    // The total length of the data (that is allocated)
    uint64_t DataLength;
    // The current position of the data (where we are writing to)
    uint64_t CurrentPosition;
    // Reallocates the stream
    void Reallocate(uint64_t Capacity);
    // Ensures we have the capacity to write, if not, reallocates
    void ValidateCapacity(uint64_t End);
public:
    MemoryWriter();
    MemoryWriter(uint32_t Capacity);
    ~MemoryWriter();

    // Close the stream (If we aren't already closed)
    void Close();

    // Gets the length of the stream
    uint64_t GetLength() const;
    // Gets the current position of the stream
    uint64_t GetPosition() const;
    // Sets the position of the stream
    void SetPosition(uint64_t Offset);

    // Gets the current stream
    int8_t* GetCurrentStream() const;

    // Write a null-terminated string
    void WriteNullTerminatedString(const std::string& Value);

    template<class T>
    // Write a value based on the given type
    void Write(T Value)
    {
        // Prepare to write it
        Write((uint8_t*)&Value, sizeof(T));
    }

    // Write a buffer to the file
    void Write(const int8_t* Buffer, uint32_t Size);
    // Write a buffer to the file
    void Write(const uint8_t* Buffer, uint32_t Size);
    // Write a safe buffer to the file
    void Write(const std::shared_ptr<int8_t[]>& Buffer, uint32_t Size);
};