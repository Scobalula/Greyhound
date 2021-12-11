#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

// We need the WraithAsset as a base
#include "WraithAsset.h"

// We need the vector 3 and quaternion classes
#include "VectorMath.h"

// Specifies how the animation data is interpreted by the importer
enum class WraithAnimationType : uint8_t
{
    // Animation translations are set to this exact value each frame
    Absolute = 0,
    // This animation is applied to existing animation data in the scene (and is absolute)
    Additive = 1,
    // Animation translations are based on rest position in scene
    Relative = 2,
    // This animation is relative and contains delta data (Whole model movement) Delta tag name must be set!
    Delta = 3,
};

template<class T>
// A structure that contains frame data for a WraithAnim
struct WraithAnimFrame
{
    // The frame of this animation key
    uint32_t Frame;
    // The frame value of this key
    T Value;
};

// A class that represents an animation
class WraithAnim : public WraithAsset
{
public:
    // Creates a new WraithAnim
    WraithAnim();
    ~WraithAnim();

    // -- Animation key data

    // A list of animation keys, by bone, for positions
    std::unordered_map<std::string, std::vector<WraithAnimFrame<Vector3>>> AnimationPositionKeys;
    // A list of animation keys, by bone, for rotations
    std::unordered_map<std::string, std::vector<WraithAnimFrame<Quaternion>>> AnimationRotationKeys;
    // A list of animation keys, by bone, for scales
    std::unordered_map<std::string, std::vector<WraithAnimFrame<Vector3>>> AnimationScaleKeys;
    // A list of animation blendshape weight keys per axis
    std::unordered_map<std::string, std::vector<WraithAnimFrame<Vector3>>> AnimationBlendShapeWeightKeys;
    // A list of animation keys, for notetracks
    std::unordered_map<std::string, std::vector<uint32_t>> AnimationNotetracks;
    // A list of animation modifiers, by bone
    std::unordered_map<std::string, WraithAnimationType> AnimationBoneModifiers;

    // -- Animation properties

    // The count of frames in the animation, this is automatically updated
    const uint32_t FrameCount() const;
    // The count of bones in the animation, this is automatically updated
    const uint32_t BoneCount() const;
    // A list of bones currently being used in this animation, this is automatically updated
    const std::unordered_set<std::string> Bones() const;
    // The count of notifications in the animation, this is automatically updated
    const uint32_t NotificationCount() const;

    // The animation type for this anim
    WraithAnimationType AnimType;
    // Whether or not the animation should loop
    bool Looping;
    // The name of the delta tag of which to treat as the delta bone (If any)
    std::string DeltaTagName;
    // The framerate of the animation as a float (Defaults to 30.0)
    float FrameRate;

    // -- Adding keys functions

    // Add a translation key for the specified bone, on the given frame, with a Vector3
    void AddTranslationKey(const std::string Bone, uint32_t Frame, float X, float Y, float Z);
    // Add a rotation key for the specified bone, on the given frame, with a Quaternion
    void AddRotationKey(const std::string Bone, uint32_t Frame, float X, float Y, float Z, float W);
    // Add a scale key for the specified bone, on the given frame, with a Vector3
    void AddScaleKey(const std::string Bone, uint32_t Frame, float X, float Y, float Z);
    // Add a blendshape key for the specified shape, on the given frame, with a Vector3 for weight
    void AddBlendShapeKey(const std::string Shape, uint32_t Frame, float X, float Y, float Z);

    // Add a notification at the specified frame
    void AddNoteTrack(const std::string Notification, uint32_t Frame);
    // Adds a bone modifier for this bone, overwriting if existing
    void AddBoneModifier(const std::string Bone, WraithAnimationType Modifier);

    // -- Removing keys functions

    // Remove a specific keyframe from a bone
    void RemoveTranslationKey(const std::string& Bone, uint32_t Frame);
    // Remove a specific keyframe from a bone
    void RemoveRotationKey(const std::string& Bone, uint32_t Frame);
    // Remove a specific keyframe from a bone
    void RemoveScaleKey(const std::string& Bone, uint32_t Frame);

    // Remove a specific notification frame
    void RemoveNotetrack(const std::string& Bone, uint32_t Frame);

    // -- Utility functions

    // Scales every translation keyframe with the given factor
    void ScaleAnimation(float ScaleFactor);
};