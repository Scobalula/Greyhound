#pragma once

#include <cstdint>
#include <string>
#include <IPHlpApi.h>

// We require the strings utility
#include "Strings.h"

// A class that handles PC authentication, banning, and verification
class Autherization
{
private:
	// -- Calculation functions

	// Calculate the hash of the MAC address
	static uint16_t HashMacAddress(PIP_ADAPTER_INFO Info);
	// Get the hash of the mac address
	static void GetMacHash(uint16_t& Mac1, uint16_t& Mac2);
	// Get the volume serial hash
	static uint16_t GetVolumeHash();
	// Get the CPU hash
	static uint16_t GetCPUHash();
	// Get the machine name
	static std::string GetMachineName();

	// -- Masking functions

	// Mask the ID
	static void MaskID(uint16_t* ID);
	// Unmask the ID
	static void UnMaskID(uint16_t* ID);

	// Mask the computer name
	static void PrepareName(std::string& Name);

public:
	// -- Generation

	// Generate a unique system ID for this PC
	static const std::string ComputeSystemID();

	// -- Anti-Debug

	// Breaks the DbgUiRemoteBreakin callback
	static void EnableAntiDebug();
};