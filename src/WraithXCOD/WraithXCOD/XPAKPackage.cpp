#include "stdafx.h"
#include "DBGameFiles.h"
#include "XPAKPackage.h"

XPAKPackage::XPAKPackage(CoDFileSystem* fileSystem, const std::string& path) : CoDPackage(fileSystem, path)
{
}

void XPAKPackage::Load()
{
    // Read the header
    auto Header = FileHandle.Read<BO3XPakHeader>();

    // Verify the magic
    if (Header.Magic != 0x4950414b)
        throw std::exception("Invalid XPAK Magic Number");

    // Jump to hash offset
    FileHandle.Seek(Header.HashOffset, SEEK_SET);

    std::set<uint64_t> IncludedHashes;

    for (uint64_t i = 0; i < Header.HashCount; i++)
        IncludedHashes.insert(FileHandle.Read<BO3XPakHashEntry>().Key);

    // A buffer for our properties
    char Props[8192]{};

    // Loop and setup entries
    for (uint64_t i = 0; i < Header.IndexCount; i++)
    {
        // Read hash and properties size
        auto Hash = FileHandle.Read<uint64_t>();
        auto Properties = FileHandle.Read<uint64_t>();

        // Ensure in map
        if (IncludedHashes.find(Hash) == IncludedHashes.end())
        {
            // Skip properties
            FileHandle.Seek(Properties, SEEK_CUR);
            continue;
        }
        // Ensure our buffer is able to fufill
        if (Properties > sizeof(Props))
            throw std::exception("Unable to parse properties, our buffer is too small for the properties.");

        // Result
        uint64_t ReadSize = 0;
        // Read buffer and parse
        FileHandle.Read((uint8_t*)Props, 0, Properties);

        // Our buffers for payloads
        std::string key;
        std::string val;
        bool keyMode = true;

        std::map<std::string, std::string> parsedProps;

        for (size_t c = 0; c < Properties; c++)
        {
            if (Props[c] == '\n')
            {
                parsedProps[key] = val;
                key.clear();
                val.clear();
                keyMode = true;
                continue;
            }
            if (Props[c] == ' ')
            {
                continue;
            }
            if (Props[c] == ':')
            {
                keyMode = false;
                continue;
            }

            if (keyMode)
            {
                key += Props[c];
            }
            else
            {
                val += Props[c];
            }
        }

        Items[Hash] = std::make_unique<CoDPackageItem>(parsedProps["name"]);
    }
}