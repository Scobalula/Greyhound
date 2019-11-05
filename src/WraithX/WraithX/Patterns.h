#pragma once

#include <string>
#include <sstream>
#include <cstdint>

// We require the strings utility
#include "Strings.h"

class Patterns
{
public:
    // Transform a text-based pattern to a pattern + mask
    static void ProcessPattern(const std::string& Pattern, std::string& Data, std::string& Mask);
    // Scan a block of memory for a given pattern
    static int64_t ScanBlock(const std::string& Data, const std::string& Mask, uint64_t BlockBegin, uint64_t BlockEnd);

    template <class T>
    // Transforms a data type to a pattern
    static std::string ToPattern(T Value)
    {
        // Pattern
        std::stringstream PatternBuilder;
        // Convert string to pattern
        const char* StringData = nullptr; 
        // Get length
        size_t Length = 0;
        // Check
        if (std::is_same<T, const char*>::value)
        {
            // Set
            StringData = reinterpret_cast<const char*>(Value);
            // We can use strlen
            Length = strlen(StringData);
        }
        else
        {
            // We can use sizeof
            Length = sizeof(Value);
            // Set
            StringData = reinterpret_cast<const char*>(&Value);
        }
        // Loop
        for (size_t i = 0; i < Length; i++)
        {
            // Format it
            PatternBuilder << std::hex << (int)StringData[i] << " ";
        }
        // Return it (trimmed though)
        return Strings::Trim(PatternBuilder.str());
    }
};