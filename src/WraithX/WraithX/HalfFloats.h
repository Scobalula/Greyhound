#pragma once

#include <cstdint>

class HalfFloats
{
private:
	// Represents the different casts of a float
	union FloatBits
	{
		float f;
		int32_t si;
		uint32_t ui;
	};

	// Shift value
	static int const shift = 13;
	// Sign used to shift
	static int const shiftSign = 16;

	// Infinity of a float
	static int32_t const infN = 0x7F800000;
	// Maximum value of a half float
	static int32_t const maxN = 0x477FE000;
	// Minimum value of a half float
	static int32_t const minN = 0x38800000;
	// Sign bit of a float
	static int32_t const signN = 0x80000000;

	// Precalculated properties of a half float
	static int32_t const infC = infN >> shift;
	static int32_t const nanN = (infC + 1) << shift;
	static int32_t const maxC = maxN >> shift;
	static int32_t const minC = minN >> shift;
	static int32_t const signC = signN >> shiftSign;

	// Precalculated (1 << 23) / minN
	static int32_t const mulN = 0x52000000;
	// Precalculated minN / (1 << (23 - shift))
	static int32_t const mulC = 0x33800000;

	// Max float subnormal down shifted
	static int32_t const subC = 0x003FF;
	// Min float normal down shifted
	static int32_t const norC = 0x00400;

	// Precalculated min and max decimals
	static int32_t const maxD = infC - maxC - 1;
	static int32_t const minD = minC - subC - 1;

public:
	// Compress a float to a half float
	static uint16_t ToHalfFloat(float Value);
	// Decompress a half float to a float
	static float ToFloat(uint16_t Value);
};