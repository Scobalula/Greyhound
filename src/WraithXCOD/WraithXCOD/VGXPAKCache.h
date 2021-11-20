#pragma once
// We need the package cache
#include "CoDAssets.h"
#include "CoDPackageCache.h"
#include "Casc.h"
#include <shared_mutex>

// A class that handles reading, caching and extracting CASC Resources
class VGXPAKCache : public CoDPackageCache
{
public:
    // Constructors
    VGXPAKCache();
    virtual ~VGXPAKCache();

    // Implement the load function
    virtual void LoadPackageCache(const std::string& BasePath);
    // Implement the load package
    virtual bool LoadPackage(const std::string& FilePath);
    // Implement the extract function
    virtual std::unique_ptr<uint8_t[]> ExtractPackageObject(uint64_t CacheID, int32_t Size, uint32_t& ResultSize);
};

#pragma pack(push, 1)
struct VGXPAKBlock
{
    uint8_t Compression;
    uint32_t CompressedSize;
    uint32_t DecompressedSize;
    uint32_t BlockOffset;
    uint32_t DecompressedOffset;
    uint32_t Unknown;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct VGXPAKHeader
{
    uint32_t Magic;
    uint16_t Unknown1;
    uint16_t Version;
    uint64_t Unknown;
    uint64_t Type;
    uint64_t Size;
    uint8_t UnknownHashes[1896];
    int64_t FileCount;
    int64_t DataOffset;
    int64_t DataSize;
    int64_t HashCount;
    int64_t HashOffset;
    int64_t HashSize;
    int64_t Unknown3;
    int64_t UnknownOffset;
    int64_t Unknown4;
    int64_t IndexCount;
    int64_t IndexOffset;
    int64_t IndexSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct VGXPAKHashEntry
{
    uint64_t Key;
    uint64_t PackedInfo;
};
#pragma pack(pop)

// Verify
static_assert(sizeof(VGXPAKBlock) == 0x15, "Invalid Vanguard Block Struct Size (Expected 0x15)");