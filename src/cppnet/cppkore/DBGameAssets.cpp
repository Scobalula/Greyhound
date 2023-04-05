#include "stdafx.h"

// The class we are implementing
#include "DBGameAssets.h"

DBGameInfo::DBGameInfo(uint64_t Pools, uint64_t Sizes, uint64_t Strings, uint64_t Package)
{
    // Set values
    DBAssetPools = Pools;
    DBPoolSizes = Sizes;
    StringTable = Strings;
    ImagePackageTable = Package;
}