#include "stdafx.h"

// The class we are implementing
#include "SettingsManager.h"

// We require the following utility functions
#include "Strings.h"
#include "FileSystems.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "Hashing.h"

// Setup
std::unique_ptr<std::unordered_map<std::string, std::string>> SettingsManager::SettingsCache = nullptr;
// Default name
std::string SettingsManager::SettingsFileName = "";

void SettingsManager::LoadSettings(const std::string& SettingsName, const std::map<std::string, std::string>& Defaults)
{
    // Prepare to load settings
    if (SettingsCache == nullptr)
    {
        // Setup
        SettingsCache.reset(new std::unordered_map<std::string, std::string>);
    }
    else
    {
        // Clear
        SettingsCache->clear();
    }
    // Grab the current directory
    auto CurrentPath = FileSystems::GetApplicationPath();
    // Our settings file
    auto ConfigPath = FileSystems::CombinePath(CurrentPath, SettingsName + ".wcfg");

    // Set the path
    SettingsFileName = ConfigPath;

    // Make sure it exists
    if (FileSystems::FileExists(ConfigPath))
    {
        // Load existing configuration
        auto Reader = std::make_unique<BinaryReader>();
        // Load file
        Reader->Open(ConfigPath);
        // Read magic and verify
        if (Reader->Read<uint32_t>() == 0x47464357)
        {
            // Read count
            auto Count = Reader->Read<uint32_t>();
            // Loop
            for (uint32_t i = 0; i < Count; i++)
            {
                // Read the key, then value
                auto Key = Reader->ReadNullTerminatedString();
                auto Value = Reader->ReadNullTerminatedString();
                // Add
                SettingsCache->insert(std::make_pair(Key, Value));
            }
        }
    }

    // Setup default configuration, then resave
    for (auto& Keys : Defaults)
    {
        // Check and set
        if (SettingsCache->find(Keys.first) == SettingsCache->end())
        {
            // Add
            SettingsCache->insert(std::make_pair(Keys.first, Keys.second));
        }
    }

    // Resave
    SaveSettings();
}

void SettingsManager::SaveSettings()
{
    // Make a writer
    auto Writer = std::make_unique<BinaryWriter>();
    // Load the file
    Writer->Create(SettingsFileName);

    // Get key size
    auto KeySize = (uint32_t)SettingsCache->size();
    // Magic value
    char Magic[4] = { 'W', 'C', 'F', 'G' };
    // Write magic
    Writer->Write(Magic);
    // Write count
    Writer->Write<uint32_t>(KeySize);
    // Loop and write keys
    for (auto& Keys : *SettingsCache)
    {
        // Write them
        Writer->WriteNullTerminatedString(Keys.first);
        Writer->WriteNullTerminatedString(Keys.second);
    }
}

std::string SettingsManager::GetSetting(const std::string& Key, const std::string& Default)
{
    // Grab a key if it exists
    if (SettingsCache->find(Key) != SettingsCache->end())
    {
        // Return it
        return ModifyValue(Key, SettingsCache->at(Key));
    }
    // Add it
    SettingsCache->insert(std::make_pair(Key, UnModifyValue(Key, Default)));
    // Return
    return Default;
}

void SettingsManager::SetSetting(const std::string& Key, const std::string& Value)
{
    // The value to use
    auto NewValue = UnModifyValue(Key, Value);

    // Check for it
    if (SettingsCache->find(Key) != SettingsCache->end())
    {
        // Set
        SettingsCache->at(Key) = NewValue;
    }
    else
    {
        // Add
        SettingsCache->insert(std::make_pair(Key, NewValue));
    }

    // Save
    SaveSettings();
}

std::string SettingsManager::ModifyValue(const std::string& Key, const std::string& Value)
{
    // Check to decrypt
    auto KeyHash = Hashing::HashXXHashString(Key);
    // Check based on hash
    if (KeyHash == 0x90007daf3980ef1f)    // Password
    {
        // Cache the result
        std::string Result = Value;
        // The password buffer
        const std::string KeyBuffer = "L9VuBReup51wLQ";
        // Cache the size
        auto SizeCache = (uint32_t)KeyBuffer.size();
        // Re-XOR the password
        for (uint32_t i = 0; i < (uint32_t)Result.size(); i++)
        {
            // XOR the char
            Result[i] ^= KeyBuffer[i % SizeCache];
        }
        // Return it
        return Result;
    }

    // Default
    return Value;
}

std::string SettingsManager::UnModifyValue(const std::string& Key, const std::string& Value)
{
    // Check to encrypt
    auto KeyHash = Hashing::HashXXHashString(Key);
    // Check based on hash
    if (KeyHash == 0x90007daf3980ef1f)    // Password
    {
        // Cache the result
        std::string Result = Value;
        // The password buffer
        const std::string KeyBuffer = "L9VuBReup51wLQ";
        // Cache the size
        auto SizeCache = (uint32_t)KeyBuffer.size();
        // Re-XOR the password
        for (uint32_t i = 0; i < (uint32_t)Result.size(); i++)
        {
            // XOR the char
            Result[i] ^= KeyBuffer[i % SizeCache];
        }
        // Return it
        return Result;
    }

    // Default
    return Value;
}