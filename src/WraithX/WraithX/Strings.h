#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

class Strings
{
public:
	// Trim a string from the start
	static std::string& StartTrim(std::string& Value);
	// Trim a string from the end
	static std::string& EndTrim(std::string& Value);
	// Trim from both ends
	static std::string& Trim(std::string& Value);

	// Format a string with the given args
	static std::string Format(const std::string Format, ...);

	// Convert a string to an integer
	static bool ToInteger(const std::string& Value, int32_t& Result);
	// Convert a string to a long
	static bool ToLong(const std::string& Value, int64_t& Result);
	// Convert a string to a float
	static bool ToFloat(const std::string& Value, float& Result);
	// Convert a string to a double
	static bool ToDouble(const std::string& Value, double& Result);

	// Checks if a string is numbers only
	static bool IsDigits(const std::string& Value);
	// Check if a string is null or whitespace
	static bool IsNullOrWhiteSpace(const std::string& Value);

	// Convert a sequence of bytes to a Base64 string
	static std::string ToBase64String(const std::vector<uint8_t>& Value);
	// Convert a Base64 string to a sequence of bytes
	static std::vector<uint8_t> FromBase64String(const std::string& Value);

	// Replace all instances of a string inside of a string
	static std::string Replace(std::string Subject, const std::string& Search, const std::string& Replace);

	// Splits a string by a char
	static std::vector<std::string> SplitString(const std::string& Search, const char Split, bool StripBlank = false);

	// Checks whether or not a string contains a string
	static bool Contains(const std::string& Subject, const std::string& Search);

	// Checks whether or not a string starts with another string
	static bool StartsWith(const std::string& Subject, const std::string& Beginning);
	// Checks whether or not a string ends with another string
	static bool EndsWith(const std::string& Subject, const std::string& Ending);

	// Converts a string to a lowercase version
	static std::string& ToLower(std::string& Value);
	// Converts a string to an uppercase version
	static std::string& ToUpper(std::string& Value);

	// Converts a wstring to a string
	static std::string ToNormalString(const std::wstring& Value);
	// Converts a string to a wstring
	static std::wstring ToUnicodeString(const std::string& Value);

	// Converts a duration in milliseconds to a readable string
	static std::string DurationToReadableTime(std::chrono::milliseconds Ms);
};