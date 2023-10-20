#include <stdafx.h>
#include "CoDXModelMeshHelper.h"

const Vector3 CoDXModelMeshHelper::Unpack21BitVertex(const uint64_t Packed, const float Scale, const Vector3 Offset)
{
    // Read and assign position
    return Vector3(
        (((((Packed >> 00) & 0x1FFFFF) * ((1.0f / 0x1FFFFF) * 2.0f)) - 1.0f) * Scale) + Offset.X,
        (((((Packed >> 21) & 0x1FFFFF) * ((1.0f / 0x1FFFFF) * 2.0f)) - 1.0f) * Scale) + Offset.Y,
        (((((Packed >> 42) & 0x1FFFFF) * ((1.0f / 0x1FFFFF) * 2.0f)) - 1.0f) * Scale) + Offset.Z);
}

const uint8_t CoDXModelMeshHelper::FindFaceIndex(const uint8_t* localPackedIndices, const size_t index, const size_t bits)
{
    unsigned long bitIndex;

    // Find the index of the highest set bit
    if (!_BitScanReverse64(&bitIndex, bits))
        bitIndex = 64;
    else
        bitIndex ^= 0x3F;

    const uint16_t offset = index * static_cast<uint8_t>(64 - bitIndex);
    const uint8_t bitCount = static_cast<uint8_t>(64 - bitIndex);
    const uint8_t* bytePtr = localPackedIndices + (offset >> 3);
    const uint8_t bitOffset = offset & 7;

    if (bitOffset == 0)
        return *bytePtr & ((1 << bitCount) - 1);

    if (8 - bitOffset < bitCount)
        return (*bytePtr >> bitOffset) & ((1 << (8 - bitOffset)) - 1) | ((bytePtr[1] & static_cast<uint8_t>((1 << (64 - bitIndex - (8 - bitOffset))) - 1)) << (8 - bitOffset));

    return (*bytePtr >> bitOffset) & ((1 << bitCount) - 1);
}

void CoDXModelMeshHelper::UnpackFaceIndices(const uint8_t* tables, const size_t tableCount, const uint8_t* localPackedIndices, const uint16_t* indices, const size_t faceIndex, uint32_t(&output)[3])
{
    size_t currentFaceIndex = faceIndex;

    for (size_t i = 0; i < tableCount; i++)
    {
        const uint8_t* table = tables + i * 40;
        const uint8_t* tableIndices = localPackedIndices + *(unsigned int*)(table + 36);
        const size_t count = *(unsigned __int8*)(table + 35);

        if (currentFaceIndex < count)
        {
            const size_t bits = *(unsigned __int8*)(table + 34) - 1i64;
            const size_t faceIndex = *(uint32_t*)(table + 28);

            output[0] = indices[FindFaceIndex(tableIndices, currentFaceIndex * 3 + 0, bits) + faceIndex];
            output[1] = indices[FindFaceIndex(tableIndices, currentFaceIndex * 3 + 1, bits) + faceIndex];
            output[2] = indices[FindFaceIndex(tableIndices, currentFaceIndex * 3 + 2, bits) + faceIndex];

            return;
        }

        currentFaceIndex -= count;
    }
}
