#pragma once

#include <string>

// We require the strings utility
#include "Strings.h"

class Hashing
{
public:
    // -- SHA1 Hashing

    // Convert a string to it's SHA-1 hash
    static std::string HashSHA1String(const std::string& Value);
    // Convert a file to it's SHA-1 hash
    static std::string HashSHA1File(const std::string& FileName);

    // -- MD5 Hashing

    // Convert a string to it's MD5 hash
    static std::string HashMD5String(const std::string& Value);
    // Convert a file to it's MD5 hash
    static std::string HashMD5File(const std::string& FileName);

    // -- CRC32 Hashing

    // Convert a string to it's CRC32 hash
    static std::string HashCRC32String(const std::string& Value, uint32_t Initial = 0);
    // Convert a buffer to it's CRC32 hash
    static std::string HashCRC32Stream(int8_t* Buffer, uint64_t Size, uint32_t Initial = 0);

    // Convert a string to it's CRC32 hash (int)
    static uint32_t HashCRC32StringInt(const std::string& Value, uint32_t Initial = 0);
    // Convert a buffer to it's CRC32 hash (int)
    static uint32_t HashCRC32StreamInt(int8_t* Buffer, uint64_t Size, uint32_t Initial = 0);

    // -- XXHASH Hashing

    // Convert a string to it's XXHash hash
    static uint64_t HashXXHashString(const std::string& Value);
    // Convert a buffer to it's XXHash hash
    static uint64_t HashXXHashStream(int8_t* Buffer, uint64_t Size);
};