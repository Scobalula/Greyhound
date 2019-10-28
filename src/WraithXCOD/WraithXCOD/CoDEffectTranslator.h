#pragma once

#include <cstdint>
#include <memory>

// We need the cod assets
#include "CoDAssets.h"
#include "TextWriter.h"

// A class that handles translating generic Call of Duty FX to a file
class CoDEffectTranslator
{
public:
    // -- Generic function

    // Translates an FX file from various games
    static void TranslateEffect(const CoDEffect_t* EffectAsset, const std::string& EffectPath);

private:
    // -- Translation functions

    // Translates an FX file from Black Ops 1
    static void BOTranslateEffect(uint64_t EffectPtr, const std::string& EffectName, const std::string& EffectPath);
    // Translates an FX file from Black Ops 2
    static void BO2TranslateEffect(uint64_t EffectPtr, const std::string& EffectName, const std::string& EffectPath);
    // Translates an FX file from Black Ops 3
    static void BO3TranslateEffect(uint64_t EffectPtr, const std::string& EffectName, const std::string& EffectPath);

    // -- Element translation functions

    // Translates an FX element from Black Ops 1-2
    static void BOTranslateElement(TextWriter& Writer, uint64_t ElementPtr, uint32_t ElementIndex, uint32_t ElementType);
    // Translates an FX element from Black Ops 3
    static void BO3TranslateElement(TextWriter& Writer, uint64_t ElementPtr, uint32_t ElementIndex, uint32_t ElementType);
};