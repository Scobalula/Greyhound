#pragma once

struct CoDCDNCacheEntry
{
	// The hash of the object stored.
	uint64_t Hash;
	// The offset within the data file.
	uint64_t Offset;
	// The size of the object 
	uint64_t Size;
};

// A class to hold a generic CDN Cache.
class CoDCDNCache
{
private:
	// A mutex for read/write operations.
	std::mutex Mutex;
	// The full path of the CDN info file.
	std::string InfoFilePath;
	// The full path of the CDN data file.
	std::string DataFilePath;
	// The cached entries.
	std::map<uint64_t, CoDCDNCacheEntry> Entries;
	// Whether or not the cache has loaded.
	bool Loaded;
public:
	// Initializes the CDN Cache.
	CoDCDNCache();
	// Loads the CDN cache info file.
	bool Load(const std::string& name);
	// Saves the CDN cache info file.
	bool Save();
	// Adds an object to the CDN cache.
	bool Add(const uint64_t hash, const uint8_t* buffer, const size_t bufferSize);
	// Extracts an object from the CDN cache.
	std::unique_ptr<uint8_t[]> Extract(const uint64_t hash, uint32_t expectedSize, size_t& bufferSize);
};

