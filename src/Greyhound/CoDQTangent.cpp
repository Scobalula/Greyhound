#include "pch.h"
#include "CoDQTangent.h"
#include "Vector3.h"

Vector3 CoDQTangent::Unpack(Vector3* tangent, Vector3* bitangent)
{
    // https://dev.theomader.com/qtangents/
    auto idx = Packed >> 30;

    auto tx = ((Packed >> 00 & 0x3FF) / 511.5f - 1.0f) / 1.4142135f;
    auto ty = ((Packed >> 10 & 0x3FF) / 511.5f - 1.0f) / 1.4142135f;
    auto tz = ((Packed >> 20 & 0x1FF) / 255.5f - 1.0f) / 1.4142135f;
    auto tw = 0.0f;

    auto sum = tx * tx + ty * ty + tz * tz;

    if (sum <= 1.0f)
        tw = (float)std::sqrtf(1.0f - sum);

    float qX = 0.0f;
    float qY = 0.0f;
    float qZ = 0.0f;
    float qW = 0.0f;

    switch (idx)
    {
    case 0:
        qX = tw; qY = tx; qZ = ty; qW = tz;
        break;
    case 1:
        qX = tx; qY = tw; qZ = ty; qW = tz;
        break;
    case 2:
        qX = tx; qY = ty; qZ = tw; qW = tz;
        break;
    case 3:
        qX = tx; qY = ty; qZ = tz; qW = tw;
        break;
    }

    Vector3 tempTangent(
        1 - 2 * (qY * qY + qZ * qZ),
        2 * (qX * qY + qW * qZ),
        2 * (qX * qZ - qW * qY)
    );
    Vector3 tempBitangent(
        2 * (qX * qY - qW * qZ),
        1 - 2 * (qX * qX + qZ * qZ),
        2 * (qY * qZ + qW * qX));
    Vector3 tempNormal(
        (tempTangent.Y * tempBitangent.Z) - (tempTangent.Z * tempBitangent.Y),
        (tempTangent.Z * tempBitangent.X) - (tempTangent.X * tempBitangent.Z),
        (tempTangent.X * tempBitangent.Y) - (tempTangent.Y * tempBitangent.X));

    // Only output these if required, we usually only require normal for exporting.
    if (tangent != nullptr)
        *tangent = tempTangent;
    if (bitangent != nullptr)
        *bitangent = tempBitangent;

    return tempNormal;
}
