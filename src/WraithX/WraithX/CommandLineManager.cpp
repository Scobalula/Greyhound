#include "stdafx.h"

// The class we are implementing
#include "CommandLineManager.h"

// We require the following utility functions
#include "Strings.h"

// Setup
std::unique_ptr<std::unordered_map<std::string, std::string>> CommandLineManager::CommandLineCache = nullptr;

void CommandLineManager::ParseCommandLine(const std::vector<std::string>& Arguments)
{
    // Prepare to load command line
    if (CommandLineCache == nullptr)
    {
        // Setup
        CommandLineCache.reset(new std::unordered_map<std::string, std::string>);
    }
    else
    {
        // Clear
        CommandLineCache->clear();
    }

    // TODO: Parse based on specifications

}