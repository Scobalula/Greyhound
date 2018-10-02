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

	// TODO: Implement features (Including .cpp file)
	// TODO: Unit tests
};