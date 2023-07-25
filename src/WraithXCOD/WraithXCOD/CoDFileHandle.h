#pragma once
#include "CoDFileSystem.h"

// A class to hold a safe file handle that automatically closes itself.
class CoDFileHandle
{
private:
	// The actual file handle.
	HANDLE Handle;
	// The file system this handle was opened from.
	CoDFileSystem* FileSystem;
public:
	// Creates a new file handle.
	CoDFileHandle();
	// Creates a new file handle.
	CoDFileHandle(HANDLE handle, CoDFileSystem* fileSystem);
	// Opens the file handle.
	bool Open(HANDLE handle, CoDFileSystem* fileSystem);
	// Destroys the file handle.
	~CoDFileHandle();
	// Checks if the file handle is valid.
	const bool IsValid() const;
	// Reads data from the file.
	std::unique_ptr<uint8_t[]> Read(const size_t size);
	// Reads data from the file.
	size_t Read(uint8_t* buffer, const size_t size);
	// Reads data from the file.
	size_t Read(uint8_t* buffer, const size_t offset, const size_t size);
	// Reads data from the file.
	size_t Write(const uint8_t* buffer, const size_t size);
	// Reads data from the file.
	size_t Write(const uint8_t* buffer, const size_t offset, const size_t size);
	// Tells the file position.
	size_t Tell();
	// Seeks within a file.
	size_t Seek(size_t position, size_t direction);
	// Gets the size of the file.
	size_t Size();
	// Reads data from the file.
	template <class T>
	T Read()
	{
		T r{};
		Read((uint8_t*)&r, 0, sizeof(r));
		return r;
	}
	// Closes the file handle.
	void Close();
	// Gets the file handle.
	HANDLE GetHandle();
};