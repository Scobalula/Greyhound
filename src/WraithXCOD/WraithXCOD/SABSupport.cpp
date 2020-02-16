#include "stdafx.h"

// The class we are implementing
#include "SABSupport.h"

// We need the following WraithX classes
#include "MemoryReader.h"
#include "BinaryReader.h"
#include "Strings.h"
#include "Sound.h"
#include "SettingsManager.h"
#include "WraithNameIndex.h"
#include "Encryption.h"
#include "FileSystems.h"

// We need the file classes
#include "CoDAssets.h"
#include "DBGameFiles.h"

// Calculates the hash of a sound string
uint32_t HashSoundString(const std::string& Value)
{
    uint32_t Result = 5381;

    for (auto& Character : Value)
        Result = (uint32_t)(tolower(Character) + (Result << 6) + (Result << 16)) - Result;

    return Result;
}

bool SABSupport::ParseSAB(const std::string& FilePath)
{
    // Prepare to parse and load entries from this file
    auto Reader = BinaryReader();
    // Open the file
    Reader.Open(FilePath);

    // Now, we must check for encrypted files, introduced in Black Ops 3
    // We have pre-set files for this, just pass to the function
    HandleSABEncryption(Reader, FilePath);

    // As of Infinite Warfare, the SAB header remains unchanged, we can read it to determine the version
    auto Header = Reader.Read<SABFileHeader>();

    // Verify magic first, same for all SAB files
    // The magic is ('2UX#')
    if (Header.Magic != 0x23585532)
    {
        // Failed, invalid file
        return false;
    }

    // Currently, the name table is at the same offset, 0x250, right before the zone name.
    // If there is no offset, there are no files names, however, there can be no names at an
    // Offset though
    Reader.SetPosition(0x250);
    // Read the offset
    auto NameTableOffset = Reader.Read<uint64_t>();

    // If a file has names, then every entry must have one, we can determine this by checking the offset
    // And checking the first value, if it has names, we read them first
    std::vector<std::string> SABFileNames;

    // Get File name for Bo4
    auto FileName = FileSystems::GetFileName(FilePath);

    // Get Settings
    auto UseIndex = SettingsManager::GetSetting("usesabindexbo4", "false") == "true";

    // Check if we have an offset, and the first entry is not blank
    bool HasNames = (NameTableOffset > 0) && (Reader.Read<uint64_t>(NameTableOffset) != 0);

    // If we don't have names, we can pre-load a name index here, depending on the game
    WraithNameIndex SABNames;
    // Check status, then load game specific file
    if (!HasNames)
    {
        // We have names for bo2 and bo3
        if (Header.Version == 0xE) { SABNames.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo2_sab.wni")); }
        if (Header.Version == 0xF) { SABNames.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo3_sab.wni")); }
    }

    // If we have names, read them, but pre-allocate the names first
    if (HasNames)
    {
        // Pre-allocate
        SABFileNames.reserve(Header.EntriesCount);

        // Jump to offset and read
        Reader.SetPosition(NameTableOffset);

        if (Header.Version == 0x15)
        {
            SABNames.LoadIndex(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo4_sab.wni"));

            // Loop and read
            for (uint32_t i = 0; i < Header.EntriesCount; i++)
            {
                // Read and mask it
                auto Hash = Reader.Read<uint64_t>() & 0xFFFFFFFFFFFFFFF;
                // Format Hash
                auto Name = Strings::Format("xsound_%llx", Hash);
                // Check for an override in the name DB
                if (SABNames.NameDatabase.find(Hash) != SABNames.NameDatabase.end())
                {
                    Name = SABNames.NameDatabase[Hash];
                }
                else
                {
                    if (UseIndex)
                    {
                        Name = Strings::Format("%s.%i", FileName.c_str(), i);
                    }
                }
                // Calculate jump
                auto PositionAdvance = (Header.SizeOfNameEntry * 2) - 8;
                // Emplace it back
                SABFileNames.emplace_back(Name);
                // Advance size of name entry * 2 - string size
                Reader.Advance(PositionAdvance);
            }
        }
        else
        {
            // Loop and read
            for (uint32_t i = 0; i < Header.EntriesCount; i++)
            {
                // Read
                auto Name = Reader.ReadNullTerminatedString();
                // Calculate jump
                auto PositionAdvance = (Header.SizeOfNameEntry * 2) - (Name.size() + 1);
                // Emplace it back
                SABFileNames.emplace_back(Name);
                // Advance size of name entry * 2 - string size
                Reader.Advance(PositionAdvance);
            }
        }
    }


    // Next step will be reading the entry table, this differs per-game, but we have it's
    // Size in the header, so it can be semi-generic
    Reader.SetPosition(Header.EntryTableOffset);

    // Read per-game entries
    switch (Header.Version)
    {
    case 0x4: 
        // Set the game, then load
        CoDAssets::GameID = SupportedGames::InfiniteWarfare;
        // Load
        HandleSABv4(Reader, Header, SABFileNames, SABNames); break;
    case 0xA:
        // Set the game, then load
        CoDAssets::GameID = SupportedGames::ModernWarfare4;
        // Load
        HandleSABv10(Reader, Header, SABFileNames, SABNames); break;
    case 0xE: 
        // Set the game, then load
        CoDAssets::GameID = SupportedGames::BlackOps2;
        // Load
        HandleSABv14(Reader, Header, SABFileNames, SABNames); break;
    case 0xF: 
        // Set the game, then load
        CoDAssets::GameID = SupportedGames::BlackOps3;
        // Load
        HandleSABv15(Reader, Header, SABFileNames, SABNames); break;
    case 0x15: 
        // Set the game, then load
        CoDAssets::GameID = SupportedGames::BlackOps4;
        // Load
        HandleSABv21(Reader, Header, SABFileNames, SABNames); break;
    }

    // Clean up names
    SABFileNames.clear();
    SABFileNames.shrink_to_fit();

    // Success unless otherwise
    return true;
}

std::unique_ptr<XSound> SABSupport::LoadSound(const CoDSound_t* SoundAsset)
{
    // Prepare to extract the sound asset, and clean up the data if need be
    auto& SoundBankPath = CoDAssets::GamePackageCache->GetPackagesPath();

    // Open the bank file
    auto Reader = BinaryReader();
    // Open it
    Reader.Open(SoundBankPath);

    // Handle encryption once again
    HandleSABEncryption(Reader, SoundBankPath);

    // Jump to the offset
    Reader.SetPosition(SoundAsset->AssetPointer);

    // Allocate the sound asset
    auto SoundResult = std::make_unique<XSound>();

    // Determine if we need header space and set format
    auto HeaderSpace = 0;
    // Check
    if (SoundAsset->DataType == SoundDataTypes::WAV_NeedsHeader)
    {
        // Set space and format
        HeaderSpace = Sound::GetMaximumWAVHeaderSize();
        SoundResult->DataType = SoundDataTypes::WAV_WithHeader;
    }
    if (SoundAsset->DataType == SoundDataTypes::FLAC_NeedsHeader)
    {
        // Set space and format
        HeaderSpace = Sound::GetMaximumFLACHeaderSize();
        SoundResult->DataType = SoundDataTypes::FLAC_WithHeader;
    }

    // Allocate the buffer
    SoundResult->DataBuffer = new int8_t[(uint32_t)(HeaderSpace + SoundAsset->AssetSize)];
    // Set the buffer size
    SoundResult->DataSize = (uint32_t)(HeaderSpace + SoundAsset->AssetSize);
    
    // Result read
    uint64_t ReadResult = 0;
    // Read the buffer
    auto AudioBuffer = Reader.Read(SoundAsset->AssetSize, ReadResult);

    // Copy it
    std::memcpy((SoundResult->DataBuffer + HeaderSpace), AudioBuffer, ReadResult);

    // Clean up
    delete[] AudioBuffer;

    // Prepare the audio header
    if (SoundAsset->DataType == SoundDataTypes::WAV_NeedsHeader)
    {
        // Write WAV Header
        Sound::WriteWAVHeaderToStream(SoundResult->DataBuffer, SoundAsset->FrameRate, SoundAsset->ChannelsCount, (uint32_t)SoundAsset->AssetSize);
    }
    else if (SoundAsset->DataType == SoundDataTypes::FLAC_NeedsHeader)
    {
        // Write FLAC Header
        Sound::WriteFLACHeaderToStream(SoundResult->DataBuffer, SoundAsset->FrameRate, SoundAsset->ChannelsCount, SoundAsset->FrameCount);
    }

    // Return result
    return SoundResult;
}

void SABSupport::HandleSABv4(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex)
{
    // Get Settings
    auto SkipBlankAudio = SettingsManager::GetSetting("skipblankaudio", "false") == "true";

    // Prepare to loop and read entries
    for (uint32_t i = 0; i < Header.EntriesCount; i++)
    {
        // Read each entry
        auto Entry = Reader.Read<SABv4Entry>();

        // Prepare to parse the information to our generic structure
        std::string EntryName = "";
        // Check our options
        if (NameList.size() > 0)
        {
            // We have it in file
            EntryName = NameList[i];
        }
        else if (NameIndex.NameDatabase.find(Entry.Key) != NameIndex.NameDatabase.end())
        {
            // We have it in a database
            EntryName = NameIndex.NameDatabase.at(Entry.Key);
        }
        else
        {
            // We don't have one
            EntryName = Strings::Format("_%llx", Entry.Key);
        }

        // Setup a new entry
        auto LoadedSound = new CoDSound_t();
        // Set the name, but remove all extensions first
        LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(EntryName);
        LoadedSound->FullPath = FileSystems::GetDirectoryName(EntryName);

        // Set various properties
        LoadedSound->FrameRate = Entry.FrameRate;
        LoadedSound->FrameCount = Entry.FrameCount;
        LoadedSound->ChannelsCount = Entry.ChannelCount;
        // The offset should be after the seek table, since it is not required
        LoadedSound->AssetPointer = (Entry.Offset + Entry.SeekTableLength);
        LoadedSound->AssetSize = Entry.Size;
        LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
        LoadedSound->IsFileEntry = true;
        LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));
        // All Infinite Warfare (v4) entries are FLAC's with no header
        LoadedSound->DataType = SoundDataTypes::FLAC_NeedsHeader;

        // Check do we want to skip this
        if(SkipBlankAudio && LoadedSound->AssetSize <= 0)
        {
            delete LoadedSound;
            continue;
        }

        // Add
        CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
    }
}

void SABSupport::HandleSABv10(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex)
{
    // Get Settings
    auto SkipBlankAudio = SettingsManager::GetSetting("skipblankaudio", "false") == "true";

    // Names (sound names are not in order)
    std::unordered_map<uint32_t, std::string> SoundNames;

    // Loop and build hash table
    for (auto& Name : NameList)
        SoundNames[HashSoundString(Name)] = Name;

    // Prepare to loop and read entries
    for (uint32_t i = 0; i < Header.EntriesCount; i++)
    {
        // Read each entry
        auto Entry = Reader.Read<SABv4Entry>();

        // Prepare to parse the information to our generic structure
        std::string EntryName = "";

        // Override if we find it
        if (SoundNames.find(Entry.Key) != SoundNames.end())
        {
            // We have it in a database
            EntryName = SoundNames.at(Entry.Key);
        }
        else
        {
            // We don't have one
            EntryName = Strings::Format("_%llx", Entry.Key);
        }

        // Setup a new entry
        auto LoadedSound = new CoDSound_t();
        // Set the name, but remove all extensions first
        LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(EntryName);
        LoadedSound->FullPath = FileSystems::GetDirectoryName(EntryName);

        // Set various properties
        LoadedSound->FrameRate = Entry.FrameRate;
        LoadedSound->FrameCount = Entry.FrameCount;
        LoadedSound->ChannelsCount = Entry.ChannelCount;
        // The offset should be after the seek table, since it is not required
        LoadedSound->AssetPointer = (Entry.Offset + Entry.SeekTableLength);
        LoadedSound->AssetSize = Entry.Size;
        LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
        LoadedSound->IsFileEntry = true;
        LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));
        // All Infinite Warfare (v4) entries are FLAC's with no header
        LoadedSound->DataType = SoundDataTypes::FLAC_NeedsHeader;

        // Check do we want to skip this
        if (SkipBlankAudio && LoadedSound->AssetSize <= 0)
        {
            delete LoadedSound;
            continue;
        }

        // Add
        CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
    }
}

void SABSupport::HandleSABv14(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex)
{
    // Get Settings
    auto SkipBlankAudio = SettingsManager::GetSetting("skipblankaudio", "false") == "true";

    // Prepare to loop and read entries
    for (uint32_t i = 0; i < Header.EntriesCount; i++)
    {
        // Read each entry
        auto Entry = Reader.Read<SABv14Entry>();

        // Prepare to parse the information to our generic structure
        std::string EntryName = "";
        // Check our options
        if (NameList.size() > 0)
        {
            // We have it in file
            EntryName = NameList[i];
        }
        else if (NameIndex.NameDatabase.find(Entry.Key) != NameIndex.NameDatabase.end())
        {
            // We have it in a database
            EntryName = NameIndex.NameDatabase.at(Entry.Key);
        }
        else
        {
            // We don't have one
            EntryName = Strings::Format("_%llx", Entry.Key);
        }

        // Setup a new entry
        auto LoadedSound = new CoDSound_t();
        // Set the name, but remove all extensions first
        LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(EntryName);
        LoadedSound->FullPath = FileSystems::GetDirectoryName(EntryName);

        // Determine framerate
        switch (Entry.FrameRateIndex)
        {
        case 0: LoadedSound->FrameRate = 8000; break;
        case 1: LoadedSound->FrameRate = 12000; break;
        case 2: LoadedSound->FrameRate = 16000; break;
        case 3: LoadedSound->FrameRate = 24000; break;
        case 4: LoadedSound->FrameRate = 32000; break;
        case 5: LoadedSound->FrameRate = 44100; break;
        case 6: LoadedSound->FrameRate = 48000; break;
        case 7: LoadedSound->FrameRate = 96000; break;
        case 8: LoadedSound->FrameRate = 192000; break;
        }

        // Set various properties
        LoadedSound->FrameCount = Entry.FrameCount;
        LoadedSound->ChannelsCount = Entry.ChannelCount;
        LoadedSound->AssetPointer = Entry.Offset;
        LoadedSound->AssetSize = Entry.Size;
        LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
        LoadedSound->IsFileEntry = true;
        LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));

        // Check the format byte and set the format
        switch (Entry.Format)
        {
        case 0: LoadedSound->DataType = SoundDataTypes::WAV_NeedsHeader; break;
        case 8: LoadedSound->DataType = SoundDataTypes::FLAC_WithHeader; break;
        }

        // Check do we want to skip this
        if (SkipBlankAudio && LoadedSound->AssetSize <= 0)
        {
            delete LoadedSound;
            continue;
        }

        // Add
        CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
    }
}

void SABSupport::HandleSABv15(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex)
{
    // Get Settings
    auto SkipBlankAudio = SettingsManager::GetSetting("skipblankaudio", "false") == "true";

    // Prepare to loop and read entries
    for (uint32_t i = 0; i < Header.EntriesCount; i++)
    {
        // Read each entry
        auto Entry = Reader.Read<SABv15Entry>();
        
        // Prepare to parse the information to our generic structure
        std::string EntryName = "";
        // Check our options
        if (NameList.size() > 0)
        {
            // We have it in file
            EntryName = NameList[i];
        }
        else if (NameIndex.NameDatabase.find(Entry.Key) != NameIndex.NameDatabase.end())
        {
            // We have it in a database
            EntryName = NameIndex.NameDatabase.at(Entry.Key);
        }
        else
        {
            // We don't have one
            EntryName = Strings::Format("_%llx", Entry.Key);
        }

        // Setup a new entry
        auto LoadedSound = new CoDSound_t();
        // Set the name, but remove all extensions first
        LoadedSound->AssetName = FileSystems::GetFileNamePurgeExtensions(EntryName);
        LoadedSound->FullPath = FileSystems::GetDirectoryName(EntryName);

        // Determine framerate
        switch (Entry.FrameRateIndex)
        {
        case 0: LoadedSound->FrameRate = 8000; break;
        case 1: LoadedSound->FrameRate = 12000; break;
        case 2: LoadedSound->FrameRate = 16000; break;
        case 3: LoadedSound->FrameRate = 24000; break;
        case 4: LoadedSound->FrameRate = 32000; break;
        case 5: LoadedSound->FrameRate = 44100; break;
        case 6: LoadedSound->FrameRate = 48000; break;
        case 7: LoadedSound->FrameRate = 96000; break;
        case 8: LoadedSound->FrameRate = 192000; break;
        }

        // Set various properties
        LoadedSound->FrameCount = Entry.FrameCount;
        LoadedSound->ChannelsCount = Entry.ChannelCount;
        LoadedSound->AssetPointer = Entry.Offset;
        LoadedSound->AssetSize = Entry.Size;
        LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
        LoadedSound->IsFileEntry = true;
        LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));

        // All Black Ops 3 (v15) entries are FLAC's with a header
        LoadedSound->DataType = SoundDataTypes::FLAC_WithHeader;

        // Check do we want to skip this
        if (SkipBlankAudio && LoadedSound->AssetSize <= 0)
        {
            delete LoadedSound;
            continue;
        }

        // Add
        CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
    }
}

void SABSupport::HandleSABv21(BinaryReader& Reader, const SABFileHeader& Header, const std::vector<std::string>& NameList, const WraithNameIndex& NameIndex)
{
    // Get Settings
    auto SkipBlankAudio = SettingsManager::GetSetting("skipblankaudio", "false") == "true";

    // Prepare to loop and read entries
    for (uint32_t i = 0; i < Header.EntriesCount; i++)
    {
        // Read each entry
        auto Entry = Reader.Read<SABv21Entry>();

        // Prepare to parse the information to our generic structure
        std::string EntryName = "";
        // Check our options
        if (NameList.size() > 0)
        {
            // We have it in file
            EntryName = NameList[i];
        }
        else if (NameIndex.NameDatabase.find(Entry.Key) != NameIndex.NameDatabase.end())
        {
            // We have it in a database
            EntryName = NameIndex.NameDatabase.at(Entry.Key);
        }
        else
        {
            // We don't have one
            EntryName = Strings::Format("_%llx", Entry.Key);
        }

        // Setup a new entry
        auto LoadedSound = new CoDSound_t();
        // Set the name, but remove all extensions first
        LoadedSound->AssetName = EntryName;
        LoadedSound->FullPath = FileSystems::GetDirectoryName(EntryName);

        // Determine framerate
        switch (Entry.FrameRateIndex)
        {
        case 0: LoadedSound->FrameRate = 8000; break;
        case 1: LoadedSound->FrameRate = 12000; break;
        case 2: LoadedSound->FrameRate = 16000; break;
        case 3: LoadedSound->FrameRate = 24000; break;
        case 4: LoadedSound->FrameRate = 32000; break;
        case 5: LoadedSound->FrameRate = 44100; break;
        case 6: LoadedSound->FrameRate = 48000; break;
        case 7: LoadedSound->FrameRate = 96000; break;
        case 8: LoadedSound->FrameRate = 192000; break;
        }

        // Set various properties
        LoadedSound->FrameCount = Entry.FrameCount;
        LoadedSound->ChannelsCount = Entry.ChannelCount;
        LoadedSound->AssetPointer = Entry.Offset;
        LoadedSound->AssetSize = Entry.Size;
        LoadedSound->AssetStatus = WraithAssetStatus::Loaded;
        LoadedSound->IsFileEntry = true;
        LoadedSound->Length = (uint32_t)(1000.0f * (float)(LoadedSound->FrameCount / (float)(LoadedSound->FrameRate)));

        // All Black Ops 4 (v21) entries are FLAC's with a header
        LoadedSound->DataType = SoundDataTypes::FLAC_WithHeader;

        // Check do we want to skip this
        if (SkipBlankAudio && LoadedSound->AssetSize <= 0)
        {
            delete LoadedSound;
            continue;
        }

        // Add
        CoDAssets::GameAssets->LoadedAssets.push_back(LoadedSound);
    }
}

void SABSupport::HandleSABEncryption(BinaryReader& Reader, const std::string& FilePath)
{
    // Currently, the only encrypted SAB files are in Black Ops 3
    // They are identified using zm_genesis and use the same key
    // The encryption is a SALSA20, the key is taken from a Base64 string
    // - Key 1: kZLU3vLqN1jnSgDBFiEmRguT0O1u7dxk2CGrqRReL8lKG7ECAAAAAA==

    if (Strings::Contains(FileSystems::GetFileName(FilePath), "zm_genesis"))
    {
        // We must swap out for a decrypted reader
        if (!FileSystems::FileExists(FilePath + ".wraithdec"))
        {
            // Allocate a key / iv buffer
            // Current data is taken from Key 1 (32 bytes, 8 bytes)
            uint8_t KeyData[32] = { 0x91, 0x92, 0xD4, 0xDE, 0xF2, 0xEA, 0x37, 0x58, 0xE7, 0x4A, 0x0, 0xC1, 0x16, 0x21, 0x26, 0x46, 0xB, 0x93, 0xD0, 0xED, 0x6E, 0xED, 0xDC, 0x64, 0xD8, 0x21, 0xAB, 0xA9, 0x14, 0x5E, 0x2F, 0xC9 };
            uint8_t IVData[8] = { 0x4A, 0x1B, 0xB1, 0x2, 0x0, 0x0, 0x0, 0x0 };

            // Result size
            uint64_t ResultSize = 0;
            // Decrypt the buffer to a file
            auto FileBuffer = Reader.Read(Reader.GetLength(), ResultSize);
            // Close the file
            Reader.Close();

            // The key
            Salsa20Key Key;
            // Set data
            Key.Key = &KeyData[0];
            Key.IV = &IVData[0];
            Key.KeyLength = 256;
            // Decrypt it
            Encryption::Salsa20Block(FileBuffer, (uint32_t)ResultSize, Key);

            // Write the clean buffer to a file
            auto Writer = BinaryWriter();

            // Write the buffer to the decrypted file...
            Writer.Create(FilePath + ".wraithdec");
            Writer.Write(FileBuffer, (uint32_t)ResultSize);
            Writer.Close();

            // Clean up
            delete[] FileBuffer;
        }

        // Open new file
        Reader.Open(FilePath + ".wraithdec");
    }
}