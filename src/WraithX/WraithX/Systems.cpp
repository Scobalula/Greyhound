#include "stdafx.h"

// The class we are implementing
#include "Systems.h"

std::vector<Process> Systems::GetProcesses()
{
    // Get a list of running processes
    std::vector<Process> Result;

    // Open the specified process
    PROCESSENTRY32 Entry;
    // Prepare the size
    Entry.dwSize = sizeof(PROCESSENTRY32);
    // Create a system snapshot
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    // Loop through results
    if (Process32First(Snapshot, &Entry) == TRUE)
    {
        do
        {
            // Get the name properly
            _bstr_t NameMarshal(Entry.szExeFile);
            // Grab it
            const char* ProcessExeFile = NameMarshal;
            // Setup and add
            Process NewProcess;
            // Set
            NewProcess.ProcessName = std::string(ProcessExeFile);
            NewProcess.ProcessID = Entry.th32ProcessID;
            NewProcess.ParentProcessID = Entry.th32ParentProcessID;
            // Add
            Result.push_back(NewProcess);
            // Loop
        } while (Process32Next(Snapshot, &Entry));
    }
    // Clean up
    CloseHandle(Snapshot);

    // Return it
    return Result;
}

void Systems::EnterDebugMode()
{
    // Prepare to enter
    SetPrivilege("SeDebugPrivilege", true);
}

void Systems::LeaveDebugMode()
{
    // Prepare to enter
    SetPrivilege("SeDebugPrivilege", false);
}

FILE* Systems::OpenFileShared(const std::string& FileName, const char* Mode)
{
    // Open the file
    HANDLE hFile = CreateFileA(FileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    // Ensure success
    if (hFile == INVALID_HANDLE_VALUE)
        return nullptr;

    // Grab a handle
    int nHandle = _open_osfhandle((long)hFile, _O_RDONLY);
    if (nHandle == -1)
    {
        // Failed, clean up
        ::CloseHandle(hFile);
        return nullptr;
    }

    // Load it
    FILE* FileHandle = _fdopen(nHandle, Mode);

    // Ensure success
    if (!FileHandle)
    {
        // Failed, clean up
        ::CloseHandle(hFile);
        FileHandle = nullptr;
    }

    // Return it
    return FileHandle;
}

void Systems::SetPrivilege(const std::string& Name, bool EnablePrivilege)
{
    // Our current process
    HANDLE hToken;

    // Open the token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        if (GetLastError() == ERROR_NO_TOKEN)
        {
            // Validate the token once more
            if (!ImpersonateSelf(SecurityImpersonation))
                return;
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
                return;
        }
        else
        {
            // Failed
            return;
        }
    }

    // Privilege buffers
    TOKEN_PRIVILEGES tp;
    LUID luid;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD cbPrevious = sizeof(TOKEN_PRIVILEGES);

    // Fetch default value
    if (!LookupPrivilegeValueA(NULL, Name.c_str(), &luid)) return;

    // First pass. Get current privilege setting
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = 0;

    // Get privilege setting
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &tpPrevious, &cbPrevious);

    // Ensure success
    if (GetLastError() != ERROR_SUCCESS) return;

    // Second pass. Set privilege based on previous setting
    tpPrevious.PrivilegeCount = 1;
    tpPrevious.Privileges[0].Luid = luid;

    // Enable or disable the flags
    if (EnablePrivilege)
        tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
    else
        tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tpPrevious.Privileges[0].Attributes);

    // Set the new token value
    AdjustTokenPrivileges(hToken, FALSE, &tpPrevious, cbPrevious, NULL, NULL);
}