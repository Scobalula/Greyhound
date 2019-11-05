#include "stdafx.h"

// The class we are implementing
#include "WraithAnim.h"

WraithAnim::WraithAnim()
{
    // Defaults
    AssetType = WraithAssetType::Animation;
    AnimType = WraithAnimationType::Absolute;
    Looping = false;
    DeltaTagName = "";
    FrameRate = 30.0f;
}

WraithAnim::~WraithAnim()
{
    // Clean up if we need to
}

const uint32_t WraithAnim::FrameCount() const
{
    // Result buffer
    uint32_t Result = 0;

    // We must iterate through all key types and compare the max keyed frame
    for (auto& Frames : AnimationPositionKeys)
    {
        // Iterate
        for (auto& Frame : Frames.second)
        {
            // Compare
            Result = std::max(Result, Frame.Frame);
        }
    }
    for (auto& Frames : AnimationRotationKeys)
    {
        // Iterate
        for (auto& Frame : Frames.second)
        {
            // Compare
            Result = std::max(Result, Frame.Frame);
        }
    }
    for (auto& Frames : AnimationScaleKeys)
    {
        // Iterate
        for (auto& Frame : Frames.second)
        {
            // Compare
            Result = std::max(Result, Frame.Frame);
        }
    }
    for (auto& Frames : AnimationNotetracks)
    {
        // Iterate
        for (auto& Frame : Frames.second)
        {
            // Compare
            Result = std::max(Result, Frame);
        }
    }

    // Frame count represents the length of the animation in frames
    // Since all animations start at frame 0, we grab the max number and
    // Add 1 to the result
    return Result + 1;
}

const uint32_t WraithAnim::BoneCount() const
{
    // Return the size of the bones result
    return (uint32_t)Bones().size();
}

const std::unordered_set<std::string> WraithAnim::Bones() const
{
    // A hashset of unique names
    std::unordered_set<std::string> UniqueBoneNames;

    // We must append the bone names from all bone type keys (and bone modifiers)
    for (auto& Frames : AnimationPositionKeys)
    {
        // Iterate
        UniqueBoneNames.insert(Frames.first);
    }
    for (auto& Frames : AnimationRotationKeys)
    {
        // Iterate
        UniqueBoneNames.insert(Frames.first);
    }
    for (auto& Frames : AnimationScaleKeys)
    {
        // Iterate
        UniqueBoneNames.insert(Frames.first);
    }
    for (auto& Modifiers : AnimationBoneModifiers)
    {
        // Iterate
        UniqueBoneNames.insert(Modifiers.first);
    }

    // Return it
    return UniqueBoneNames;
}

const uint32_t WraithAnim::NotificationCount() const
{
    // The total count
    uint32_t Result = 0;

    // Loop and add
    for (auto& NoteList : AnimationNotetracks)
    {
        // Append
        Result += (uint32_t)NoteList.second.size();
    }

    // Return it
    return Result;
}

void WraithAnim::AddTranslationKey(const std::string Bone, uint32_t Frame, float X, float Y, float Z)
{
    // Make it
    WraithAnimFrame<Vector3> NewFrame;
    // Set values
    NewFrame.Frame = Frame;
    NewFrame.Value = Vector3(X, Y, Z);
    // Add the key itself
    AnimationPositionKeys[Bone].push_back(NewFrame);
}

void WraithAnim::AddRotationKey(const std::string Bone, uint32_t Frame, float X, float Y, float Z, float W)
{
    // Make it
    WraithAnimFrame<Quaternion> NewFrame;
    // Set values
    NewFrame.Frame = Frame;
    NewFrame.Value = Quaternion(X, Y, Z, W);
    // Add the key itself
    AnimationRotationKeys[Bone].push_back(NewFrame);
}

void WraithAnim::AddScaleKey(const std::string Bone, uint32_t Frame, float X, float Y, float Z)
{
    // Make it
    WraithAnimFrame<Vector3> NewFrame;
    // Set values
    NewFrame.Frame = Frame;
    NewFrame.Value = Vector3(X, Y, Z);
    // Add the key itself
    AnimationScaleKeys[Bone].push_back(NewFrame);
}

void WraithAnim::AddNoteTrack(const std::string Notification, uint32_t Frame)
{
    // Add it
    AnimationNotetracks[Notification].push_back(Frame);
}

void WraithAnim::AddBoneModifier(const std::string Bone, WraithAnimationType Modifier)
{
    // Add the modifier
    AnimationBoneModifiers[Bone] = Modifier;
}

void WraithAnim::RemoveTranslationKey(const std::string& Bone, uint32_t Frame)
{
    // Make sure the bone exists
    if (AnimationPositionKeys.find(Bone) != AnimationPositionKeys.end())
    {
        // It exists, loop and check for the key
        for (size_t i = 0; i < AnimationPositionKeys[Bone].size(); i++)
        {
            // Check
            if (AnimationPositionKeys[Bone][i].Frame == Frame)
            {
                // Remove it
                AnimationPositionKeys[Bone].erase(AnimationPositionKeys[Bone].begin() + i);
                // End
                break;
            }
        }
    }
}

void WraithAnim::RemoveRotationKey(const std::string& Bone, uint32_t Frame)
{
    // Make sure the bone exists
    if (AnimationRotationKeys.find(Bone) != AnimationRotationKeys.end())
    {
        // It exists, loop and check for the key
        for (size_t i = 0; i < AnimationRotationKeys[Bone].size(); i++)
        {
            // Check
            if (AnimationRotationKeys[Bone][i].Frame == Frame)
            {
                // Remove it
                AnimationRotationKeys[Bone].erase(AnimationRotationKeys[Bone].begin() + i);
                // End
                break;
            }
        }
    }
}

void WraithAnim::RemoveScaleKey(const std::string& Bone, uint32_t Frame)
{
    // Make sure the bone exists
    if (AnimationScaleKeys.find(Bone) != AnimationScaleKeys.end())
    {
        // It exists, loop and check for the key
        for (size_t i = 0; i < AnimationScaleKeys[Bone].size(); i++)
        {
            // Check
            if (AnimationScaleKeys[Bone][i].Frame == Frame)
            {
                // Remove it
                AnimationScaleKeys[Bone].erase(AnimationScaleKeys[Bone].begin() + i);
                // End
                break;
            }
        }
    }
}

void WraithAnim::RemoveNotetrack(const std::string& Bone, uint32_t Frame)
{
    // Make sure the bone exists
    if (AnimationNotetracks.find(Bone) != AnimationNotetracks.end())
    {
        // It exists, loop and check for the key
        for (size_t i = 0; i < AnimationNotetracks[Bone].size(); i++)
        {
            // Check
            if (AnimationNotetracks[Bone][i] == Frame)
            {
                // Remove it
                AnimationNotetracks[Bone].erase(AnimationNotetracks[Bone].begin() + i);
                // End
                break;
            }
        }
    }
}

void WraithAnim::ScaleAnimation(float ScaleFactor)
{
    // We must iterate over all of the frames
    for (auto& Frames : AnimationPositionKeys)
    {
        // Iterate
        for (auto& Frame : Frames.second)
        {
            // Scale them
            Frame.Value *= ScaleFactor;
        }
    }
}