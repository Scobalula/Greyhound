#pragma once

#include <string>
#include <cstdint>
#include <mutex>
#include <vector>
#include <unordered_map>

// A structure that represents a package cache object, this is generic and is used in all of the assets
struct PackageCacheObject
{
    // The data offset of this object
    uint64_t Offset;
    // The size of this object
    uint64_t CompressedSize;
    // The size of this object uncompressed
    uint64_t UncompressedSize;

    // The index of the package file used for this
    uint32_t PackageFileIndex;
};

// A class that handles reading and indexing game package files for later extraction of data
class CoDPackageCache
{
public:
    // Constructor
    CoDPackageCache();
    virtual ~CoDPackageCache();

    // -- Properties

    // Gets whether or not the package was loaded
    bool HasCacheLoaded();
    // Gets whether or not the package is loading
    bool IsCacheLoading();

    // -- Load function

    // Loads the package cache, with the given path, in async
    void LoadPackageCacheAsync(const std::string& BasePath);
    // Loads the package cache file, with the given path, in async
    void LoadPackageAsync(const std::string& FilePath);

    // Waits for the package cache to load, blocking the current thread
    void WaitForPackageCacheLoad();

    // Returns a cache object (nullptr if not found)
    virtual std::unique_ptr<uint8_t[]> ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize) { return nullptr; }

    // Hashes a cache object id if needed
    virtual uint64_t HashPackageID(const std::string& Value) { return 0; }

    // Returns the current packages path
    std::string GetPackagesPath() { return PackageFilesPath; }

    // Sets the cache state to loaded
    virtual void SetLoadedState();

protected:

    // Loads the package cache (This should be called from LoadPackageCacheAsync only!)
    virtual void LoadPackageCache(const std::string& BasePath);
    // Loads the package file
    virtual bool LoadPackage(const std::string& FilePath);    

    // -- Cache data

    // A list of cached objects in this package cache
    std::unordered_map<uint64_t, PackageCacheObject> CacheObjects;
    // A list of cached package file paths
    std::vector<std::string> PackageFilePaths;

    // A path of packages, used for games that don't require a cache
    std::string PackageFilesPath;

private:

    // -- Cache read data

    // A mutex for read operations
    std::mutex CacheMutex;

    // Whether or not this package is loaded
    bool HasLoaded;
    // Whether or not this package is loading
    bool CacheLoading;
};