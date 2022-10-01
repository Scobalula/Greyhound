#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>

// A class that handles reading and writing settings
class SettingsManager
{
private:
    // The settings cache
    static std::unordered_map<std::string, std::string> SettingsCache;
    // The settings file name
    static std::string SettingsFileName;

    // -- Functions
    
    // Decrypt value types
    static std::string ModifyValue(const std::string& Key, const std::string& Value);
    // Encrypt value types
    static std::string UnModifyValue(const std::string& Key, const std::string& Value);

public:
    // -- Functions

    // Loads the settings file from the disk
    static void LoadSettings(const std::string& SettingsName, const std::map<std::string, std::string>& Defaults);
    // Saves the settings file to the disk, must have loaded first
    static void SaveSettings();

    // Gets a setting from the cache
    static std::string GetSetting(const std::string& Key, const std::string& Default = "");
    // Sets a setting from the cache
    static void SetSetting(const std::string& Key, const std::string& Value);
};