#pragma once
#include "CoDCDNCache.h"
#include "CoDFileSystem.h"
#include "WebClient.h"

class CoDCDNDownloader
{
protected:
    // The cache of objects downloaded from the server.
    CoDCDNCache Cache;
    // The File System being used.
    std::unique_ptr<CoDFileSystem> FileSystem;
    // A map of cache IDs and their last attempt for failures.
    std::map<uint64_t, std::chrono::system_clock::time_point> FailedAttempts;
    // The web client used for downloading.
    WebClient Client;
public:
    // Deinitializes the CDN downloader.
    virtual ~CoDCDNDownloader();
    // Initializes the CDN downloader.
    virtual bool Initialize(const std::string& gameDirectory) { return true; };
    // Adds a failed item to the list, if an item has failed, we attempt for another space of time.
    virtual void AddFailed(const uint64_t cacheID);
    // Decides whether or not this ID has failed before.
    virtual bool HasFailed(const uint64_t cacheID);
    // Returns a cache object (nullptr if not found)
    virtual std::unique_ptr<uint8_t[]> ExtractCDNObject(uint64_t cacheID, size_t sizeOfBuffer, size_t& ResultSize) { return nullptr; }
};
