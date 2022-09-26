#include "stdafx.h"
#include "WinFileSystem.h"

ps::WinFileSystem::WinFileSystem(const std::string& directory)
{
	Directory = directory;

	// Check if the directory actually exists.
	DWORD dwAttrib = GetFileAttributesA(directory.c_str());
	
	if (dwAttrib == INVALID_FILE_ATTRIBUTES || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		LastErrorCode = 0x5005050;
	}
	else
	{
		LastErrorCode = 0;
	}
}

ps::WinFileSystem::~WinFileSystem()
{
	for (auto openHandle : OpenHandles)
	{
		CloseHandle(openHandle);
	}

	OpenHandles.clear();
}

HANDLE ps::WinFileSystem::OpenFile(const std::string& fileName, const std::string& mode)
{
	HANDLE result = NULL;
	char buffer[MAX_PATH]{};

	if (sprintf_s(buffer, "%s%s%s", Directory.c_str(), (Directory.size() > 0 ? "\\" : ""), fileName.c_str()) <= 0)
	{
		LastErrorCode = 0x505000;
		return NULL;
	}

	if (mode == "r")
	{
		result = CreateFileA(
			buffer,
			FILE_READ_DATA | FILE_READ_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
	}
	else if (mode == "w")
	{
		result = CreateFileA(
			buffer,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_NEW | OPEN_EXISTING,
			0,
			NULL);
	}
	else
	{
		LastErrorCode = 0x505001;
		return NULL;
	}

	if (result == INVALID_HANDLE_VALUE || result == NULL)
	{
		LastErrorCode = 0x505002;
		return NULL;
	}

	LastErrorCode = 0;
	return result;
}

void ps::WinFileSystem::CloseHandle(HANDLE handle)
{
	::CloseHandle(handle);
}

bool ps::WinFileSystem::Exists(const std::string& fileName)
{
	auto fullPath = Directory + "\\" + fileName;
	DWORD dwAttrib = GetFileAttributesA(fullPath.c_str());
	LastErrorCode = 0;
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

size_t ps::WinFileSystem::Read(HANDLE handle, uint8_t* buffer, const size_t offset, const size_t size)
{
	DWORD sizeRead = 0;

	if (!ReadFile(handle, buffer, (DWORD)size, &sizeRead, NULL))
	{
		LastErrorCode = 0x505003;
		return 0;
	}

	return sizeRead;
}

size_t ps::WinFileSystem::Write(HANDLE handle, const uint8_t* buffer, const size_t offset, const size_t size)
{
	DWORD sizeWritten = 0;

	if (!WriteFile(handle, buffer, (DWORD)size, &sizeWritten, NULL))
	{
		LastErrorCode = 0x505004;
		return 0;
	}

	return sizeWritten;
}

size_t ps::WinFileSystem::Tell(HANDLE handle)
{
	LARGE_INTEGER output{};

	if (!SetFilePointerEx(handle, { 0 }, &output, FILE_CURRENT))
	{
		LastErrorCode = 0x505005;
		return 0;
	}

	LastErrorCode = 0;
	return output.QuadPart;
}

size_t ps::WinFileSystem::Seek(HANDLE handle, size_t position, size_t direction)
{
	LARGE_INTEGER input{};
	input.QuadPart = position;
	LARGE_INTEGER output{};
	output.QuadPart = position;

	if (!SetFilePointerEx(handle, input, &output, (DWORD)direction))
	{
		LastErrorCode = 0x505006;
		return 0;
	}

	LastErrorCode = 0;
	return output.QuadPart;
}

size_t ps::WinFileSystem::Size(HANDLE handle)
{
	LARGE_INTEGER output{};

	if (!GetFileSizeEx(handle, &output))
	{
		LastErrorCode = 0x505007;
		return 0;
	}

	return output.QuadPart;
}

size_t ps::WinFileSystem::EnumerateFiles(const std::string& pattern, std::function<void(const std::string&, const size_t)> onFileFound)
{
	HANDLE findHandle;
	WIN32_FIND_DATAA findData;
	size_t results = 0;

	auto fullPath = Directory + "\\" + pattern;
	findHandle = FindFirstFileA(fullPath.c_str(), &findData);

	if (findHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			onFileFound(findData.cFileName, (size_t)findData.nFileSizeLow | ((size_t)findData.nFileSizeHigh << 32));
			results++;
		} while (FindNextFileA(findHandle, &findData));

		FindClose(findHandle);
	}

	return results;
}