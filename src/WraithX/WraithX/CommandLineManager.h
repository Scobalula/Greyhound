#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

// A class that handles reading command line arguments
class CommandLineManager
{
private:
	// The command line cache
	static std::unique_ptr<std::unordered_map<std::string, std::string>> CommandLineCache;

public:
	// -- Functions

	// Parse the arguments given to the application
	static void ParseCommandLine(const std::vector<std::string>& Arguments);
};