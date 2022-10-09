#include "stdafx.h"

// The class we are implementing
#include "CoDCDNDownloaderV0.h"

// We need the following classes.
#include "FileSystems.h"
#include "Strings.h"

// We need the file system classes.
#include "CoDFileHandle.h"
#include "CoDFileSystem.h"
#include "CascFileSystem.h"
#include "WinFileSystem.h"

// We need the XPAK Class
#include "XPAKCache.h"

// We need web class
#include "WebClient.h"

// The CDN URL for V2 titles.
constexpr auto CoDV0CDNURL = "http://cod-assets.cdn.blizzard.com/pc/0";

bool CoDCDNDownloaderV0::Initialize(const std::string& gameDirectory)
{
	// Verify the CDN is available.
	if (Client.DownloadData(CoDV0CDNURL) == nullptr)
		return false;

	// Load cache
	Cache.Load("CoDCDNV0");

	return true;
}

std::unique_ptr<uint8_t[]> CoDCDNDownloaderV0::ExtractCDNObject(uint64_t cacheID, size_t sizeOfBuffer, size_t& resultSize)
{
	// Aquire lock
	std::lock_guard<std::shared_mutex> Gaurd(Mutex);

	// Attempt to extract it first from the CDN cache, to avoid constantly requesting from
	// the remote CDN. For V0, we won't know the size of the cached item.
	size_t bufferSize = 0;
	auto cdnBuffer = Cache.Extract(cacheID, 0, bufferSize);

	if (cdnBuffer != nullptr)
	{
		// V0 stores metadata right before the image, so must skip it.
		auto decompressed = XPAKCache::DecompressPackageObject(
			cacheID,
			cdnBuffer.get() + *(uint16_t*)cdnBuffer.get() + 2,
			bufferSize - *(uint16_t*)cdnBuffer.get() - 2,
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

	auto url = Strings::Format("%s/%016llx", CoDV0CDNURL, cacheID);
	auto result = Client.DownloadData(url);

	if (result == nullptr)
	{
		// Append a failed attempt, we won't try to export this again until a load game attempt or time has passed.
		AddFailed(cacheID);
		return nullptr;
	}

	// Add to the cache
	Cache.Add(cacheID, result->DataBuffer, result->BufferSize);

	return XPAKCache::DecompressPackageObject(
		cacheID,
		result->DataBuffer + *(uint16_t*)result->DataBuffer + 2,
		result->BufferSize - *(uint16_t*)result->DataBuffer - 2,
		sizeOfBuffer,
		resultSize);
}
