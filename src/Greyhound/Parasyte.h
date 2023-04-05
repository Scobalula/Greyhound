#pragma once
#include <vector>
#include <stdint.h>
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <fstream>

// Parasyte Helpers.
namespace ps
{
    // A class to hold Parasyte State Information
    class State
    {
    public:
        // Current Game Directory
        std::string GameDirectory;
        // Current Game ID
        uint64_t GameID;
        // Address of Asset Pools
        uint64_t PoolsAddress;
        // Address of Strings
        uint64_t StringsAddress;
        // The game specific flags as a string array.
        std::vector<std::string> Flags;

        // Creates a new state.
        State();

        // Loads the context from the provided file.
        bool Load(const std::string& path);
        // Adds a game specific flag to this handler.
        void AddFlag(const std::string& flag);
        // Checks if the provided game specific flag is available.
        bool HasFlag(const std::string& flag);
    };

    // Parasytes XAsset Structure.
    struct XAsset64
    {
        // The previous xasset in the list.
        uint64_t Previous;
        // The next xasset in the list.
        uint64_t Next;
        // First child asset we have overriden.
        uint64_t FirstChild;
        // Last child asset we have overriden.
        uint64_t LastChild;
        // The asset header.
        uint64_t Header;
        // The fast file that owns this asset.
        uint64_t Owner;
        // Header Memory.
        uint64_t HeaderMemory;
        // The extended data, if any.
        uint64_t ExtendedData;
        // The pointer that points to the extended data.
        uint64_t ExtendedDataPtrOffset;
        // The size of the extended data.
        uint64_t ExtendedDataSize;
        // The xasset type depending on the game it is from.
        uint64_t Type;
        // The xasset size.
        uint64_t HeaderSize;
        // The xasset ID.
        uint64_t ID;
        // Whether or not this asset is a tempt slot.
        uint64_t Temp;
    };

    // Parasytes XAsset Pool Structure.
    struct XAssetPool64
    {
        // The start of the asset chain.
        uint64_t FirstXAsset;
        // The end of the asset chain.
        uint64_t LastXAsset;
        // The asset hash table for this pool.
        uint64_t LookupTable;
        // Storage for asset headers for this pool.
        uint64_t HeaderMemory;
    };

    // Current State Information
    extern std::unique_ptr<State> state;

    // Parse the pool, givin the current offset and asset type
    void PoolParser64(uint64_t offset, std::function<XAsset64(const uint64_t&)> request, std::function<void(XAsset64&)> callback);
};

