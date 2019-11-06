#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <array>

// We need the DBGameAssets and CoDAssetType classes
#include "DBGameAssets.h"
#include "CoDAssetType.h"

// Handles reading from Quantum Solace
class GameQuantumSolace
{
public:
    // -- Game functions

    // Loads offsets for Quantum Solace
    static bool LoadOffsets();
    // Loads assets for Quantum Solace
    static bool LoadAssets();

    // Reads an XAnim from Quantum Solace
    static std::unique_ptr<XAnim_t> ReadXAnim(const CoDAnim_t* Animation);
    // Reads a XModel from Quantum Solace
    static std::unique_ptr<XModel_t> ReadXModel(const CoDModel_t* Model);

    // Reads an XImageDDS from a image reference from Quantum Solace
    static std::unique_ptr<XImageDDS> LoadXImage(const XImage_t& Image);

    // Reads a string via it's string index for Quantum Solace
    static std::string LoadStringEntry(uint64_t Index);

private:
    // -- Game offsets databases

    // A list of offsets for Quantum Solace
    static std::array<DBGameInfo, 2> Offsets;
    // A list of library names for Quantum Solace
    static std::array<std::string, 2> LibraryNames;

    // -- Game utilities

    // Reads an XMaterial from it's logical offset in memory
    static const XMaterial_t ReadXMaterial(uint64_t MaterialPointer);
};