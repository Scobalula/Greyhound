#pragma once

#include <string>

// We require the strings utility
#include "Strings.h"

// A class that handles interfacing with WraithUpdater
class WraithUpdate
{
public:

    // Attempts to check for updates, can wait until the spawned process closes
    static void CheckForUpdates(const std::string& GithubName, const std::string& GithubRepoName, const std::string& ApplicationName, const std::string& AssemblyName, bool WaitForResult = false);
};