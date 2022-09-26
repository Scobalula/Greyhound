#pragma once
#include "FileSystem.h"

namespace ps
{
	// A class for handling Windows File System.
	class WinFileSystem : public FileSystem
	{
	public:
		// Creates a new File System instance.
		WinFileSystem(const std::string& directory);
		// Destroys the File System instance.
		~WinFileSystem();
		// Opens a file with the given name and mode.
		virtual HANDLE OpenFile(const std::string& fileName, const std::string& mode);
		// Closes the handle only.
		virtual void CloseHandle(HANDLE handle);
		// Checks if the provided file exists.
		virtual bool Exists(const std::string& fileName);
		// Reads data from the file.
		virtual size_t Read(HANDLE handle, uint8_t* buffer, const size_t offset, const size_t size);
		// Reads data from the file.
		virtual size_t Write(HANDLE handle, const uint8_t* buffer, const size_t offset, const size_t size);
		// Tells the file position.
		virtual size_t Tell(HANDLE handle);
		// Seeks within a file.
		virtual size_t Seek(HANDLE handle, size_t position, size_t direction);
		// Gets the size of the file.
		virtual size_t Size(HANDLE handle);
		// Gets the list of files matching the provided pattern.
		virtual size_t EnumerateFiles(const std::string& pattern, std::function<void(const std::string&, const size_t)> onFileFound);
	};
}