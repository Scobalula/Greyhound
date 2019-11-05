#pragma once

#include <cstdint>

// We need to include external libraries for encryption
#include "Salsa20.h"

// -- Salsa20 structures

// Contains the information for a Salsa20 Cipher
struct Salsa20Key
{
    // The key for the Salsa20 cipher
    uint8_t* Key;
    // The length of the key, either 128 or 256 bits
    uint32_t KeyLength;
    // The iv for the Salsa20 cipher
    uint8_t* IV;

    Salsa20Key()
    {
        Key = nullptr;
        KeyLength = 0;
        IV = nullptr;
    }
};

class Encryption
{
public:
    // -- Salsa20 Functions (One for salsa)

    // Encrypt/Decrypts a block of Salsa20 data, with the given Key and IV
    static uint32_t Salsa20Block(int8_t* Buffer, uint32_t BufferSize, const Salsa20Key& Key);
};