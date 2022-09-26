#include "stdafx.h"
#include "CascFileSystem.h"
#include "Strings.h"
#include "CascLib.h"

ps::CascFileSystem::CascFileSystem(const std::string& directory)
{
    StorageHandle = NULL;

    auto asWStr = Strings::ToUnicodeString(directory);

    if (!CascOpenStorage(asWStr.c_str(), CASC_LOCALE_NONE, &StorageHandle))
    {
        LastErrorCode = GetCascError();
    }
    else
    {
        LastErrorCode = 0;
    }
}

ps::CascFileSystem::~CascFileSystem()
{
    for (auto openHandle : OpenHandles)
    {
        CascCloseFile(openHandle);
    }

    OpenHandles.clear();
    CascCloseStorage(StorageHandle);
}

HANDLE ps::CascFileSystem::OpenFile(const std::string& fileName, const std::string& mode)
{
    // Casc only supports reading
    if (mode != "r")
    {
        LastErrorCode = 0x345300;
        return NULL;
    }

    HANDLE result = NULL;

    if (!CascOpenFile(StorageHandle, fileName.c_str(), CASC_LOCALE_NONE, CASC_OPEN_BY_NAME, &result))
    {
        LastErrorCode = GetCascError();
        result = NULL;
    }
    else
    {
        LastErrorCode = 0;
        OpenHandles.push_back(result);
    }

    return result;
}

void ps::CascFileSystem::CloseHandle(HANDLE handle)
{
    CascCloseFile(handle);
}

bool ps::CascFileSystem::Exists(const std::string& fileName)
{
    CASC_FIND_DATA findData{};
    HANDLE fileHandle = CascFindFirstFile(StorageHandle, fileName.c_str(), &findData, NULL);
    bool result = fileHandle != INVALID_HANDLE_VALUE && findData.bFileAvailable > 0;
    CascFindClose(fileHandle);
    return result;
}

size_t ps::CascFileSystem::Read(HANDLE handle, uint8_t* buffer, const size_t offset, const size_t size)
{
    DWORD sizeRead = 0;

    if (!CascReadFile(handle, buffer + offset, size, &sizeRead))
    {
        LastErrorCode = 0x345301;
    }
    else
    {
        LastErrorCode = 0;
    }

    return sizeRead;
}

size_t ps::CascFileSystem::Write(HANDLE handle, const uint8_t* buffer, const size_t offset, const size_t size)
{
    LastErrorCode = 0x345302;
    return 0;
}

size_t ps::CascFileSystem::Tell(HANDLE handle)
{
    ULONGLONG result = 0;

    if (CascSetFilePointer64(handle, 0, &result, FILE_CURRENT))
    {
        LastErrorCode = 0x345302;
    }
    else
    {
        LastErrorCode = 0;
    }

    return result;
}

size_t ps::CascFileSystem::Seek(HANDLE handle, size_t position, size_t direction)
{
    ULONGLONG result = 0;

    if (CascSetFilePointer64(handle, position, &result, direction))
    {
        LastErrorCode = 0x345303;
    }
    else
    {
        LastErrorCode = 0;
    }

    return result;
}

size_t ps::CascFileSystem::Size(HANDLE handle)
{
    if (handle == NULL)
    {
        LastErrorCode = 0x345303;
        return 0;
    }

    ULONGLONG result = 0;

    if (!CascGetFileSize64(handle, &result))
    {
        LastErrorCode = 0x345303;
        result = 0;
    }
    else
    {
        LastErrorCode = 0;
    }

    return result;
}

size_t ps::CascFileSystem::EnumerateFiles(const std::string& pattern, std::function<void(const std::string&, const size_t)> onFileFound)
{
    size_t entriesConsumed = 0;
    HANDLE fileHandle;
    CASC_FIND_DATA findData;

    fileHandle = CascFindFirstFile(StorageHandle, pattern.c_str(), &findData, NULL);

    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (findData.bFileAvailable)
            {
                if (findData.bFileAvailable)
                {
                    onFileFound(findData.szFileName, findData.FileSize);
                }
            }
        } while (CascFindNextFile(fileHandle, &findData));
    }

    CascFindClose(fileHandle);

    return false;
}
