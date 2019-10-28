#include "stdafx.h"

// The class we are implementing
#include "IPAKSupport.h"

// We need the following WraithX classes
#include "MemoryReader.h"
#include "BinaryReader.h"
#include "Strings.h"
#include "SettingsManager.h"
#include "WraithNameIndex.h"
#include "FileSystems.h"

// We need the file classes
#include "CoDAssets.h"
#include "CoDIWITranslator.h"
#include "DBGameFiles.h"

bool IPAKSupport::ParseIPAK(const std::string& FilePath)
{
    // Prepare to load the index list from an IPAK (Images only)
    auto Reader = BinaryReader();
    // Open it
    Reader.Open(FilePath);

    // Read the header
    auto Header = Reader.Read<BO2IPakHeader>();

    // Verify the magic
    if (Header.Magic == 0x4950414b)
    {
        // We have a valid file, load the bo2_ipak name database
        WraithNameIndex IPAKNames(FileSystems::CombinePath(FileSystems::GetApplicationPath(), "package_index\\bo2_ipak.wni"));
        
        // Prepare to read segments
        auto IPAKEntriesSegment = BO2IPakSegment();
        auto IPAKDataSegment = BO2IPakSegment();

        // Read segments until we find the info one
        for (uint32_t i = 0; i < Header.SegmentCount; i++)
        {
            // Read segment
            auto Segment = Reader.Read<BO2IPakSegment>();

            // Check for the info one, then break if found
            if (Segment.Type == 1) { IPAKEntriesSegment = Segment; }
            if (Segment.Type == 2) { IPAKDataSegment = Segment; }
        }

        // Jump to the offset, if we are sure it's right
        if (IPAKEntriesSegment.Type == 1)
        {
            // Hop to the offset
            Reader.SetPosition(IPAKEntriesSegment.Offset);

            // Loop and read entries
            for (uint32_t i = 0; i < IPAKEntriesSegment.EntryCount; i++)
            {
                // Read entry
                auto Entry = Reader.Read<BO2IPakEntry>();

                // The entry name
                std::string AssetName = "";
                // Attempt to find the name
                if (IPAKNames.NameDatabase.find(Entry.Key) != IPAKNames.NameDatabase.end())
                {
                    // Set
                    AssetName = IPAKNames.NameDatabase[Entry.Key];
                }
                else
                {
                    // Format from key
                    AssetName = Strings::Format("_%llx", Entry.Key);
                }

                // Make and add
                auto LoadedImage = new CoDImage_t();
                // Set
                LoadedImage->AssetName = AssetName;
                LoadedImage->AssetPointer = Entry.Key;
                LoadedImage->Width = 0;
                LoadedImage->Height = 0;
                LoadedImage->AssetStatus = WraithAssetStatus::Loaded;
                // File entry mode
                LoadedImage->IsFileEntry = true;

                // Add
                CoDAssets::GameAssets->LoadedAssets.push_back(LoadedImage);
            }
        }
    }
    else
    {
        // Failed, invalid
        return false;
    }

    // Success unless otherwise
    return true;
}

std::unique_ptr<XImageDDS> IPAKSupport::ReadImageFile(const CoDImage_t* Image)
{
    // Resulting buffer
    uint32_t ResultSize = 0;
    // We have a streamed image, prepare to extract (AssetPointer = Image Key)
    auto ImageData = CoDAssets::GamePackageCache->ExtractPackageObject(Image->AssetPointer, ResultSize);

    // Prepare if we have it
    if (ImageData != nullptr)
    {
        // Prepare to create a MemoryDDS file
        auto Result = CoDIWITranslator::TranslateIWI(ImageData, ResultSize);

        // Check for, and apply patch if required, if we got a raw result
        if (Result != nullptr && (SettingsManager::GetSetting("patchnormals", "true") == "true"))
        {
            // We can check the name here for _n and _nml
            if (Strings::EndsWith(Image->AssetName, "_n") || Strings::EndsWith(Image->AssetName, "_nml") | Strings::EndsWith(Image->AssetName, "_n2") || Strings::EndsWith(Image->AssetName, "_n3"))
            {
                // This is a normal map
                Result->ImagePatchType = ImagePatch::Normal_Expand;
            }
        }

        // Return it
        return Result;
    }

    // Failed
    return nullptr;
}