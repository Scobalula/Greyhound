#pragma once
#include "CoDCDNDownloader.h"

// A struct to hold a V1 Entry
struct CoDCDNDownloaderV0Entry
{
    // The stream hash
    uint64_t Hash;
    // The size of the compressed buffer.
    uint64_t Size;
    // The flags assigned to the entry.
    uint64_t Flags;
};

// A class to handle V1 CDNs (Vanguard)
class CoDCDNDownloaderV0 : public CoDCDNDownloader
{
private:
    // Entries stored in the CDN files.
    std::map<uint64_t, CoDCDNDownloaderV0Entry> Entries;
public:
    // Initializes the CDN downloader.
    virtual bool Initialize(const std::string& gameDirectory);
    // Returns a cache object (nullptr if not found)
    virtual std::unique_ptr<uint8_t[]> ExtractCDNObject(uint64_t cacheID, size_t sizeOfBuffer, size_t& ResultSize);
};

