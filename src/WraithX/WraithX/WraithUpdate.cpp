#include "stdafx.h"

// The class we are implementing
#include "WraithUpdate.h"

// We need the following classes
#include "Strings.h"
#include "FileSystems.h"

void TryCleanupTemp(std::string Dir)
{
    // Try delete them, we don't really care if we fail, as they will be
    // cleaned up eventually down the line, by either or us or Windows
    for (auto& File : FileSystems::GetFiles(Dir, "GreyhoundUpdater*.exe"))
    {
        FileSystems::DeleteFile(File);
    }
}

void WraithUpdate::CheckForUpdates(const std::string& GithubName, const std::string& GithubRepoName, const std::string& ApplicationName, const std::string& AssemblyName, bool WaitForResult)
{
    // Attempt to check for updates to the tool
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFOA StartupInfo;

    // Initialize
    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    // Get Temp
    char TempPath[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, TempPath))
        return;
    // Clean up
    TryCleanupTemp(TempPath);
    // Get path
    auto UpdaterPath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "GreyhoundUpdater.exe");
    auto NewPath = FileSystems::CombinePath(TempPath, Strings::Format("GreyhoundUpdater_%llx.exe", std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now())));
    FileSystems::CopyFile(UpdaterPath, NewPath);

    // Build arguments
    auto Arguments = Strings::Format("\"%s\" %s %s %s %s \"%s\" true", NewPath.c_str(), GithubName.c_str(), GithubRepoName.c_str(), ApplicationName.c_str(), AssemblyName.c_str(), FileSystems::GetApplicationPath().c_str());

    // Create the process
    if (CreateProcessA(NULL, (char*)Arguments.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo))
    {
        // Check to wait
        if (WaitForResult)
        {
            // Wait
            WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

            // Clean up
            CloseHandle(ProcessInfo.hThread);
            CloseHandle(ProcessInfo.hProcess);
        }
    }
}