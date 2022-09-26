#pragma once
#include "FileSystem.h"

namespace ps
{
	// A class to hold a safe file handle that automatically closes itself.
	class FileHandle
	{
	private:
		// The file system this handle was opened from.
		ps::FileSystem* FileSystem;
		// The actual file handle.
		HANDLE Handle;
	public:
		// Creates a new file handle.
		FileHandle(HANDLE handle, ps::FileSystem* fileSystem);
		// Checks if the file handle is valid.
		const bool IsValid() const;
		// Destroys the file handle.
		~FileHandle();
		// Closes the file handle.
		void Close();
		// Gets the file handle.
		HANDLE GetHandle();
	};
}