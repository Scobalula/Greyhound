#include "stdafx.h"

// The class we are implementing
#include "CoDCDNDownloaderV2.h"

// We need the following classes.
#include "FileSystems.h"

// We need the file system classes.
#include "CoDFileHandle.h"
#include "CoDFileSystem.h"
#include "CascFileSystem.h"
#include "WinFileSystem.h"

// We need the XSUB Classes
#include "XSUBCache.h"
#include "XSUBCacheV2.h"

// We need web class
#include "WebClient.h"

// The CDN URL for V2 titles.
constexpr auto CoDV2CDNURL = "http://cod-assets.cdn.blizzard.com/pc/iw9_2";

bool CoDCDNDownloaderV2::Initialize(const std::string& gameDirectory)
{
	// Check for Casc
	if (FileSystems::FileExists(gameDirectory + "\\.build.info"))
	{
		FileSystem = std::make_unique<CascFileSystem>(gameDirectory);
	}
	else
	{
		FileSystem = std::make_unique<WinFileSystem>(gameDirectory);
	}

	// Verify the directory opened, for Casc this is ensuring
	// we opened storage, for Windows, it's ensuring the
	// directory exists.
	if (!FileSystem->IsValid())
	{
		return false;
	}

	// Enumerate all cdn files.
	FileSystem->EnumerateFiles("*cdn.xpak", [this](const std::string& fileName, const size_t size)
	{
		this->LoadCDNXPAK(fileName);
	});

	return true;
}

bool CoDCDNDownloaderV2::LoadCDNXPAK(const std::string& fileName)
{
	// Open CASC File
	auto handle = CoDFileHandle(FileSystem->OpenFile(fileName, "r"), FileSystem.get());
	auto header = handle.Read<XSUBHeaderV2>();

	// If we have a hash offset outside file, we not doing too good.
	if (header.Magic != 0x4950414b && header.HashOffset >= handle.Size())
		return false;

	handle.Seek(header.HashOffset, SEEK_SET);

	for (uint64_t i = 0; i < header.HashCount; i++)
	{
		// Read it
		auto packedEntry = handle.Read<XSUBHashEntryV2>();

		CoDCDNDownloaderV2Entry entry{};

		entry.Hash = packedEntry.Key;
		entry.Size = (packedEntry.PackedInfo >> 1) & 0x3FFFFFFF;
		entry.Flags = packedEntry.PackedInfo & 1;

		Entries[entry.Hash] = entry;
	}

	return true;
}

std::unique_ptr<uint8_t[]> CoDCDNDownloaderV2::ExtractCDNObject(uint64_t cacheID, size_t sizeOfBuffer, size_t& resultSize)
{
	auto potentialEntry = Entries.find(cacheID);

	if (potentialEntry == Entries.end())
		return nullptr;

	auto entry = potentialEntry->second;
	auto url = Strings::Format("%s/22/%02x/%016llx_%08llx_%s", CoDV2CDNURL, (uint8_t)entry.Hash, entry.Hash, entry.Size, entry.Flags ? "1" : "0");
	auto result = Client.DownloadData(url);

	printf("Sacrifice Ploof\n");

	return nullptr;
}
