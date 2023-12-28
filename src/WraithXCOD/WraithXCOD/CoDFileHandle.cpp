#include "stdafx.h"
#include "CoDFileHandle.h"

CoDFileHandle::CoDFileHandle(HANDLE handle, CoDFileSystem* fileSystem)
{
    FileSystem = fileSystem;
    Handle = handle;
}

const bool CoDFileHandle::IsValid() const
{
    return Handle != NULL && Handle != INVALID_HANDLE_VALUE;
}

std::unique_ptr<uint8_t[]> CoDFileHandle::Read(const size_t size)
{
    return FileSystem->Read(Handle, size);
}

size_t CoDFileHandle::Read(uint8_t* buffer, const size_t size)
{
    return Read(buffer, 0, size);
}

size_t CoDFileHandle::Read(uint8_t* buffer, const size_t offset, const size_t size)
{
    return FileSystem->Read(Handle, buffer, offset, size);
}

size_t CoDFileHandle::Write(const uint8_t* buffer, const size_t size)
{
    return Write(buffer, 0, size);
}

size_t CoDFileHandle::Write(const uint8_t* buffer, const size_t offset, const size_t size)
{
    return FileSystem->Write(Handle, buffer, offset, size);
}

size_t CoDFileHandle::Tell()
{
    return FileSystem->Tell(Handle);
}

size_t CoDFileHandle::Seek(size_t position, size_t direction)
{
    return FileSystem->Seek(Handle, position, direction);
}

CoDFileHandle::~CoDFileHandle()
{
    FileSystem->CloseFile(Handle);
}

size_t CoDFileHandle::Size()
{
    return FileSystem->Size(Handle);
}

void CoDFileHandle::Close()
{
    FileSystem->CloseFile(Handle);
}

HANDLE CoDFileHandle::GetHandle()
{
    return Handle;
}
