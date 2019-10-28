#pragma once

#include <cstdint>
#include <functional>

// We need the following classes
#include "CoDAssets.h"
#include "CoDAssetType.h"

// We need the following WraithX classes
#include "MemoryReader.h"

template<typename T, typename A>
// Parse the pool, givin the current offset and asset type
void CoDXPoolParser(T PoolOffset, uint32_t PoolSize, std::function<void(A&, T&)> OnAssetParsed)
{
    // Enforce no structure packing
#pragma pack(push, 1)
    union XAssetBase
    {
        T NextHeaderPtr;
        A Base;
    };
#pragma pack(pop)

    // Buffer for size
    uintptr_t ResultSize = 0;

    // We will read the entire memory structure to a buffer before continuing
    auto ResultBuffer = CoDAssets::GameInstance->Read(PoolOffset, sizeof(A) * PoolSize, ResultSize);
    auto Reader = MemoryReader(ResultBuffer, ResultSize);

    // Calculate maximum pool offset
    auto MaximumPoolOffset = (PoolSize * sizeof(A)) + PoolOffset;

    // Loop and check if each asset is valid, if so, perform callback
    for (uint32_t i = 0; i < PoolSize; i++)
    {
        // Grab the offset
        T AssetOffset = (T)(Reader.GetPosition() + PoolOffset);

        // Grab the asset
        auto Asset = Reader.Read<A>();

        // Compare the next header pointer to the maximum pool offset, whichever comes first
        if (((XAssetBase*)&Asset)->NextHeaderPtr > PoolOffset && ((XAssetBase*)&Asset)->NextHeaderPtr < MaximumPoolOffset || Asset.NamePtr == 0)
        {
            // Skip over this asset
            continue;
        }

        // We should process this asset
        OnAssetParsed(Asset, AssetOffset);
    }
}