#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <Windows.h>

// -- Structures

// Represents a process on this PC
struct Process
{
    // The name of this process, including extensions
    std::string ProcessName;
    // The process ID, provided by windows
    uint32_t ProcessID;
    // The parent process ID, provided by windows
    uint32_t ParentProcessID;
};

// -- End structures

class Systems
{
public:
    // -- Process related functions

    // Gets a list of all running processes on this PC
    static std::vector<Process> GetProcesses();

    // Allows our process to debug other processes
    static void EnterDebugMode();
    // Leaves debugging mode
    static void LeaveDebugMode();

    // Open the file, shared access mode
    static FILE* OpenFileShared(const std::string& FileName, const char* Mode);

private:

    // Sets an internal process attribute
    static void SetPrivilege(const std::string& Name, bool EnablePrivilege);
};