#include "stdafx.h"

// The class we are implementing
#include "MemoryReader.h"

// We require the patterns utility functions
#include "Patterns.h"

MemoryReader::MemoryReader()
{
	// Defaults
	DataPointer = nullptr;
	DataLength = 0;
	CurrentPosition = 0;
	KeepAlive = false;
}

MemoryReader::MemoryReader(int8_t* Stream, uint64_t StreamLength, bool KeepStreamAlive)
{
	// Defaults
	DataPointer = Stream;
	DataLength = StreamLength;
	CurrentPosition = 0;
	KeepAlive = KeepStreamAlive;
}

MemoryReader::~MemoryReader()
{
	// Clean up if need be
	Close();
}

void MemoryReader::Setup(int8_t* Stream, uint64_t StreamLength, bool KeepStreamAlive)
{
	// Check if we need to close
	if (DataPointer != nullptr)
	{
		// Close
		Close();
	}
	// Set
	DataPointer = Stream;
	DataLength = StreamLength;
	CurrentPosition = 0;
	KeepAlive = KeepStreamAlive;
}

void MemoryReader::Close()
{
	// Check the pointer
	if (DataPointer != nullptr)
	{
		// Check to close, we close by default
		if (!KeepAlive)
		{
			// Close
			delete[] DataPointer;
		}
		// Set
		DataPointer = nullptr;
	}
}

uint64_t MemoryReader::GetLength() const
{
	// Return it
	return DataLength;
}

uint64_t MemoryReader::GetPosition() const
{
	// Return it
	return CurrentPosition;
}

int8_t* MemoryReader::GetCurrentStream() const
{
	// Return it
	return DataPointer;
}

void MemoryReader::SetPosition(uint64_t Offset)
{
	// Only set it if we are within bounds
	if (Offset <= DataLength)
	{
		// Set
		CurrentPosition = Offset;
	}
}

void MemoryReader::Advance(uint64_t Length)
{
	// Only set it if we are within bounds
	if ((Length + CurrentPosition) <= DataLength)
	{
		// Set
		CurrentPosition += Length;
	}
}

void MemoryReader::Read(uint64_t Length, int8_t* Result)
{
	// Ensure the stream is valid
	if (DataPointer != nullptr && Result != nullptr)
	{
		// Read the value from the stream if we are within bounds
		if ((CurrentPosition + Length) <= DataLength)
		{
			// Copy
			std::memcpy(Result, (DataPointer + CurrentPosition), (size_t)Length);
			// Advance
			CurrentPosition += Length;
		}
	}
}

void MemoryReader::Read(uint64_t Length, const std::shared_ptr<int8_t[]>& Result)
{
	// Ensure the stream is valid
	if (DataPointer != nullptr && Result != nullptr)
	{
		// Read the value from the stream if we are within bounds
		if ((CurrentPosition + Length) <= DataLength)
		{
			// Copy
			std::memcpy(Result.get(), (DataPointer + CurrentPosition), (size_t)Length);
			// Advance
			CurrentPosition += Length;
		}
	}
}

std::string MemoryReader::ReadNullTerminatedString()
{
	// Make sure we are loaded
	if (DataPointer != nullptr)
	{
		// We can read
		std::stringstream ResultBuffer;
		// First char
		char CurrentChar = Read<char>();
		// Loop until null or EOF
		while (CurrentChar != 0)
		{
			// Add
			ResultBuffer << CurrentChar;
			// Read again
			CurrentChar = Read<char>();
		}
		// Return it
		return ResultBuffer.str();
	}
	// Failed to perform read
	return "";
}

std::string MemoryReader::ReadNullTerminatedString(uint64_t Offset)
{
	// Make sure we are loaded
	if (DataPointer != nullptr)
	{
		// Jump to offset
		SetPosition(Offset);
		// Read
		ReadNullTerminatedString();
	}
	// Failed to perform read
	return "";
}

std::string MemoryReader::ReadString(uint32_t Size)
{
	// Make sure we are loaded
	if (DataPointer != nullptr)
	{
		// Ensure that we have enough data to read
		if ((CurrentPosition + Size) <= DataLength)
		{
			// Read it
			auto ResultStr = std::string((DataPointer + CurrentPosition), (DataPointer + CurrentPosition + Size));
			// Advance
			CurrentPosition += Size;
			// Return it
			return ResultStr;
		}
	}
	// Failed to perform read
	return "";
}

std::string MemoryReader::ReadString(uint32_t Size, uint64_t Offset)
{
	// Make sure we are loaded
	if (DataPointer != nullptr)
	{
		// Set position
		SetPosition(Offset);
		// Read
		return ReadString(Size);
	}
	// Failed to perform read
	return "";
}

uint64_t MemoryReader::ReadVarInt()
{
	// Result buffer
	uint64_t Result = 0;
	// Shift
	uint64_t Shift = 0;

	// Loop
	while (true)
	{
		// Read byte
		auto ByteVal = Read<uint8_t>();
		// Shift result
		Result |= ((ByteVal & 0x7F) << Shift);
		// Check for ending
		if ((ByteVal & 0x80) == 0) { break; }
		// Shift it
		Shift += 7;
	}

	// Return result
	return Result;
}

uint64_t MemoryReader::ReadVarInt(uint64_t Offset)
{
	// Make sure we are loaded
	if (DataPointer != nullptr)
	{
		// Set position
		SetPosition(Offset);
		// Read
		return ReadVarInt();
	}
	// Failed to perform read
	return 0;
}