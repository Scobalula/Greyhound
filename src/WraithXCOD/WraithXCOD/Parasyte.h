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
        // The asset header.
        uint64_t Header;
        // Whether or not this asset is a temp slot.
        uint64_t Temp;
        // The next xasset in the list.
        uint64_t Next;
        // The previous xasset in the list.
        uint64_t Previous;
    };

    // Parasytes XAsset Pool Structure.
    struct XAssetPool64
    {
        // The start of the asset chain.
        uint64_t Root;
        // The end of the asset chain.
        uint64_t End;
        // The asset hash table for this pool.
        uint64_t LookupTable;
        // Storage for asset headers for this pool.
        uint64_t HeaderMemory;
        // Storage for asset entries for this pool.
        uint64_t AssetMemory;
    };

    // Current State Information
    extern std::unique_ptr<State> state;

    // Parse the pool, givin the current offset and asset type
    void PoolParser64(uint64_t offset, std::function<XAsset64(const uint64_t&)> request, std::function<void(XAsset64&)> callback);
};

