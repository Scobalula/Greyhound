#pragma once

#include <cstdint>
#include <memory>

// We need the asset type
#include "WraithModel.h"

// We need the following classes
#include "BinaryWriter.h"

// A class that handles processing GDT data
class CoDGDTProcessor
{
private:
	// The current cache file instance
	BinaryWriter CacheHandle;
	// The current cache file name
	std::string CacheFileName;

public:
	CoDGDTProcessor();
	~CoDGDTProcessor();

	// Sets up this GDT processor instance
	void SetupProcessor(const std::string& GameShorthand);

	// Reloads from the last name
	void ReloadProcessor();

	// Close the processor
	void CloseProcessor();

	// Process a model's GDT info (Adds a model to the GDT cache file)
	void ProcessModelGDT(const std::unique_ptr<WraithModel>& Model);

	// Exports game GDTs for the current cache entries
	void ExportGameGDTs();
};