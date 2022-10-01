#pragma once
#include "VectorMath.h"

// A struct to hold a 
class CoDQTangent
{
private:
	// The packed Quaternion value.
	uint32_t Packed = 0;
public:
	// Unpacks the QTangent value into a normal. Optionally can output tangent and bitangent.
	Vector3 Unpack(Vector3* tangent, Vector3* bitangents);
};