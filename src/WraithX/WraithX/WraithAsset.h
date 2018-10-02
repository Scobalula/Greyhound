#pragma once

#include <cstdint>
#include <string>

// A list of available asset types
enum class WraithAssetType
{
	// An animation asset
	Animation,
	// An image asset
	Image,
	// A model asset
	Model,
	// A sound file asset
	Sound,
	// A fx file asset
	Effect,
	// A raw file asset
	RawFile,
	// A custom asset
	Custom,
	// An unknown asset, not loaded
	Unknown,
};

enum class WraithAssetStatus
{
	// The asset is currently loaded
	Loaded,
	// The asset was exported
	Exported,
	// The asset was not loaded
	NotLoaded,
	// The asset is a placeholder
	Placeholder,
	// The asset is being processed
	Processing,
	// The asset had an error
	Error
};

// A class that represents a WraithAsset to be shown in the asset list
class WraithAsset
{
public:
	// Create a new WraithAsset
	WraithAsset();
	// Cleanup of a WraithAsset, ensure virtual to call the destructor of the new one
	virtual ~WraithAsset();

	// The type of asset we have
	WraithAssetType AssetType;
	// The name of this asset
	std::string AssetName;
	// The status of this asset
	WraithAssetStatus AssetStatus;
	// The size of this asset, -1 = N/A
	int64_t AssetSize;
};