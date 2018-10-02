#pragma once

#include <string>
#include <cstdint>
#include <memory>

// We need the following classes
#include "CoDAssetType.h"
#include "BinaryReader.h"
#include "DBGameFiles.h"
#include "WraithNameIndex.h"

// A class that handles parsing SAB files
class SABSupport
{
public:

	// -- Parsing functions

	// Parse and load sound assets within a SAB file
	static bool ParseSAB(const std::string& FilePath);

	// -- Exporting functions

	// Loads a sound file from a SAB package
	static std::unique_ptr<XSound> LoadSound(const CoDSound_t* SoundAsset);

private:

	// -- Utility functions

	// Handles decrypting SAB files to unencrypted results and loading them
	static void HandleSABEncryption(BinaryReader& Reader, const std::string& FilePath);

	// -- Game specific load functions

	// Handle reading entries for a v4 SAB file (0x4)
	static void HandleSABv4(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex);
	// Handle reading entries for a v14 SAB file (0xE)
	static void HandleSABv14(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex);
	// Handle reading entries for a v15 SAB file (0xF)
	static void HandleSABv15(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex);
	// Handle reading entries for a v21 SAB file (0x15)
	static void HandleSABv21(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex);
};