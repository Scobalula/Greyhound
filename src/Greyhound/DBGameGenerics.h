#pragma once

#include <cstdint>

// We need the following classes
#include <DirectXMathVector.inl>

// -- Contains structures for various shared game structures

struct DObjAnimMat
{
    Math::Quaternion Rotation;
    Vector3 Translation;
    float TranslationWeight;
};

union GfxPackedUnitVec
{
    uint32_t PackedInteger;
    int8_t PackedBytes[4];
};

struct GfxVertexBuffer
{
    Vector3 Position;

    uint32_t BiNormal;
    uint8_t Color[4];
    uint16_t UVUPos;
    uint16_t UVVPos;

    uint32_t Normal;
    uint32_t Tangent;
};

struct GfxFaceBuffer
{
    uint16_t Index1;
    uint16_t Index2;
    uint16_t Index3;
};

struct GfxRigidVerts
{
    uint16_t BoneIndex;
    uint16_t VertexCount;

    uint16_t FacesCount;
    uint16_t FacesIndex;

    uint32_t SurfaceCollisionPtr;
};

struct GfxRigidVertsQS
{
    uint16_t BoneIndex;
    uint16_t VertexCount;
    uint16_t FacesIndex;
    uint16_t FacesCount;
};

struct GfxRigidVerts64
{
    uint16_t BoneIndex;
    uint16_t VertexCount;

    uint16_t FacesCount;
    uint16_t FacesIndex;

    uint64_t SurfaceCollisionPtr;
};

struct QuatData
{
    int16_t RotationX;
    int16_t RotationY;
    int16_t RotationZ;
    int16_t RotationW;
};

struct Quat2Data
{
    int16_t RotationZ;
    int16_t RotationW;
};

struct __declspec(align(4)) GfxBoneInfo
{
    float Bounds[2][3];
    float Offset[3];
    float RadiusSquared;
    uint8_t Collmap;
};

// -- Verify structures

static_assert(sizeof(DObjAnimMat) == 0x20, "Invalid DObjAnimMat Size (Expected 0x20)");
static_assert(sizeof(QuatData) == 0x8, "Invalid QuatData Size (Expected 0x8)");
static_assert(sizeof(Quat2Data) == 0x4, "Invalid Quat2Data Size (Expected 0x4)");
static_assert(sizeof(GfxVertexBuffer) == 0x20, "Invalid GfxVertexBuffer Size (Expected 0x20)");
static_assert(sizeof(GfxPackedUnitVec) == 0x4, "Invalid GfxPackedUnitVec Size (Expected 0x4)");
static_assert(sizeof(GfxRigidVerts) == 0xC, "Invalid GfxRigidVerts Size (Expected 0xC)");
static_assert(sizeof(GfxRigidVerts64) == 0x10, "Invalid GfxRigidVerts64 Size (Expected 0x10)");
static_assert(sizeof(GfxBoneInfo) == 0x2C, "Invalid GfxBoneInfo Size (Expected 0x2C)");

// -- End reading structures