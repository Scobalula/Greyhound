#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <cstdio>

// A class that handles reading binary data from memory
class MemoryReader
{
private:
    // A pointer to the data
    int8_t* DataPointer;
    // The total length of the data
    uint64_t DataLength;
    // The current position of the data
    uint64_t CurrentPosition;
    // Whether or not to keep the buffer alive
    bool KeepAlive;

public:
    __declspec(noinline) MemoryReader();
    __declspec(noinline) MemoryReader(int8_t* Stream, uint64_t StreamLength, bool KeepStreamAlive = false);
    ~MemoryReader();

    // Setup a stream for reading
    void Setup(int8_t* Stream, uint64_t StreamLength, bool KeepStreamAlive = false);

    // Close the stream (If we aren't already closed)
    void Close();

    // Gets the length of the stream
    uint64_t GetLength() const;
    // Gets the current position of the stream
    uint64_t GetPosition() const;

    // Gets the current stream
    int8_t* GetCurrentStream() const;

    // Sets the position of the stream
    void SetPosition(uint64_t Offset);
    // Advances the position of the stream
    void Advance(uint64_t Length);

    template <class T>
    // Read a block of data from the stream with the given type
    T Read()
    {
        // Ensure the stream is valid
        if (DataPointer != nullptr)
        {
            // We must read the data based on type.
            T ResultValue;
            // Get size
            auto ResultSize = sizeof(ResultValue);
            // Zero out the memory
            std::memset(&ResultValue, 0, ResultSize);
            // Read the value from the stream if we are within bounds
            if ((CurrentPosition + ResultSize) <= DataLength)
            {
                // We can read it
                ResultValue = *(reinterpret_cast<T*>(DataPointer + CurrentPosition));
                // Advance
                CurrentPosition += ResultSize;
                // Return the result
                return ResultValue;
            }
        }
        // Failed to perform read
#ifdef _DEBUG
        throw new std::exception("No stream is open");
#else
        throw new std::exception("");
#endif
    }
    template <class T>
    // Read a block of data from the stream with the given type at the offset
    T Read(uint64_t Offset)
    {
        // Only if we're open
        if (DataPointer != nullptr)
        {
            // Go to the offset first
            SetPosition(Offset);
            // Return the result
            return Read<T>();
        }
        // Failed to perform read
#ifdef _DEBUG
        throw new std::exception("No stream is open");
#else
        throw new std::exception("");
#endif
    }
    // Read a block of data from the stream into the buffer
    void Read(uint64_t Length, int8_t* Result);
    // Read a block of data from the stream to the safe buffer
    void Read(uint64_t Length, const std::shared_ptr<int8_t[]>& Result);
    // Read a null-terminated string from the stream
    std::string ReadNullTerminatedString();
    // Read a null-terminated string from the stream with the given offset
    std::string ReadNullTerminatedString(uint64_t Offset);
    // Read a sized string from the stream
    std::string ReadString(uint32_t Size);
    // Read a sized string from the stream, with the given offset
    std::string ReadString(uint32_t Size, uint64_t Offset);
    // Read a varint from the stream
    uint64_t ReadVarInt();
    // Read a varint from the stream, with the given offset
    uint64_t ReadVarInt(uint64_t Offset);
};