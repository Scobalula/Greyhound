#include "stdafx.h"

// The class we are implementing
#include "HalfFloats.h"

uint16_t HalfFloats::ToHalfFloat(float Value)
{
    // Define bit values
    FloatBits v, s;
    // Set the initial value
    v.f = Value;
    // Calculate
    uint32_t sign = v.si & signN;
    v.si ^= sign;
    // Logical shift
    sign >>= shiftSign;
    s.si = mulN;
    // Correct subnormals
    s.si = (int32_t)(s.f * v.f);
    v.si ^= (s.si ^ v.si) & -(minN > v.si);
    v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
    v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
    // Logical shift
    v.ui >>= shift; 
    v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
    v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
    // The result with the correct sign
    return v.ui | sign;
}

float HalfFloats::ToFloat(uint16_t Value)
{
    // Define bit values
    FloatBits v, s;
    // Set the initial value
    v.ui = Value;
    // Calculate sign
    int32_t sign = v.si & signC;
    v.si ^= sign;
    sign <<= shiftSign;
    v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
    v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
    // Inverse Subnormals
    s.si = mulC;
    s.f *= v.si;
    int32_t mask = -(norC > v.si);
    v.si <<= shift;
    v.si ^= (s.si ^ v.si) & mask;
    v.si |= sign;
    // Return the expanded result
    return v.f;
}