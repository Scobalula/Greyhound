#pragma once

#include <string>
#include <cstdint>

// We need to include external libraries for animations
#include "WraithAnim.h"

// The file format version to write
enum class XAnimRawVersion : uint16_t
{
	WorldAtWar = 17,
	BlackOps = 19
};

// A class that handles writing XAnim animation files
class XAnimRaw
{
public:
	// Export a WraithAnim to a XAnim file
	static void ExportXAnimRaw(const WraithAnim& Animation, const std::string& FileName, XAnimRawVersion Version = XAnimRawVersion::WorldAtWar);
};