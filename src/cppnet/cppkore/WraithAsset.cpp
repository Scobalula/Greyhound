#include "stdafx.h"

// The class we are implementing
#include "WraithAsset.h"

WraithAsset::WraithAsset()
{
    // Set defaults
    AssetType = WraithAssetType::Unknown;
    AssetName = "WraithAsset";
    AssetStatus = WraithAssetStatus::NotLoaded;
    AssetSize = -1;
    Streamed = false;
}

WraithAsset::~WraithAsset()
{
    // Default
}