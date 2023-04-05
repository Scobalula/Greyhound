#include "pch.h"
#include "CoDFileSystem.h"

CoDFileSystem::~CoDFileSystem()
{
}

void CoDFileSystem::CloseFile(HANDLE handle)
{
	for (auto it = OpenHandles.begin(); it != OpenHandles.end(); )
	{
		if (*it == handle)
		{
			it = OpenHandles.erase(it);
		}
		else
		{
			it++;
		}
	}

	CloseHandle(handle);
}

std::unique_ptr<uint8_t[]> CoDFileSystem::Read(HANDLE handle, const size_t size)
{
	auto result = std::make_unique<uint8_t[]>(size);

	if (Read(handle, result.get(), 0, size) != size)
	{
		return nullptr;
	}

	return result;
}

size_t CoDFileSystem::Write(HANDLE handle, const uint8_t* buffer, const size_t size)
{
	return Write(handle, buffer, 0, size);
}

const size_t CoDFileSystem::GetLastError() const
{
	return LastErrorCode;
}

const bool CoDFileSystem::IsValid() const
{
	return LastErrorCode == 0;
}
