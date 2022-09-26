#include "stdafx.h"
#include "FileHandle.h"

ps::FileHandle::FileHandle(HANDLE handle, ps::FileSystem* fileSystem)
{
    FileSystem = fileSystem;
    Handle = handle;
}

const bool ps::FileHandle::IsValid() const
{
    return Handle != NULL && Handle != INVALID_HANDLE_VALUE;
}

ps::FileHandle::~FileHandle()
{
    FileSystem->CloseFile(Handle);
}

void ps::FileHandle::Close()
{
    FileSystem->CloseFile(Handle);
}

HANDLE ps::FileHandle::GetHandle()
{
    return Handle;
}
