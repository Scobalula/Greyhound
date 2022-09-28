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
    // The web client used for downloading.
    WebClient Client;
public:
    // Initializes the CDN downloader.
    virtual bool Initialize(const std::string& gameDirectory) { return true; };
    // Returns a cache object (nullptr if not found)
    virtual std::unique_ptr<uint8_t[]> ExtractCDNObject(uint64_t cacheID, size_t sizeOfBuffer, size_t& ResultSize) { return nullptr; }
};
