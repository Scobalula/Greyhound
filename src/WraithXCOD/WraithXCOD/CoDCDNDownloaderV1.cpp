#include "stdafx.h"

// The class we are implementing
#include "CoDCDNDownloaderV1.h"

// We need the following classes.
#include "FileSystems.h"

// We need the file system classes.
#include "CoDFileHandle.h"
#include "CoDFileSystem.h"
#include "CascFileSystem.h"
#include "WinFileSystem.h"

// We need the XSUB Classes
#include "VGXSUBCache.h"
#include "XSUBCache.h"

// We need web class
#include "WebClient.h"

// The CDN URL for V2 titles.
constexpr auto CoDV1CDNURL = "http://cod-assets.cdn.blizzard.com/pc/s4_1";

bool CoDCDNDownloaderV1::Initialize(const std::string& gameDirectory)
{
	// Verify the CDN is available.
	if (Client.DownloadData(CoDV1CDNURL) == nullptr)
		return false;

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

	// Load cache
	Cache.Load("CoDCDNV1");

	return true;
}

bool CoDCDNDownloaderV1::LoadCDNXPAK(const std::string& fileName)
{
	// Open CASC File
	auto handle = CoDFileHandle(FileSystem->OpenFile(fileName, "r"), FileSystem.get());
	auto header = handle.Read<VGXSUBHeader>();

	// If we have a hash offset outside file, we not doing too good.
	if (header.Magic != 0x4950414b && (uint64_t)header.HashOffset >= handle.Size())
		return false;

	handle.Seek(header.HashOffset, SEEK_SET);

	for (int64_t i = 0; i < header.HashCount; i++)
	{
		// Read it
		auto packedEntry = handle.Read<VGXSUBHashEntry>();

		CoDCDNDownloaderV1Entry entry{};

		entry.Hash = packedEntry.Key;
		entry.Size = (packedEntry.PackedInfo >> 1) & 0x3FFFFFFF;
		entry.Flags = packedEntry.PackedInfo & 1;

		Entries[entry.Hash] = entry;
	}

	return true;
}

std::unique_ptr<uint8_t[]> CoDCDNDownloaderV1::ExtractCDNObject(uint64_t cacheID, size_t sizeOfBuffer, size_t& resultSize)
{
	auto potentialEntry = Entries.find(cacheID);

	if (potentialEntry == Entries.end())
		return nullptr;

	// Aquire lock
	std::lock_guard<std::shared_mutex> Gaurd(Mutex);

	auto entry = potentialEntry->second;

	// Attempt to extract it first from the CDN cache, to avoid constantly requesting from
	// the remote CDN.
	size_t bufferSize = 0;
	auto cdnBuffer = Cache.Extract(cacheID, entry.Size, bufferSize);

	if (cdnBuffer != nullptr)
	{
		auto decompressed = VGXSUBCache::DecompressPackageObject(
			entry.Hash,
			cdnBuffer.get(),
			bufferSize,
			sizeOfBuffer,
			resultSize);

		if (decompressed != nullptr)
		{
			return decompressed;
		}

	}

	// Fallback to CDN.
	// If we can't extract it (i.e. previous attempt failed) then we should wait before attempting again.
	if (HasFailed(cacheID))
	{
		return nullptr;
	}

	auto url = Strings::Format("%s/21/%02x/%016llx_%08llx_%s", CoDV1CDNURL, (uint8_t)entry.Hash, entry.Hash, entry.Size, entry.Flags ? "1" : "0");
	auto result = Client.DownloadData(url);

	if (result == nullptr || result->BufferSize != entry.Size)
	{
		// Append a failed attempt, we won't try to export this again until a load game attempt or time has passed.
		AddFailed(cacheID);
		return nullptr;
	}

	// Add to the cache
	Cache.Add(cacheID, result->DataBuffer, result->BufferSize);

	return VGXSUBCache::DecompressPackageObject(
		entry.Hash,
		result->DataBuffer,
		result->BufferSize,
		sizeOfBuffer,
		resultSize);
}
