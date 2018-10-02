#pragma once

#include <cstdint>

// Global constant values
const double EpsilonValue = 4.37114e-05;
const double PIValue = 3.14159265358979323846;

// A class that handles math calculations
class VectorMath
{
public:
	// Convert degrees to radians
	static float DegreesToRadians(float Value);
	// Convert radians to degrees
	static float RadiansToDegrees(float Value);

	template <typename T>
	// Clamp a number between two bounds
	static T Clamp(const T& Value, const T& Lower, const T& Upper)
	{
		// Clamp it
		return std::max(Lower, std::min(Value, Upper));
	}

	template< typename Iter1, typename Iter2 >
	// A safe equals comparison wrapper
	static bool EqualsSafe(Iter1 begin1, Iter1 end1, Iter2 begin2, Iter2 end2)
	{
		while (begin1 != end1 && begin2 != end2)
		{
			if (*begin1 != *begin2)
			{
				return false;
			}
			++begin1;
			++begin2;
		}
		return begin1 == end1 && begin2 == end2;
	}

	template<typename T>
	// Compare two floats for equality using epsilon
	static bool FloatEquals(const T& lhs, const T& rhs)
	{
		return (std::fabs(lhs - rhs) < EpsilonValue);
	}
};

// A class that handles math for a 2D Vector (Or UV set)
class Vector2
{
public:
	// Create a new Vector2 set to zero
	Vector2();
	// Create a new Vector2 with the given coordinates
	Vector2(float XCoord, float YCoord);
	~Vector2();

	// Coordinate (X or UV U)
	union
	{
		// The X coordinate of the vector
		float X;
		// The U position of a UV
		float U;
	};
	// Coordinate (Y or UV V)
	union
	{
		// The Y coordinate of the vector
		float Y;
		// The V position of a UV
		float V;
	};

	//-- Operators

	// Array access operator
	float& operator[](int n);
	// Addition operator
	Vector2 operator+(const Vector2& Value) const;
	// Subtraction operator
	Vector2 operator-(const Vector2& Value) const;
	// Multiplication operator
	Vector2 operator*(const Vector2& Value) const;
	// Division operator
	Vector2 operator/(const Vector2& Value) const;
	// Addition assign operator
	Vector2& operator+=(const Vector2& Value);
	// Subtraction assign operator
	Vector2& operator-=(const Vector2& Value);
	// Multiplication assign operator
	Vector2& operator*=(const Vector2& Value);
	// Division assign operator
	Vector2& operator/=(const Vector2& Value);
	// Scale add operator
	Vector2 operator+(float Value) const;
	// Scale subtraction operator
	Vector2 operator-(float Value) const;
	// Scale multiplication operator
	Vector2 operator*(float Value) const;
	// Scale division operator
	Vector2 operator/(float Value) const;
	// Scale add assign operator
	Vector2& operator+=(float Value);
	// Scale subtraction assign operator
	Vector2& operator-=(float Value);
	// Scale multiplication assign operator
	Vector2& operator*=(float Value);
	// Scale division assign operator
	Vector2& operator/=(float Value);
	// Equals operator
	bool operator==(const Vector2& Value) const;
	// Doesn't equal operator
	bool operator!=(const Vector2& Value) const;
	// Unary operator
	Vector2 operator-() const;

	// -- Operations

	// Get the length of the Vector2
	float Length() const;
	// Get the length squared of the Vector2
	float LengthSq() const;
	// Normalize the Vector2
	void Normalize();
	// Get the normalized version of this Vector2
	Vector2 GetNormalized() const;
	
	// Linear interpolate to another Vector2
	Vector2 Lerp(float Factor, const Vector2& Value) const;
};

// A class that handles math for a 3D Vector
class Vector3
{
public:
	// Create a new Vector3 set to zero
	Vector3();
	// Create a new Vector3 with the given coordinates
	Vector3(float XCoord, float YCoord, float ZCoord);
	~Vector3();

	// Coordinate (X)
	union
	{
		// The X coordinate of the vector
		float X;
	};
	// Coordinate (Y)
	union
	{
		// The Y coordinate of the vector
		float Y;
	};
	// Coordinate (Z)
	union
	{
		// The Z coordinate of the vector
		float Z;
	};

	// -- Operators

	// Array access operator
	float& operator[](int n);
	// Addition operator
	Vector3 operator+(const Vector3& Value) const;
	// Subtraction operator
	Vector3 operator-(const Vector3& Value) const;
	// Multiplication operator
	Vector3 operator*(const Vector3& Value) const;
	// Division operator
	Vector3 operator/(const Vector3& Value) const;
	// Addition assign operator
	Vector3& operator+=(const Vector3& Value);
	// Subtraction assign operator
	Vector3& operator-=(const Vector3& Value);
	// Multiplication assign operator
	Vector3& operator*=(const Vector3& Value);
	// Division assign operator
	Vector3& operator/=(const Vector3& Value);
	// Scale add operator
	Vector3 operator+(float Value) const;
	// Scale subtraction operator
	Vector3 operator-(float Value) const;
	// Scale multiplication operator
	Vector3 operator*(float Value) const;
	// Scale division operator
	Vector3 operator/(float Value) const;
	// Scale add assign operator
	Vector3& operator+=(float Value);
	// Scale subtraction assign operator
	Vector3& operator-=(float Value);
	// Scale multiplication assign operator
	Vector3& operator*=(float Value);
	// Scale division assign operator
	Vector3& operator/=(float Value);
	// Equals operator
	bool operator==(const Vector3& Value) const;
	// Doesn't equal operator
	bool operator!=(const Vector3& Value) const;
	// Unary operator
	Vector3 operator-() const;

	// -- Operations

	// Get the length of the Vector3
	float Length() const;
	// Get the length squared of the Vector3
	float LengthSq() const;
	// Normalize the Vector3
	void Normalize();
	// Get the normalized version of this Vector3
	Vector3 GetNormalized() const;
	// Linear interpolate to another Vector3
	Vector3 Lerp(float Factor, const Vector3& Value) const;
};

// A class that handles math for a Quaternion rotation
class Quaternion
{
public:
	// Create a new Quaternion set to identity
	Quaternion();
	// Create a new Quaternion with the given coordinates
	Quaternion(float XCoord, float YCoord, float ZCoord, float WCoord);
	~Quaternion();

	// Coordinate (X)
	union
	{
		// The X coordinate of the quaternion
		float X;
	};
	// Coordinate (Y)
	union
	{
		// The Y coordinate of the quaternion
		float Y;
	};
	// Coordinate (Z)
	union
	{
		// The Z coordinate of the quaternion
		float Z;
	};
	// Coordinate (W)
	union
	{
		// The W coordinate of the quaternion
		float W;
	};

	// -- Operators

	// Array access operator
	float& operator[](int n);
	// Addition operator
	Quaternion operator+(const Quaternion& Value) const;
	// Subtraction operator
	Quaternion operator-(const Quaternion& Value) const;
	// Multiplication operator
	Quaternion operator*(const Quaternion& Value) const;
	// Addition assign operator
	Quaternion& operator+=(const Quaternion& Value);
	// Subtraction assign operator
	Quaternion& operator-=(const Quaternion& Value);
	// Multiplication assign operator
	Quaternion& operator*=(const Quaternion& Value);
	// Equals operator
	bool operator==(const Quaternion& Value) const;
	// Doesn't equal operator
	bool operator!=(const Quaternion& Value) const;
	// Unary negate operator
	Quaternion operator-() const;
	// Conjugate operator
	Quaternion operator~() const;

	// -- Operations

	// Get the length of the Quaternion
	float Length() const;
	// Get the length squared of the Quaternion
	float LengthSq() const;
	// Normalize the Quaternion
	void Normalize();
	// Get the normalized version of this Quaternion
	Quaternion GetNormalized() const;

	// Calculate euler angles of the Quaternion
	Vector3 ToEulerAngles() const;
	// Calculate the inverse of the Quaternion
	Quaternion Inverse() const;

	// -- Static operations

	// Get a quaternion from euler angles (degrees)
	static Quaternion FromEulerAngles(float XCoord, float YCoord, float ZCoord);
	// Get a quaternion rotated around an axis
	static Quaternion FromAxisRotation(Vector3 Axis, float Angle);

	// Gets an identity quaternion
	static Quaternion Identity();
};

// -- Matrix 4x4 notation
/*

	m11, m12, m13, m14 = 0,0 0,1 0,2 0,3
	m21, m22, m23, m24 = 1,0 1,1 1,2 1,3
	m31, m32, m33, m34 = 2,0 2,1 2,2 2,3
	m41, m42, m43, m44 = 3,0 3,1 3,2 3,3

*/

// A class that handles math for a 4x4 Matrix
class Matrix
{
private:
	// The data for the Matrix
	float Data[16];

public:
	// Create a new identity Matrix
	Matrix();
	~Matrix();

	// -- Operators

	// Equals operator
	bool operator==(const Matrix& Value) const;
	// Doesn't equal operator
	bool operator!=(const Matrix& Value) const;
	// Addition operator
	Matrix operator+(const Matrix& Value) const;
	// Subtraction operator
	Matrix operator-(const Matrix& Value) const;
	// Multiplication operator
	Matrix operator*(const Matrix& Value) const;
	// Scalar divide operator
	Matrix operator/(float Value) const;

	// -- Operations

	// Get a value from a normal Matrix
	float& Mat(int X, int Y);
	// Get a value from a normal Matrix constant
	const float& Mat(int X, int Y) const;
	// Calculates the determinant of the Matrix
	float Determinant();
	// Calculate the inverse of this Matrix
	Matrix Inverse();

	// -- Static operations

	// Create a Matrix from a Quaternion
	static Matrix CreateFromQuaternion(const Quaternion& Value);
	// Transform a Vector3 by the given Matrix
	static Vector3 TransformVector(const Vector3& Vector, const Matrix& Value);
};

// A class used to handle various packing methods
class VectorPacking
{
public:

	// Unpack a quaternion using method (A)
	static Quaternion QuatPackingA(const uint64_t PackedData);
	// Unpack a quaternion (2d) using method (A)
	static Quaternion QuatPacking2DA(const uint32_t PackedData);
};