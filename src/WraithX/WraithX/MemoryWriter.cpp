#include "stdafx.h"

// The class we are implementing
#include "MemoryWriter.h"

#include "VectorMath.h"

void MemoryWriter::Reallocate(uint64_t Capacity)
{
    auto TempPtr = (int8_t*)std::realloc(DataPointer, Capacity);

    if (TempPtr != nullptr)
    {
        DataPointer = TempPtr;
        DataLength = Capacity;
    }
}

void MemoryWriter::ValidateCapacity(uint64_t End)
{
    if (End > DataLength)
    {
        auto NewCapacity = DataLength * 2;
        Reallocate(VectorMath::Clamp<uint64_t>(NewCapacity, End, NewCapacity));
    }
}

MemoryWriter::MemoryWriter() : MemoryWriter(256)
{
}

MemoryWriter::MemoryWriter(uint32_t Capacity)
{
    // Defaults, but allocate starting buffer
    DataPointer = new int8_t[Capacity];
    DataLength = Capacity;
    // Reset current position
    CurrentPosition = 0;
}

MemoryWriter::~MemoryWriter()
{
    // Clean up if need be
    Close();
}

void MemoryWriter::Close()
{
    // Check if we have data
    if (DataPointer != nullptr)
    {
        // Close
        delete[] DataPointer;
        // Set
        DataPointer = nullptr;
    }
}

uint64_t MemoryWriter::GetLength() const
{
    // Return it
    return DataLength;
}

uint64_t MemoryWriter::GetPosition() const
{
    // Return it
    return CurrentPosition;
}

void MemoryWriter::SetPosition(uint64_t Offset)
{
    ValidateCapacity(Offset);

    CurrentPosition = Offset;
}

int8_t* MemoryWriter::GetCurrentStream() const
{
    // Return it
    return DataPointer;
}

void MemoryWriter::WriteNullTerminatedString(const std::string& Value)
{
    Write((uint8_t*)Value.c_str(), Value.size() + 1);
}

void MemoryWriter::Write(const int8_t* Buffer, uint32_t Size)
{
    ValidateCapacity(CurrentPosition + Size);

    std::memcpy(DataPointer + CurrentPosition, (int8_t*)Buffer, Size);
    CurrentPosition += Size;
}

void MemoryWriter::Write(const uint8_t* Buffer, uint32_t Size)
{
    ValidateCapacity(CurrentPosition + Size);

    std::memcpy(DataPointer + CurrentPosition, (uint8_t*)Buffer, Size);
    CurrentPosition += Size;
}

void MemoryWriter::Write(const std::shared_ptr<int8_t[]>& Buffer, uint32_t Size)
{
    ValidateCapacity(CurrentPosition + Size);

    std::memcpy(DataPointer + CurrentPosition, (uint8_t*)Buffer.get(), Size);
    CurrentPosition += Size;
}
