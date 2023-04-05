#pragma once

#include <string>
#include <cstdint>
#include <memory>

// We need the following classes
#include "CoDAssetType.h"
#include "WraithBinaryReader.h"
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
    // Loads a sound file from a SAB package that uses Opus
    static std::unique_ptr<XSound> LoadOpusSound(const CoDSound_t* SoundAsset);

    // -- Decoding functions

    // Decodes an Opus Buffer
    static std::unique_ptr<XSound> DecodeOpusInterleaved(uint8_t* OpusBuffer, size_t OpusBufferSize, size_t OpusDataOffset, uint32_t FrameRate, uint32_t Channels, uint32_t FrameCount);

private:

    // -- Utility functions

    // Handles decrypting SAB files to unencrypted results and loading them
    static void HandleSABEncryption(BinaryReader& Reader, const std::string& FilePath);

    // -- Game specific load functions

    // Handle reading entries for a v4 SAB file (0x4)
    static void HandleSABv4(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex);
    // Handle reading entries for a v10 SAB file (0xA)
    static void HandleSABv10(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex);
    // Handle reading entries for a v14 SAB file (0xE)
    static void HandleSABv14(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex);
    // Handle reading entries for a v15 SAB file (0xF)
    static void HandleSABv15(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex);
    // Handle reading entries for a v17 SAB file (0x11)
    static void HandleSABv17(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex);
    // Handle reading entries for a v21 SAB file (0x15)
    static void HandleSABv21(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex, const std::string FileName);
    // Handle reading entries for a v21 SAB file (0x15) (Modern Warfare 5)
    static void HandleSABv21v2(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex, const std::string FileName);
};