#include "stdafx.h"

// The class we are implementing
#include "Strings.h"
#include <locale>
#include <codecvt>

std::string& Strings::Trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c); }));
    return s;
}

std::string Strings::Format(const std::string Format, ...)
{
    // We must reserve two times as much as the length of the format
    int32_t FinalLength, Length = (int32_t)(Format.size() * 2);
    // A safe buffer of formatted chars
    std::unique_ptr<char[]> Formatted;
    // A list of args
    va_list ArgList;
    // Loop until finished
    while (true)
    {
        // Set it up
        Formatted.reset(new char[Length]);
        // Copy buffer
        strcpy_s(&Formatted[0], Length, Format.c_str());
        // Start
        va_start(ArgList, Format);
        // Set it
#pragma warning (disable:4996)
        FinalLength = vsnprintf(&Formatted[0], Length, Format.c_str(), ArgList);
#pragma warning (default:4996)
        // End
        va_end(ArgList);
        // Calculate
        if (FinalLength < 0 || FinalLength >= Length)
        {
            // Set
            Length += abs(FinalLength - Length + 1);
        }
        else
        {
            // Done
            break;
        }
    }
    // Return result
    return std::string(Formatted.get());
}

bool Strings::ToInteger(const std::string& Value, int32_t& Result)
{
    // The result
    int Parsed = -1;
    // Convert it
    try
    {
        // Convert the result
        Parsed = std::stoi(Value, nullptr);
    }
    catch (...)
    {
        // Nothing, we just failed to parse it
        return false;
    }
    // Set it
    Result = Parsed;
    // Return success
    return true;
}

bool Strings::ToLong(const std::string& Value, int64_t& Result)
{
    // The result
    int64_t Parsed = -1;
    // Convert it
    try
    {
        // Convert the result
        Parsed = std::stol(Value, nullptr);
    }
    catch (...)
    {
        // Nothing, we just failed to parse it
        return false;
    }
    // Set it
    Result = Parsed;
    // Return success
    return true;
}

bool Strings::ToFloat(const std::string& Value, float& Result)
{
    // The result
    float Parsed = -1;
    // Convert it
    try
    {
        // Convert the result
        Parsed = std::stof(Value, nullptr);
    }
    catch (...)
    {
        // Nothing, we just failed to parse it
        return false;
    }
    // Set it
    Result = Parsed;
    // Return success
    return true;
}

bool Strings::ToDouble(const std::string& Value, double& Result)
{
    // The result
    double Parsed = -1;
    // Convert it
    try
    {
        // Convert the result
        Parsed = std::stod(Value, nullptr);
    }
    catch (...)
    {
        // Nothing, we just failed to parse it
        return false;
    }
    // Set it
    Result = Parsed;
    // Return success
    return true;
}

bool Strings::IsDigits(const std::string& Value)
{
    // Search for numbers and return
    return Value.find_first_not_of("0123456789") == std::string::npos;
}

bool Strings::IsNullOrWhiteSpace(const std::string& Value)
{
    // If we're empty we are obviously null
    if (Value.empty()) { return true; }

    // Otherwise, check the chars
    auto it = Value.begin();

    // Loop until end
    do
    {
        // If we're at the end, it's null or whitespace
        if (it == Value.end()) { return true; }
        // Compare char for spaces
    } while (*it >= 0 && *it <= 0x7f && std::isspace(*(it++)));

    // Not null or whitespace
    return false;
}

std::string Strings::ToBase64String(const std::vector<uint8_t>& Value)
{
    // Base64 chars
    const std::string Base64CharBuffer = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    // Buffer for string
    std::stringstream StringBuffer;

    // Calculate length
    uint32_t LengthMod = Value.size() % 3;
    uint32_t CalcLength = (uint32_t)(Value.size() - LengthMod);
    // Indexes
    uint32_t I, J = 0;

    // Loop
    for (I = 0; I < CalcLength; I += 3)
    {
        // Set chars
        StringBuffer << Base64CharBuffer[(Value[I] & 0xFC) >> 2];
        StringBuffer << Base64CharBuffer[((Value[I] & 0x3) << 4) | ((Value[I + 1] & 0xF0) >> 4)];
        StringBuffer << Base64CharBuffer[((Value[I + 1] & 0xF) << 2) | ((Value[I + 2] & 0xC0) >> 6)];
        StringBuffer << Base64CharBuffer[(Value[I + 2] & 0x3F)];
    }

    // Set
    I = CalcLength;

    // Padding
    switch (LengthMod)
    {
        // Single padding
    case 2:
        StringBuffer << Base64CharBuffer[(Value[I] & 0xFC) >> 2];
        StringBuffer << Base64CharBuffer[((Value[I] & 0x3) << 4) | ((Value[I + 1] & 0xF0) >> 4)];
        StringBuffer << Base64CharBuffer[((Value[I + 1] & 0xF) << 2)];
        StringBuffer << Base64CharBuffer[64]; // Padding
        break;
        // Double padding
    case 1:
        StringBuffer << Base64CharBuffer[(Value[I] & 0xFC) >> 2];
        StringBuffer << Base64CharBuffer[((Value[I] & 0x3) << 4)];
        StringBuffer << Base64CharBuffer[64]; // Padding
        StringBuffer << Base64CharBuffer[64]; // Padding
        break;
    }

    // Return result
    return StringBuffer.str();
}

std::vector<uint8_t> Strings::FromBase64String(const std::string& Value)
{
    // Buffer for result
    std::vector<uint8_t> ByteBuffer;

    // The input buffer
    uint8_t* InputPtr = (uint8_t*)Value.c_str();
    uint8_t* EndPtr = InputPtr + Value.size();

    // For this to work, value's size must be divisible by 4
    if (Value.size() % 4 != 0) { return ByteBuffer; }

    // Buffers for faster conversion
    const uint32_t IntA = (uint32_t)'A';
    const uint32_t Inta = (uint32_t)'a';
    const uint32_t Int0 = (uint32_t)'0';
    const uint32_t IntEq = (uint32_t)'=';
    const uint32_t IntPlus = (uint32_t)'+';
    const uint32_t IntSlash = (uint32_t)'/';
    const uint32_t IntSpace = (uint32_t)' ';
    const uint32_t IntTab = (uint32_t)'\t';
    const uint32_t IntNLn = (uint32_t)'\n';
    const uint32_t IntCRt = (uint32_t)'\r';
    const uint32_t IntAtoZ = (uint32_t)('Z' - 'A');
    const uint32_t Int0to9 = (uint32_t)('9' - '0');

    // The current block
    uint32_t CurrentBlock = 0xFF;

    // Current code
    uint32_t CurrentCode = 0;

    // Loop until we've processed all data
    while (true)
    {
        // Exit when finished
        if (InputPtr >= EndPtr) { goto AllInputConsumed; }

        // Get next segment
        CurrentCode = (uint32_t)(*InputPtr);
        // Advance
        InputPtr++;

        // Determine code branch
        if (CurrentCode - IntA <= IntAtoZ) { CurrentCode -= IntA; }

        // Lowercase check
        else if (CurrentCode - Inta <= IntAtoZ) { CurrentCode -= (Inta - 26); }

        // Digit check
        else if (CurrentCode - Int0 <= Int0to9) { CurrentCode -= (Int0 - 52); }

        // Handle less common cases
        else
        {
            switch (CurrentCode)
            {
            case IntPlus:
                CurrentCode = 62;
                break;
            case IntSlash:
                CurrentCode = 63;
                break;
            case IntEq:
                goto EqualityReached;
            }
        }

        // Save the code block
        CurrentBlock = (CurrentBlock << 6) | CurrentCode;

        // The last bit in block codes will be on after and shifed right 4 times
        if ((CurrentBlock & 0x80000000) != 0)
        {
            // Set the data
            ByteBuffer.push_back((uint8_t)(CurrentBlock >> 16));
            ByteBuffer.push_back((uint8_t)(CurrentBlock >> 8));
            ByteBuffer.push_back((uint8_t)(CurrentBlock));
            // Reset
            CurrentBlock = 0xFF;
        }
    }

    // Only hit if we make it to the '=' early
EqualityReached:

    // '=' can only be in the last pos
    if (InputPtr == EndPtr)
    {
        // Code is 0 for trailing
        CurrentBlock <<= 6;

        // Good to store last bytes
        ByteBuffer.push_back((uint8_t)(CurrentBlock >> 16));
        ByteBuffer.push_back((uint8_t)(CurrentBlock >> 8));

        // Reset
        CurrentBlock = 0xFF;
    }
    else
    {
        // We may have two equal signs
        if (InputPtr == (EndPtr - 1) && *(InputPtr) == '=')
        {
            // Code is 0 for each trailing
            CurrentBlock <<= 12;

            // Good to store the last byte
            ByteBuffer.push_back((uint8_t)(CurrentBlock >> 16));

            // Reset
            CurrentBlock = 0xFF;
        }
    }

    // If we had no padding
AllInputConsumed:


    // Return result
    return ByteBuffer;
}

std::string Strings::Replace(std::string Subject, const std::string& Search, const std::string& Replace)
{
    // Current search path
    size_t CurrentPosition = 0;
    // Loop until we can't find it
    while ((CurrentPosition = Subject.find(Search, CurrentPosition)) != std::string::npos)
    {
        // Replace it
        Subject.replace(CurrentPosition, Search.length(), Replace);
        // Jump ahead
        CurrentPosition += Replace.length();
    }
    // Return result
    return Subject;
}

std::vector<std::string> Strings::SplitString(const std::string& Search, const char Split, bool StripBlank)
{
    // The result strings
    std::vector<std::string> Result;

    // Position chars
    size_t Start = 0;
    size_t End = Search.find_first_of(Split);

    // Loop until end of buffer
    while (End <= std::string::npos)
    {
        // Add result
        if (StripBlank)
        {
            // Check before adding
            auto Str = Search.substr(Start, End - Start);
            if (Str != "") { Result.emplace_back(Str); }
        }
        else
        {
            // Just add
            Result.emplace_back(Search.substr(Start, End - Start));
        }

        // Break if we're already at end
        if (End == std::string::npos) { break; }

        // Calculate next start
        Start = End + 1;
        // Find next char
        End = Search.find_first_of(Split, Start);
    }

    // Return it
    return std::move(Result);
}

bool Strings::Contains(const std::string& Subject, const std::string& Search)
{
    // Return whether or not we found this string in the subject
    return (Subject.find(Search) != std::string::npos);
}

bool Strings::StartsWith(const std::string& Subject, const std::string& Beginning)
{
    // Ensure we can even fit
    if (Subject.length() >= Beginning.length())
    {
        // Return the compare result
        return (0 == Subject.compare(0, Beginning.length(), Beginning));
    }
    // No match
    return false;
}

bool Strings::EndsWith(const std::string& Subject, const std::string& Ending)
{
    // Ensure we can even fit
    if (Subject.length() >= Ending.length())
    {
        // Return the compare result
        return (0 == Subject.compare(Subject.length() - Ending.length(), Ending.length(), Ending));
    }
    // No match
    return false;
}

std::string& Strings::ToLower(std::string& Value)
{
    // Make it lowercase
    std::transform(Value.begin(), Value.end(), Value.begin(), ::tolower);
    // Return it
    return Value;
}

std::string& Strings::ToUpper(std::string& Value)
{
    // Make it uppercase
    std::transform(Value.begin(), Value.end(), Value.begin(), ::toupper);
    // Return it
    return Value;
}

std::string Strings::ToNormalString(const std::wstring& Value)
{
    // Make the converter
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> WStringConvert;
    // Convert and return
    return WStringConvert.to_bytes(Value);
}

std::wstring Strings::ToUnicodeString(const std::string& Value)
{
    // Make the converter
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> StringConvert;
    // Convert and return
    return StringConvert.from_bytes(Value);
}

std::string Strings::DurationToReadableTime(std::chrono::milliseconds Ms)
{
    // Prepare to convert to each part

    // Check for small durations
    if (Ms.count() < 1000)
    {
        // Only use Ms
        return Format("%dms", Ms.count());
    }

    // Otherwise convert from time 

    auto Hours = std::chrono::duration_cast<std::chrono::hours>(Ms);
    // Remove from count
    Ms -= Hours;

    auto Minutes = std::chrono::duration_cast<std::chrono::minutes>(Ms);
    // Remove from count
    Ms -= Minutes;

    auto Seconds = std::chrono::duration_cast<std::chrono::seconds>(Ms);
    // Remove from count
    Ms -= Seconds;

    // Check positive values and format
    auto HoursFmt = (Hours.count() > 0) ? Format("%dh ", Hours.count()) : "";
    auto MinutesFmt = (Minutes.count() > 0) ? Format("%dm ", Minutes.count()) : "";
    auto SecondsFmt = (Seconds.count() > 0) ? Format("%ds ", Seconds.count()) : "";

    // Return it
    return Format("%s%s%s", HoursFmt.c_str(), MinutesFmt.c_str(), SecondsFmt.c_str());
}