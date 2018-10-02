#include "stdafx.h"

// The class we are implementing
#include "Patterns.h"

void Patterns::ProcessPattern(const std::string& Pattern, std::string& Data, std::string& Mask)
{
	// Buffers for the data
	std::stringstream DataStr;
	std::stringstream MaskStr;

	// Buffers for temporary processing flags
	uint8_t TempDigit = 0;
	bool TempFlag = false;
	bool LastWasUnknown = false;

	// Iterate over all bytes
	for (auto & ch : Pattern)
	{
		// If it's a space, just skip it
		if (ch == ' ')
		{
			// Reset
			LastWasUnknown = false;
			// Skip
			continue;
		}
		else if (ch == '?')
		{
			// This is an unknown instance
			if (LastWasUnknown)
			{
				// This is second one, just disable
				LastWasUnknown = false;
			}
			else
			{
				// Append mask
				DataStr << '\x00';
				MaskStr << '?';
				// Set it
				LastWasUnknown = true;
			}
		}
		else if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))
		{
			// This is a hex value
			char StrBuffer[] = { ch, 0 };
			// Convert to digit
			int thisDigit = strtol(StrBuffer, nullptr, 16);

			// Check if we need the second digit
			if (!TempFlag)
			{
				// We do
				TempDigit = (thisDigit << 4);
				TempFlag = true;
			}
			else
			{
				// This is the second digit, process
				TempDigit |= thisDigit;
				TempFlag = false;

				// Append data to mask and data string
				DataStr << TempDigit;
				MaskStr << 'x';
			}
			// Reset
			LastWasUnknown = false;
		}
	}

	// Set both results before returning
	Data = DataStr.str();
	Mask = MaskStr.str();
}

int64_t Patterns::ScanBlock(const std::string& Data, const std::string& Mask, uint64_t BlockBegin, uint64_t BlockEnd)
{
	// Scan a block of memory for the given pattern, check for SSE4.2
	bool UseSSE = false;
	// Check it
	{
		// The CPU info buffer
		int cpuid[4];
		// Fetch it
		__cpuid(cpuid, 0);
		// Check if we can even compare the mask
		if (Mask.size() <= 16)
		{
			// Check for support
			if (cpuid[0] >= 1)
			{
				__cpuidex(cpuid, 1, 0);
				// Whether or not we have support for it
				UseSSE = ((cpuid[2] & (1 << 20)) > 0);
			}
		}
	}

	// If we can't use SSE just check each byte
	if (!UseSSE)
	{
		// Convert
		const char* PatternData = Data.c_str();
		const char* MaskData = Mask.c_str();

		// Data to search
		char* DataPtr = reinterpret_cast<char*>(BlockBegin);

		// Check each
		for (uint64_t i = 0; i < BlockEnd; i++)
		{
			// If we found it
			bool IsMatch = true;
			// Check for a match, if success, return it
			for (size_t c = 0; c < Data.size(); c++)
			{
				// Check
				if (Mask[c] == '?')
				{
					// Skip
					continue;
				}

				// Check the data
				if (PatternData[c] != DataPtr[i + c])
				{
					// Not match
					IsMatch = false;
					// Stop
					break;
				}
			}
			// Check
			if (IsMatch)
			{
				// Return result
				return (int64_t)(i);
			}
		}
	}
	else
	{
		// We can use SSE to speed this up
		__declspec(align(16)) char DesiredMask[16] = { 0 };

		// Build the mask
		for (size_t i = 0; i < Mask.size(); i++)
		{
			DesiredMask[i / 8] |= ((Mask[i] == '?') ? 0 : 1) << (i % 8);
		}

		// Load the mask and the data
		__m128i mask = _mm_load_si128(reinterpret_cast<const __m128i*>(DesiredMask));
		__m128i comparand = _mm_loadu_si128(reinterpret_cast<const __m128i*>(Data.c_str()));

		// Loop and compare data in up to 16 byte chunks (SSE4.2)
		for (uint64_t i = BlockBegin; i <= (BlockBegin + BlockEnd); i++)
		{
			// Compare
			__m128i value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(i));
			__m128i result = _mm_cmpestrm(value, 16, comparand, (int)Data.size(), _SIDD_CMP_EQUAL_EACH);

			// See if we can match with the mask
			__m128i matches = _mm_and_si128(mask, result);
			__m128i equivalence = _mm_xor_si128(mask, matches);

			// Test the result
			if (_mm_test_all_zeros(equivalence, equivalence))
			{
				// We got a result here return it
				return (int64_t)(i - BlockBegin);
			}
		}
	}

	// Failed to find one
	return -1;
}