#pragma once

#include <string>
#include <cstdint>

// We need to include external libraries for animations
#include "WraithAnim.h"

// A class that handles writing SEAnim animation files
class SEAnim
{
public:
	// Export a WraithAnim to a SEAnim file
	static void ExportSEAnim(const WraithAnim& Animation, const std::string& FileName);
};