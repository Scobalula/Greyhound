#pragma once

#include <cstdint>
#include <iostream>
#include <string>

class SHA1
{
public:
    // Create a new SHA-1 calculator
    SHA1();
    // Update the data with our string buffer
    void update(const std::string &s);
    // Update the data with our file
    void update(std::istream &is);
    // Calculate the final hash
    std::string final();
    // Calculate the hash of a file
    static std::string from_file(const std::string &filename);

private:
    // The actual SHA-1 data
    uint32_t digest[5];
    // The buffer to store data
    std::string buffer;
    // Transform data
    uint64_t transforms;
};