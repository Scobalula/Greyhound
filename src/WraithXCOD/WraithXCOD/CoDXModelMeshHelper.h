#pragma once
#include "VectorMath.h"

class CoDXModelMeshHelper
{
public:
	// Unpacks a 21Bit Vertex 
	static const Vector3 Unpack21BitVertex(const uint64_t Packed, const float Scale, const Vector3 Offset);
	// Finds the absolute index from a given table and packed buffer.
	static const uint8_t FindFaceIndex(const uint8_t* a1, const size_t a2, const size_t a3);
	// Unpacks the 3 vertex indices for the given face.
	static void UnpackFaceIndices(const uint8_t* tables, const size_t tableCount, const uint8_t* packedInfos, const uint16_t* indices, const size_t faceIndex, uint32_t (&output)[3]);
};

