#pragma once

namespace ps
{
	// A class for handling a File System.
	class FileSystem
	{
	protected:
		// The directory where we are loading from.
		std::string Directory;
		// The last error code.
		size_t LastErrorCode = 0;
		// Open file handles.
		std::vector<HANDLE> OpenHandles;
	public:
		// Opens a file with the given name and mode.
		virtual HANDLE OpenFile(const std::string& fileName, const std::string& mode) = 0;
		// Destroys the file system.
		virtual ~FileSystem();
		// Closes the file.
		virtual void CloseFile(HANDLE handle);
		// Closes the handle only.
		virtual void CloseHandle(HANDLE handle) = 0;
		// Checks if the provided file exists.
		virtual bool Exists(const std::string& fileName) = 0;
		// Reads data from the file.
		virtual std::unique_ptr<uint8_t[]> Read(HANDLE handle, const size_t size);
		// Reads data from the file.
		virtual size_t Read(HANDLE handle, uint8_t* buffer, const size_t offset, const size_t size) = 0;
		// Reads data from the file.
		template <class T>
		T Read(HANDLE handle)
		{
			T r{};
			Read(handle, (uint8_t*)&r, 0, sizeof(r));
			return r;
		}
		// Reads data from the file.
		virtual size_t Write(HANDLE handle, const uint8_t* buffer, const size_t size);
		// Reads data from the file.
		virtual size_t Write(HANDLE handle, const uint8_t* buffer, const size_t offset, const size_t size) = 0;
		// Tells the file position.
		virtual size_t Tell(HANDLE handle) = 0;
		// Seeks within a file.
		virtual size_t Seek(HANDLE handle, size_t position, size_t direction) = 0;
		// Gets the size of the file.
		virtual size_t Size(HANDLE handle) = 0;
		// Gets the list of files matching the provided pattern.
		virtual size_t EnumerateFiles(const std::string& pattern, std::function<void(const std::string&, const size_t)> onFileFound) = 0;
		// Gets the last error.
		const size_t GetLastError() const;
		// Checks if the last call provided a valid result.
		const bool IsValid() const;
	};
}

