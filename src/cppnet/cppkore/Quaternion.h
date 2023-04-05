#pragma once

#include <cstdint>
#include "MathHelper.h"
#include "Vector3.h"

namespace Math
{
	// Represents a 3D rotation (X/Y/Z/W)
	class Quaternion
	{
	public:
		Quaternion();
		Quaternion(float X, float Y, float Z, float W);

		float X;
		float Y;
		float Z;
		float W;

		// Logical operators
		Quaternion operator+(const Quaternion& Rhs) const;
		Quaternion operator-(const Quaternion& Rhs) const;
		Quaternion operator*(const Quaternion& Rhs) const;

		// Logical assignment operators
		Quaternion& operator+=(const Quaternion& Rhs);
		Quaternion& operator-=(const Quaternion& Rhs);
		Quaternion& operator*=(const Quaternion& Rhs);

		float operator[](int i) const;
		float& operator[](int i);

		// Equality operator
		bool operator==(const Quaternion& Rhs) const;
		// Inequality operator
		bool operator!=(const Quaternion& Rhs) const;

		// Unary operator
		Quaternion operator-() const;
		// Conjugate operator
		Quaternion operator~() const;

		// Get the length of this instance
		float Length() const;
		// Get the length squared of this instance
		float LengthSq() const;
		// Normalize this instance
		void Normalize();
		// Get a normalized version of this instance
		Quaternion GetNormalized() const;

		// Convert this rotation to euler angles
		Vector3 ToEulerAngles() const;
		// Calculate the inverse of this instance
		Quaternion Inverse() const;

		// Quaternion slerp interpolation
		Quaternion Slerp(Quaternion& Rhs, float Time);

		// Convert this euler rotation to a quaternion rotation
		static Quaternion FromEulerAngles(float X, float Y, float Z);
		// Convert an axis rotation to a quaternion rotation
		static Quaternion FromAxisRotation(Vector3 Axis, float Angle);

		// Get an identity quaternion
		static Quaternion Identity();

		// Unpack a quaternion using method (A)
		inline static Quaternion QuatPackingA(const uint64_t PackedData)
		{
			// Load and shift 2 bits
			uint64_t PackedQuatData = PackedData;
			int Axis = PackedQuatData & 3;
			int WSign = PackedQuatData >> 63;
			PackedQuatData >>= 2;

			// Calculate XYZ
			int ix = (int)(PackedQuatData & 0xfffff);
			if (ix > 0x7ffff) ix -= 0x100000;
			int iy = (int)((PackedQuatData >> 20) & 0xfffff);
			if (iy > 0x7ffff) iy -= 0x100000;
			int iz = (int)((PackedQuatData >> 40) & 0xfffff);
			if (iz > 0x7ffff) iz -= 0x100000;
			float x = ix / 1048575.f;
			float y = iy / 1048575.f;
			float z = iz / 1048575.f;

			// Mod all values
			x *= 1.41421f;
			y *= 1.41421f;
			z *= 1.41421f;

			// Calculate W
			float w = (float)std::pow<float>(1 - x * x - y * y - z * z, 0.5f);

			// Determine sign of W
			if (WSign)
				w = -w;

			// Determine axis
			switch (Axis)
			{
			case 0: return Quaternion(w, x, y, z);
			case 1: return Quaternion(x, y, z, w);
			case 2: return Quaternion(y, z, w, x);
			case 3: return Quaternion(z, w, x, y);
			default: return Quaternion::Identity();
			}
		}

		// Unpack a quaternion (2d) using method (A)
		inline static Quaternion QuatPacking2DA(const uint32_t PackedData)
		{
			// Load data, calculate WSign, mask off bits
			uint32_t PackedQuatData = PackedData;
			int WSign = (PackedQuatData >> 30) & 1;
			PackedQuatData &= 0xBFFFFFFF;

			// Calculate Z W
			float z = *(float*)&PackedQuatData;
			float w = (float)std::sqrtf(1.f - std::pow<float>(z, 2.f));

			// Determine sign of W
			if (WSign)
				w = -w;

			// Return it
			return Quaternion(0, 0, z, w);
		}
	};

	static_assert(sizeof(Quaternion) == 0x10, "Invalid Math::Quaternion size, expected 0x10");
}