#include "stdafx.h"

// The class we are implementing
#include "BinaryReader.h"

// We require the patterns utility functions
#include "Patterns.h"
#include "Systems.h"

BinaryReader::BinaryReader()
{
	// Init the handle
	FileHandle = nullptr;
	// Init the length
	FileLength = 0;
}

BinaryReader::~BinaryReader()
{
	// Close if possible
	Close();
}

bool BinaryReader::Open(const std::string& FileName, bool Shared)
{
	// Close if we are open
	if (FileHandle != nullptr)
	{
		// Close
		Close();
	}

	// Open the file for reading
	if (Shared)
		FileHandle = Systems::OpenFileShared(FileName, "rb");
	else
		fopen_s(&FileHandle, FileName.c_str(), "rb");

	// Check
	if (FileHandle)
	{
		// Go to end
		_fseeki64(FileHandle, 0, SEEK_END);
		// Set
		FileLength = (uint64_t)_ftelli64(FileHandle);
		// Go back
		_fseeki64(FileHandle, 0, SEEK_SET);
		// Worked
		return true;
	}
	// Failed
	return false;
}

bool BinaryReader::Connect(FILE* FileHandleReference)
{
	// Close if we are open
	if (FileHandle != nullptr)
	{
		// Close
		Close();
	}
	// Setit
	FileHandle = FileHandleReference;
	// Check and get length
	if (FileHandle)
	{
		// Grab place
		auto CurrentPosition = _ftelli64(FileHandle);
		// Go to end
		_fseeki64(FileHandle, 0, SEEK_END);
		// Set
		FileLength = _ftelli64(FileHandle);
		// Go back
		_fseeki64(FileHandle, CurrentPosition, SEEK_SET);
		// Worked
		return true;
	}
	// Failed
	return false;
}

bool BinaryReader::IsOpen()
{
	// Make sure we aren't closed
	if (FileHandle != nullptr)
	{
		// We are open, not possible to be closed
		return true;
	}
	// Closed
	return false;
}

void BinaryReader::Close()
{
	// Make sure we aren't closed
	if (FileHandle != nullptr)
	{
		// We can close it
		fclose(FileHandle);
		// Set the values
		FileHandle = nullptr;
		FileLength = 0;
	}
}

uint64_t BinaryReader::GetLength() const
{
	// Return the loaded length
	if (FileHandle != nullptr)
	{
		// Result
		return FileLength;
	}
	// Failed
	return 0;
}

uint64_t BinaryReader::GetPosition() const
{
	// Return the position
	if (FileHandle != nullptr)
	{
		// Result
		return _ftelli64(FileHandle);
	}
	// Failed
	return 0;
}

void BinaryReader::SetPosition(uint64_t Offset)
{
	// Set the position
	if (FileHandle != nullptr)
	{
		// Set it
		_fseeki64(FileHandle, Offset, SEEK_SET);
	}
}

void BinaryReader::Advance(uint64_t Length)
{
	// Set the position
	if (FileHandle != nullptr)
	{
		// Set it
		_fseeki64(FileHandle, Length, SEEK_CUR);
	}
}

std::string BinaryReader::ReadNullTerminatedString()
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
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
#ifdef _DEBUG
	throw new std::exception("No file is open");
#else
	throw new std::exception("");
#endif
}

std::string BinaryReader::ReadNullTerminatedString(uint64_t Offset)
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
	{
		// Jump to offset first
		_fseeki64(FileHandle, Offset, SEEK_SET);
		// Read
		return ReadNullTerminatedString();
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("No file is open");
#else
	throw new std::exception("");
#endif
}

std::string BinaryReader::ReadString(uint32_t Size)
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
	{
		// We can read
		std::string ResultStr = "";
		// Result
		uint64_t Result = 0;
		// Read the value
		int8_t* ResultBuffer = Read(Size, Result);
		// Check
		if (Result > 0)
		{
			// Set
			ResultStr = std::string((const char*)ResultBuffer, ((const char*)ResultBuffer + Size));
		}
		// Clean up
		delete[] ResultBuffer;
		// Return it
		return ResultStr;
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("No file is open");
#else
	throw new std::exception("");
#endif
}

std::string BinaryReader::ReadString(uint32_t Size, uint64_t Offset)
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
	{
		// Jump to offset first
		_fseeki64(FileHandle, Offset, SEEK_SET);
		// Read
		return ReadString(Size);
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("No file is open");
#else
	throw new std::exception("");
#endif
}

int8_t* BinaryReader::Read(uint64_t Length, uint64_t& Result)
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
	{
		// We can read the block
		int8_t* ResultBlock = new int8_t[(size_t)Length];
		// Zero out the memory
		std::memset(ResultBlock, 0, (size_t)Length);
		// Read it
		size_t LengthRead = fread_s(ResultBlock, (size_t)Length, 1, (size_t)Length, FileHandle);
		// Set result
		Result = LengthRead;
		// Return result
		return ResultBlock;
	}
	// Failed
	return nullptr;
}

int8_t* BinaryReader::Read(uint64_t Offset, uint64_t Length, uint64_t& Result)
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
	{
		// Go to offset first
		_fseeki64(FileHandle, Offset, SEEK_SET);
		// Read
		return Read(Length, Result);
	}
	// Failed
	return nullptr;
}

void BinaryReader::Read(uint8_t* Buffer, uint64_t Length, uint64_t& Result)
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
	{
		// Read to the buffer
		Result = fread(Buffer, 1, Length, FileHandle);
	}
}

void BinaryReader::Read(uint8_t* Buffer, uint64_t Offset, uint64_t Length, uint64_t& Result)
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
	{
		// Go to offset first
		_fseeki64(FileHandle, Offset, SEEK_SET);
		// Read
		Read(Buffer, Length, Result);
	}
}

int64_t BinaryReader::Scan(const std::string& Pattern)
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
	{
		// The resulting offset
		intptr_t ResultOffset = 0;

		// Pattern data
		std::string PatternBytes;
		std::string PatternMask;

		// Process the input patterm
		Patterns::ProcessPattern(Pattern, PatternBytes, PatternMask);

		// Total amount of data read
		uint64_t DataRead = 0;

		// Read the file data into chunks then scan for the result (100mb buffers padded for the size)
		uint64_t ChunkSize = (0x5F5E100 + (0x5F5E100 % PatternMask.size()));

		// Result
		int64_t Result = -1;

		// Loop
		while (true)
		{
			// Chunk position
			auto StartPosition = GetPosition();
			// Read chunk chunk
			int8_t* ChunkToScan = Read(ChunkSize, DataRead);

			// Scan the chunk
			Result = Patterns::ScanBlock(PatternBytes, PatternMask, (uint64_t)ChunkToScan, DataRead);

			// Clean up the memory block
			delete[] ChunkToScan;

			// Check if we got it, if so, add chunk offset then jump to found position
			if (Result > -1)
			{
				// We got it, hop to offset
				SetPosition(Result + StartPosition);
				// Return the offset of the item in the file
				return (Result + StartPosition);
			}

			// Check if we're done
			if (DataRead < ChunkSize) { break; }
		}

		// Return it
		return Result;
	}
	// Failed
	return -1;
}

int64_t BinaryReader::Scan(const std::string& Pattern, uintptr_t Offset)
{
	// Make sure we are loaded
	if (FileHandle != nullptr)
	{
		// Go to offset first
		_fseeki64(FileHandle, Offset, SEEK_SET);
		// Scan
		return Scan(Pattern);
	}
	// Failed
	return -1;
}