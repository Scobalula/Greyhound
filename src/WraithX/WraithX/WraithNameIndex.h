#pragma once

#include <cstdint>
#include <string>
#include <cstdio>
#include <unordered_map>

// A class that handles reading and writing WraithNameIndex files
class WraithNameIndex
{
public:
	WraithNameIndex();
	WraithNameIndex(const std::string& FilePath);

	// A database of names from their hashes
	std::unordered_map<uint64_t, std::string> NameDatabase;

	// Save the current database to a file
	void SaveIndex(const std::string& FilePath);
	// Loads an index database
	void LoadIndex(const std::string& LoadIndex);
};