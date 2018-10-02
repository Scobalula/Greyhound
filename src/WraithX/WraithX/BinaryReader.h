#pragma once

#include <cstdint>
#include <string>
#include <cstdio>

// A class that handles reading and scanning a binary file for data
class BinaryReader
{
private:
	// A handle to the file
	FILE* FileHandle;
	// The length of the file
	uint64_t FileLength;

public:
	BinaryReader();
	~BinaryReader();

	// Open a file for reading
	bool Open(const std::string& FileName, bool Shared = false);
	// Connect this reader to an already open handle
	bool Connect(FILE* FileHandleReference);

	// Whether or not the file is still open
	bool IsOpen();

	// Close the file (If we aren't already closed)
	void Close();

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
			fread_s(&ResultValue, sizeof(ResultValue), sizeof(ResultValue), 1, FileHandle);
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
	template <class T>
	// Read a block of data from the file with the given type at the offset
	T Read(uint64_t Offset)
	{
		// Only if we're open
		if (FileHandle != nullptr)
		{
			// Go to the offset first
			_fseeki64(FileHandle, Offset, SEEK_SET);
			// Return the result
			return Read<T>();
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
	// Read a null-terminated string from the file with the given offset
	std::string ReadNullTerminatedString(uint64_t Offset);
	// Read a sized string from the file
	std::string ReadString(uint32_t Size);
	// Read a sized string from the file, with the given offset
	std::string ReadString(uint32_t Size, uint64_t Offset);
	// Read a block of data from the file with a given length
	int8_t* Read(uint64_t Length, uint64_t& Result);
	// Read a block of data from the file with the given length and offset
	int8_t* Read(uint64_t Offset, uint64_t Length, uint64_t& Result);
	// Read a block of data from the file with a given length to the buffer
	void Read(uint8_t* Buffer, uint64_t Length, uint64_t& Result);
	// Read a block of data from the file with the given length and offset to the buffer
	void Read(uint8_t* Buffer, uint64_t Offset, uint64_t Length, uint64_t& Result);

	// Scan the file for a pattern (From current position)
	int64_t Scan(const std::string& Pattern);
	// Scan the file for a pattern with the given offset
	int64_t Scan(const std::string& Pattern, uintptr_t Offset);
};
