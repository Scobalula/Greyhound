#pragma once

#include <cstdint>
#include <memory>

// We need the following WraithX classes
#include <WraithAnim.h>

// We need the cod assets
#include "CoDAssets.h"

// A class that handles translating generic Call of Duty XAnims to Wraith Anims
class CoDXAnimTranslator
{
public:
    // -- Translation functions

    // Translates an in-game XAnim to a WraithAnim
    static std::unique_ptr<WraithAnim> TranslateXAnim(const std::unique_ptr<XAnim_t>& Animation);

private:
    // -- Translation utilities (XAnims)

    // Skip over inline animation indicies, if supported
    static void SkipInlineAnimationIndicies(const std::unique_ptr<XAnim_t>& Animation);

    // Build Notetracks (Standard)
    static void NotetracksStandard(const std::unique_ptr<WraithAnim>& Anim, const std::unique_ptr<XAnim_t>& Animation);
    // Build Notetracks (BO3)
    static void NotetracksBO3(const std::unique_ptr<WraithAnim>& Anim, const std::unique_ptr<XAnim_t>& Animation);
    // Build Notetracks (BO4)
    static void NotetracksBO4(const std::unique_ptr<WraithAnim>& Anim, const std::unique_ptr<XAnim_t>& Animation);
    // Build Notetracks (CW)
    static void NotetracksCW(const std::unique_ptr<WraithAnim>& Anim, const std::unique_ptr<XAnim_t>& Animation);
    // Build Notetracks (VG)
    static void NotetracksVG(const std::unique_ptr<WraithAnim>& Anim, const std::unique_ptr<XAnim_t>& Animation);

    // Build Delta Translations for 32bit games
    static void DeltaTranslations32(const std::unique_ptr<WraithAnim>& Anim, uint32_t FrameSize, const std::unique_ptr<XAnim_t>& Animation);
    // Build Delta Translations for 64bit games
    static void DeltaTranslations64(const std::unique_ptr<WraithAnim>& Anim, uint32_t FrameSize, const std::unique_ptr<XAnim_t>& Animation);

    // Build Delta 2D Rotations for 32bit games
    static void Delta2DRotations32(const std::unique_ptr<WraithAnim>& Anim, uint32_t FrameSize, const std::unique_ptr<XAnim_t>& Animation);
    // Build Delta 2D Rotations for 64bit games
    static void Delta2DRotations64(const std::unique_ptr<WraithAnim>& Anim, uint32_t FrameSize, const std::unique_ptr<XAnim_t>& Animation);

    // Build Delta 3D Rotations for 32bit games
    static void Delta3DRotations32(const std::unique_ptr<WraithAnim>& Anim, uint32_t FrameSize, const std::unique_ptr<XAnim_t>& Animation);
    // Build Delta 3D Rotations for 64bit games
    static void Delta3DRotations64(const std::unique_ptr<WraithAnim>& Anim, uint32_t FrameSize, const std::unique_ptr<XAnim_t>& Animation);
};