#include "stdafx.h"

// The class we are implementing
#include "MemoryWriter.h"

MemoryWriter::MemoryWriter()
{
	// Defaults
	MemoryWriter(256);
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