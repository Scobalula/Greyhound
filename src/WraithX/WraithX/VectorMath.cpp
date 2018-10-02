#include "stdafx.h"

// The class we are implementing
#include "VectorMath.h"

// -- VectorMath

float VectorMath::DegreesToRadians(float Value)
{
	// Convert
	return (float)((Value * PIValue) / 180.0f);
}

float VectorMath::RadiansToDegrees(float Value)
{
	// Convert
	return (float)(((Value * 180.0f) / PIValue));
}

// -- Vector2

Vector2::Vector2()
{
	// Defaults
	X = 0;
	Y = 0;
}

Vector2::Vector2(float XCoord, float YCoord)
{
	// Set
	X = XCoord;
	Y = YCoord;
}

Vector2::~Vector2()
{
	// Default
}

float& Vector2::operator[](int n)
{
	// Check index
	if (n == 0)
	{
		return X;
	}
	// Default to Y
	return Y;
}

Vector2 Vector2::operator+(const Vector2& Value) const
{
	// Add and make a new result
	return Vector2(X + Value.X, Y + Value.Y);
}

Vector2 Vector2::operator-(const Vector2& Value) const
{
	// Subtract and make a new result
	return Vector2(X - Value.X, Y - Value.Y);
}

Vector2 Vector2::operator*(const Vector2& Value) const
{
	// Multiply and make a new result
	return Vector2(X * Value.X, Y * Value.Y);
}

Vector2 Vector2::operator/(const Vector2& Value) const
{
	// Divide and make a new result
	return Vector2(X / Value.X, Y / Value.Y);
}

Vector2& Vector2::operator+=(const Vector2& Value)
{
	// Add and return ourself
	X += Value.X;
	Y += Value.Y;
	return *this;
}

Vector2& Vector2::operator-=(const Vector2& Value)
{
	// Subtract and return ourself
	X -= Value.X;
	Y -= Value.Y;
	return *this;
}

Vector2& Vector2::operator*=(const Vector2& Value)
{
	// Multiply and return ourself
	X *= Value.X;
	Y *= Value.Y;
	return *this;
}

Vector2& Vector2::operator/=(const Vector2& Value)
{
	// Divide and return ourself
	X /= Value.X;
	Y /= Value.Y;
	return *this;
}

Vector2 Vector2::operator+(float Value) const
{
	// Add then return
	return Vector2(X + Value, Y + Value);
}

Vector2 Vector2::operator-(float Value) const
{
	// Subtract then return
	return Vector2(X - Value, Y - Value);
}

Vector2 Vector2::operator*(float Value) const
{
	// Multiply then return
	return Vector2(X * Value, Y * Value);
}

Vector2 Vector2::operator/(float Value) const
{
	// Divide then return
	return Vector2(X / Value, Y / Value);
}

Vector2& Vector2::operator+=(float Value)
{
	// Add then return us
	X += Value;
	Y += Value;
	return *this;
}

Vector2& Vector2::operator-=(float Value)
{
	// Subtract then return us
	X -= Value;
	Y -= Value;
	return *this;
}

Vector2& Vector2::operator*=(float Value)
{
	// Multiply then return us
	X *= Value;
	Y *= Value;
	return *this;
}

Vector2& Vector2::operator/=(float Value)
{
	// Divide then return us
	X /= Value;
	Y /= Value;
	return *this;
}

bool Vector2::operator==(const Vector2& Value) const
{
	// Ensure our values are within the minumum float value
	return (std::fabs(X - Value.X) < EpsilonValue) && (std::fabs(Y - Value.Y) < EpsilonValue);
}

bool Vector2::operator!=(const Vector2& Value) const
{
	// Compare against equals
	return !(*this == Value);
}

Vector2 Vector2::operator-() const
{
	// Just negate
	return Vector2(-X, -Y);
}

float Vector2::Length() const
{
	// Calculate length
	return (float)std::sqrt(X * X + Y * Y);
}

float Vector2::LengthSq() const
{
	// Calculate length sq
	return X * X + Y * Y;
}

void Vector2::Normalize()
{
	// Normalize it
	float Len = Length();
	// Divide out
	X /= Len;
	Y /= Len;
}

Vector2 Vector2::GetNormalized() const
{
	// Normalize it
	float Len = Length();

	// Return it
	return Vector2(X / Len, Y / Len);
}

Vector2 Vector2::Lerp(float Factor, const Vector2& Value) const
{
	// Interpolate
	return (*this) + (Value - (*this)) * Factor;
}

// -- Vector3

Vector3::Vector3()
{
	// Defaults
	X = 0;
	Y = 0;
	Z = 0;
}

Vector3::Vector3(float XCoord, float YCoord, float ZCoord)
{
	// Set
	X = XCoord;
	Y = YCoord;
	Z = ZCoord;
}

Vector3::~Vector3()
{
	// Default
}

float& Vector3::operator[](int n)
{
	// Check index
	if (n == 0)
	{
		return X;
	}
	else if (n == 1)
	{
		return Y;
	}
	// Default to Z
	return Z;
}

Vector3 Vector3::operator+(const Vector3& Value) const
{
	// Add and make a new result
	return Vector3(X + Value.X, Y + Value.Y, Z + Value.Z);
}

Vector3 Vector3::operator-(const Vector3& Value) const
{
	// Subtract and make a new result
	return Vector3(X - Value.X, Y - Value.Y, Z - Value.Z);
}

Vector3 Vector3::operator*(const Vector3& Value) const
{
	// Multiply and make a new result
	return Vector3(X * Value.X, Y * Value.Y, Z * Value.Z);
}

Vector3 Vector3::operator/(const Vector3& Value) const
{
	// Divide and make a new result
	return Vector3(X / Value.X, Y / Value.Y, Z / Value.Z);
}

Vector3& Vector3::operator+=(const Vector3& Value)
{
	// Add and return ourself
	X += Value.X;
	Y += Value.Y;
	Z += Value.Z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& Value)
{
	// Subtract and return ourself
	X -= Value.X;
	Y -= Value.Y;
	Z -= Value.Z;
	return *this;
}

Vector3& Vector3::operator*=(const Vector3& Value)
{
	// Multiply and return ourself
	X *= Value.X;
	Y *= Value.Y;
	Z *= Value.Z;
	return *this;
}

Vector3& Vector3::operator/=(const Vector3& Value)
{
	// Divide and return ourself
	X /= Value.X;
	Y /= Value.Y;
	Z /= Value.Z;
	return *this;
}

Vector3 Vector3::operator+(float Value) const
{
	// Add then return
	return Vector3(X + Value, Y + Value, Z + Value);
}

Vector3 Vector3::operator-(float Value) const
{
	// Subtract then return
	return Vector3(X - Value, Y - Value, Z - Value);
}

Vector3 Vector3::operator*(float Value) const
{
	// Multiply then return
	return Vector3(X * Value, Y * Value, Z * Value);
}

Vector3 Vector3::operator/(float Value) const
{
	// Divide then return
	return Vector3(X / Value, Y / Value, Z / Value);
}

Vector3& Vector3::operator+=(float Value)
{
	// Add then return us
	X += Value;
	Y += Value;
	Z += Value;
	return *this;
}

Vector3& Vector3::operator-=(float Value)
{
	// Subtract then return us
	X -= Value;
	Y -= Value;
	Z -= Value;
	return *this;
}

Vector3& Vector3::operator*=(float Value)
{
	// Multiply then return us
	X *= Value;
	Y *= Value;
	Z *= Value;
	return *this;
}

Vector3& Vector3::operator/=(float Value)
{
	// Divide then return us
	X /= Value;
	Y /= Value;
	Z /= Value;
	return *this;
}

bool Vector3::operator==(const Vector3& Value) const
{
	// Ensure our values are within the minumum float value
	return (std::fabs(X - Value.X) < EpsilonValue) && (std::fabs(Y - Value.Y) < EpsilonValue) && (std::fabs(Z - Value.Z) < EpsilonValue);
}

bool Vector3::operator!=(const Vector3& Value) const
{
	// Compare against equals
	return !(*this == Value);
}

Vector3 Vector3::operator-() const
{
	// Just negate
	return Vector3(-X, -Y, -Z);
}

float Vector3::Length() const
{
	// Get length sq
	return (float)std::sqrt(X * X + Y * Y + Z * Z);
}

float Vector3::LengthSq() const
{
	// Get length sq
	return X * X + Y * Y + Z * Z;
}

void Vector3::Normalize()
{
	// Normalize it
	float Len = Length();
	// Divide out
	X /= Len;
	Y /= Len;
	Z /= Len;
}

Vector3 Vector3::GetNormalized() const
{
	// Normalize it
	float Len = Length();

	// TODO: ADD TO OTHER VECTOR TYPES

	// Return
	return Vector3(X / Len, Y / Len, Z / Len);
}

Vector3 Vector3::Lerp(float Factor, const Vector3& Value) const
{
	// Interpolate
	return (*this) + (Value - (*this)) * Factor;
}

// -- Matrix

Matrix::Matrix()
{
	// Set to Identity
	for (int i = 0; i < 16; i++) { Data[i] = (i % 5) ? 0.0f : 1.0f; }
}

Matrix::~Matrix()
{
	// Default
}

bool Matrix::operator==(const Matrix& Value) const
{
	// Loop and check each
	for (int i = 0; i < 16; i++)
	{
		if (std::fabs(Data[i] - Value.Data[i]) >= EpsilonValue)
		{
			// Not a match
			return false;
		}
	}
	// They must match
	return true;
}

bool Matrix::operator!=(const Matrix& Value) const
{
	// Check against equals
	return !(*this == Value);
}

Matrix Matrix::operator+(const Matrix& Value) const
{
	// Buffer
	Matrix Result;
	// Loop and add
	for (int i = 0; i < 16; i++) { Result.Data[i] = Data[i] + Value.Data[i]; }
	// Return result
	return Result;
}

Matrix Matrix::operator-(const Matrix& Value) const
{
	// Buffer
	Matrix Result;
	// Loop and add
	for (int i = 0; i < 16; i++) { Result.Data[i] = Data[i] - Value.Data[i]; }
	// Return result
	return Result;
}

Matrix Matrix::operator*(const Matrix& Value) const
{
	// The result buffer
	Matrix Result;
	// Loop for cols
	for (int i = 0; i < 4; i++)
	{
		// Loop for rows
		for (int j = 0; j < 4; j++)
		{
			// Buffer
			float Buffer = 0;
			// Loop
			for (int k = 0; k < 4; k++)
			{
				// Multiply
				Buffer += Value.Mat(i, k) * Mat(k, j);
			}
			// Set
			Result.Mat(i, j) = Buffer;
		}
	}
	// Return result
	return Result;
}

Matrix Matrix::operator/(float Value) const
{
	// Buffer
	Matrix Result;
	// Loop and divide
	for (int i = 0; i < 16; i++) { Result.Data[i] = Data[i] / Value; }
	// Return
	return Result;
}

float& Matrix::Mat(int X, int Y)
{
	// Return it
	return Data[X * 4 + Y];
}

const float& Matrix::Mat(int X, int Y) const
{
	// Return it
	return Data[X * 4 + Y];
}

float Matrix::Determinant()
{
	// Col(1) Compute Row 1
	return Mat(3, 0) * Mat(2, 1) * Mat(1, 2) * Mat(0, 3) - Mat(2, 0) * Mat(3, 1) * Mat(1, 2) * Mat(0, 3)
		// Col(1) Compute Row 2
		- Mat(3, 0) * Mat(1, 1) * Mat(2, 2) * Mat(0, 3) + Mat(1, 0) * Mat(3, 1) * Mat(2, 2) * Mat(0, 3)
		// Col(2) Compute Row 1 & 2
		+ Mat(2, 0) * Mat(1, 1) * Mat(3, 2) * Mat(0, 3) - Mat(1, 0) * Mat(2, 1) * Mat(3, 2) * Mat(0, 3)
		- Mat(3, 0) * Mat(2, 1) * Mat(0, 2) * Mat(1, 3) + Mat(2, 0) * Mat(3, 1) * Mat(0, 2) * Mat(1, 3)
		// Col(3) Compute Row 1 & 2
		+ Mat(3, 0) * Mat(0, 1) * Mat(2, 2) * Mat(1, 3) - Mat(0, 0) * Mat(3, 1) * Mat(2, 2) * Mat(1, 3)
		- Mat(2, 0) * Mat(0, 1) * Mat(3, 2) * Mat(1, 3) + Mat(0, 0) * Mat(2, 1) * Mat(3, 2) * Mat(1, 3)
		// Col(4) Compute Row 1 & 2
		+ Mat(3, 0) * Mat(1, 1) * Mat(0, 2) * Mat(2, 3) - Mat(1, 0) * Mat(3, 1) * Mat(0, 2) * Mat(2, 3)
		- Mat(3, 0) * Mat(0, 1) * Mat(1, 2) * Mat(2, 3) + Mat(0, 0) * Mat(3, 1) * Mat(1, 2) * Mat(2, 3)
		// Col(5) Compute Row 1 & 2
		+ Mat(1, 0) * Mat(0, 1) * Mat(3, 2) * Mat(2, 3) - Mat(0, 0) * Mat(1, 1) * Mat(3, 2) * Mat(2, 3)
		- Mat(2, 0) * Mat(1, 1) * Mat(0, 2) * Mat(3, 3) + Mat(1, 0) * Mat(2, 1) * Mat(0, 2) * Mat(3, 3)
		// Col(6) Compute Row 1 & 2
		+ Mat(2, 0) * Mat(0, 1) * Mat(1, 2) * Mat(3, 3) - Mat(0, 0) * Mat(2, 1) * Mat(1, 2) * Mat(3, 3)
		- Mat(1, 0) * Mat(0, 1) * Mat(2, 2) * Mat(3, 3) + Mat(0, 0) * Mat(1, 1) * Mat(2, 2) * Mat(3, 3);
}

Matrix Matrix::Inverse()
{
	// Buffer
	Matrix Result;

	// Calculate each row and col
	Result.Mat(0, 0) = + Mat(2, 1) * Mat(3, 2) * Mat(1, 3) - Mat(3, 1) * Mat(2, 2) * Mat(1, 3) + Mat(3, 1) * Mat(1, 2) * Mat(2, 3)
		- Mat(1, 1) * Mat(3, 2) * Mat(2, 3) - Mat(2, 1) * Mat(1, 2) * Mat(3, 3) + Mat(1, 1) * Mat(2, 2) * Mat(3, 3);

	Result.Mat(1, 0) = + Mat(3, 0) * Mat(2, 2) * Mat(1, 3) - Mat(2, 0) * Mat(3, 2) * Mat(1, 3) - Mat(3, 0) * Mat(1, 2) * Mat(2, 3)
		+ Mat(1, 0) * Mat(3, 2) * Mat(2, 3) + Mat(2, 0) * Mat(1, 2) * Mat(3, 3) - Mat(1, 0) * Mat(2, 2) * Mat(3, 3);

	Result.Mat(2, 0) = + Mat(2, 0) * Mat(3, 1) * Mat(1, 3) - Mat(3, 0) * Mat(2, 1) * Mat(1, 3) + Mat(3, 0) * Mat(1, 1) * Mat(2, 3)
		- Mat(1, 0) * Mat(3, 1) * Mat(2, 3) - Mat(2, 0) * Mat(1, 1) * Mat(3, 3) + Mat(1, 0) * Mat(2, 1) * Mat(3, 3);

	Result.Mat(3, 0) = + Mat(3, 0) * Mat(2, 1) * Mat(1, 2) - Mat(2, 0) * Mat(3, 1) * Mat(1, 2) - Mat(3, 0) * Mat(1, 1) * Mat(2, 2)
		+ Mat(1, 0) * Mat(3, 1) * Mat(2, 2) + Mat(2, 0) * Mat(1, 1) * Mat(3, 2) - Mat(1, 0) * Mat(2, 1) * Mat(3, 2);

	Result.Mat(0, 1) = + Mat(3, 1) * Mat(2, 2) * Mat(0, 3) - Mat(2, 1) * Mat(3, 2) * Mat(0, 3) - Mat(3, 1) * Mat(0, 2) * Mat(2, 3)
		+ Mat(0, 1) * Mat(3, 2) * Mat(2, 3) + Mat(2, 1) * Mat(0, 2) * Mat(3, 3) - Mat(0, 1) * Mat(2, 2) * Mat(3, 3);

	Result.Mat(1, 1) = + Mat(2, 0) * Mat(3, 2) * Mat(0, 3) - Mat(3, 0) * Mat(2, 2) * Mat(0, 3) + Mat(3, 0) * Mat(0, 2) * Mat(2, 3)
		- Mat(0, 0) * Mat(3, 2) * Mat(2, 3) - Mat(2, 0) * Mat(0, 2) * Mat(3, 3) + Mat(0, 0) * Mat(2, 2) * Mat(3, 3);

	Result.Mat(2, 1) = + Mat(3, 0) * Mat(2, 1) * Mat(0, 3) - Mat(2, 0) * Mat(3, 1) * Mat(0, 3) - Mat(3, 0) * Mat(0, 1) * Mat(2, 3)
		+ Mat(0, 0) * Mat(3, 1) * Mat(2, 3) + Mat(2, 0) * Mat(0, 1) * Mat(3, 3) - Mat(0, 0) * Mat(2, 1) * Mat(3, 3);

	Result.Mat(3, 1) = + Mat(2, 0) * Mat(3, 1) * Mat(0, 2) - Mat(3, 0) * Mat(2, 1) * Mat(0, 2) + Mat(3, 0) * Mat(0, 1) * Mat(2, 2)
		- Mat(0, 0) * Mat(3, 1) * Mat(2, 2) - Mat(2, 0) * Mat(0, 1) * Mat(3, 2) + Mat(0, 0) * Mat(2, 1) * Mat(3, 2);

	Result.Mat(0, 2) = + Mat(1, 1) * Mat(3, 2) * Mat(0, 3) - Mat(3, 1) * Mat(1, 2) * Mat(0, 3) + Mat(3, 1) * Mat(0, 2) * Mat(1, 3)
		- Mat(0, 1) * Mat(3, 2) * Mat(1, 3) - Mat(1, 1) * Mat(0, 2) * Mat(3, 3) + Mat(0, 1) * Mat(1, 2) * Mat(3, 3);

	Result.Mat(1, 2) = + Mat(3, 0) * Mat(1, 2) * Mat(0, 3) - Mat(1, 0) * Mat(3, 2) * Mat(0, 3) - Mat(3, 0) * Mat(0, 2) * Mat(1, 3)
		+ Mat(0, 0) * Mat(3, 2) * Mat(1, 3) + Mat(1, 0) * Mat(0, 2) * Mat(3, 3) - Mat(0, 0) * Mat(1, 2) * Mat(3, 3);

	Result.Mat(2, 2) = + Mat(1, 0) * Mat(3, 1) * Mat(0, 3) - Mat(3, 0) * Mat(1, 1) * Mat(0, 3) + Mat(3, 0) * Mat(0, 1) * Mat(1, 3)
		- Mat(0, 0) * Mat(3, 1) * Mat(1, 3) - Mat(1, 0) * Mat(0, 1) * Mat(3, 3) + Mat(0, 0) * Mat(1, 1) * Mat(3, 3);

	Result.Mat(3, 2) = + Mat(3, 0) * Mat(1, 1) * Mat(0, 2) - Mat(1, 0) * Mat(3, 1) * Mat(0, 2) - Mat(3, 0) * Mat(0, 1) * Mat(1, 2)
		+ Mat(0, 0) * Mat(3, 1) * Mat(1, 2) + Mat(1, 0) * Mat(0, 1) * Mat(3, 2) - Mat(0, 0) * Mat(1, 1) * Mat(3, 2);

	Result.Mat(0, 3) = + Mat(2, 1) * Mat(1, 2) * Mat(0, 3) - Mat(1, 1) * Mat(2, 2) * Mat(0, 3) - Mat(2, 1) * Mat(0, 2) * Mat(1, 3)
		+ Mat(0, 1) * Mat(2, 2) * Mat(1, 3) + Mat(1, 1) * Mat(0, 2) * Mat(2, 3) - Mat(0, 1) * Mat(1, 2) * Mat(2, 3);

	Result.Mat(1, 3) = + Mat(1, 0) * Mat(2, 2) * Mat(0, 3) - Mat(2, 0) * Mat(1, 2) * Mat(0, 3) + Mat(2, 0) * Mat(0, 2) * Mat(1, 3)
		- Mat(0, 0) * Mat(2, 2) * Mat(1, 3) - Mat(1, 0) * Mat(0, 2) * Mat(2, 3) + Mat(0, 0) * Mat(1, 2) * Mat(2, 3);

	Result.Mat(2, 3) = + Mat(2, 0) * Mat(1, 1) * Mat(0, 3) - Mat(1, 0) * Mat(2, 1) * Mat(0, 3) - Mat(2, 0) * Mat(0, 1) * Mat(1, 3)
		+ Mat(0, 0) * Mat(2, 1) * Mat(1, 3) + Mat(1, 0) * Mat(0, 1) * Mat(2, 3) - Mat(0, 0) * Mat(1, 1) * Mat(2, 3);

	Result.Mat(3, 3) = + Mat(1, 0) * Mat(2, 1) * Mat(0, 2) - Mat(2, 0) * Mat(1, 1) * Mat(0, 2) + Mat(2, 0) * Mat(0, 1) * Mat(1, 2)
		- Mat(0, 0) * Mat(2, 1) * Mat(1, 2) - Mat(1, 0) * Mat(0, 1) * Mat(2, 2) + Mat(0, 0) * Mat(1, 1) * Mat(2, 2);

	// Return it (Divide by the determinant)
	return Result / Determinant();
}

Matrix Matrix::CreateFromQuaternion(const Quaternion& Value)
{
	// Buffer
	Matrix Result;

	// Get squared calculations
	float XX = Value.X * Value.X;
	float XY = Value.X * Value.Y;
	float XZ = Value.X * Value.Z;
	float XW = Value.X * Value.W;

	float YY = Value.Y * Value.Y;
	float YZ = Value.Y * Value.Z;
	float YW = Value.Y * Value.W;

	float ZZ = Value.Z * Value.Z;
	float ZW = Value.Z * Value.W;

	// Calculate matrix
	Result.Mat(0, 0) = 1 - 2 * (YY + ZZ);
	Result.Mat(1, 0) = 2 * (XY - ZW);
	Result.Mat(2, 0) = 2 * (XZ + YW);
	Result.Mat(3, 0) = 0;

	Result.Mat(0, 1) = 2 * (XY + ZW);
	Result.Mat(1, 1) = 1 - 2 * (XX + ZZ);
	Result.Mat(2, 1) = 2 * (YZ - XW);
	Result.Mat(3, 1) = 0;

	Result.Mat(0, 2) = 2 * (XZ - YW);
	Result.Mat(1, 2) = 2 * (YZ + XW);
	Result.Mat(2, 2) = 1 - 2 * (XX + YY);
	Result.Mat(3, 2) = 0;

	Result.Mat(0, 3) = 0;
	Result.Mat(1, 3) = 0;
	Result.Mat(2, 3) = 0;
	Result.Mat(3, 3) = 1;

	// Return it
	return Result;
}

Vector3 Matrix::TransformVector(const Vector3& Vector, const Matrix& Value)
{
	// Buffer
	Vector3 Result;

	// Calculate
	Result.X = (Vector.X * Value.Mat(0, 0)) + (Vector.Y * Value.Mat(1, 0)) + (Vector.Z * Value.Mat(2, 0)) + Value.Mat(3, 0);
	Result.Y = (Vector.X * Value.Mat(0, 1)) + (Vector.Y * Value.Mat(1, 1)) + (Vector.Z * Value.Mat(2, 1)) + Value.Mat(3, 1);
	Result.Z = (Vector.X * Value.Mat(0, 2)) + (Vector.Y * Value.Mat(1, 2)) + (Vector.Z * Value.Mat(2, 2)) + Value.Mat(3, 2);

	// Return result
	return Result;
}

// -- Quaternion

Quaternion::Quaternion()
{
	// Defaults (Identity)
	X = 0;
	Y = 0;
	Z = 0;
	W = 1;
}

Quaternion::Quaternion(float XCoord, float YCoord, float ZCoord, float WCoord)
{
	// Set
	X = XCoord;
	Y = YCoord;
	Z = ZCoord;
	W = WCoord;
}

Quaternion::~Quaternion()
{
	// Default
}

float& Quaternion::operator[](int n)
{
	// Check index
	if (n == 0)
	{
		return X;
	}
	else if (n == 1)
	{
		return Y;
	}
	else if (n == 2)
	{
		return Z;
	}
	// Default to W
	return W;
}

Quaternion Quaternion::operator+(const Quaternion& Value) const
{
	// Add and make a new result
	return Quaternion(X + Value.X, Y + Value.Y, Z + Value.Z, W + Value.W);
}

Quaternion Quaternion::operator-(const Quaternion& Value) const
{
	// Add and make a new result
	return Quaternion(X - Value.X, Y - Value.Y, Z - Value.Z, W - Value.W);
}

Quaternion Quaternion::operator*(const Quaternion& Value) const
{
	// Multiply and make a new result
	return Quaternion(
		// Calculate the new X
		W * Value.X + X * Value.W + Y * Value.Z - Z * Value.Y,
		// Calculate the new Y
		W * Value.Y - X * Value.Z + Y * Value.W + Z * Value.X,
		// Calculate the new Z
		W * Value.Z + X * Value.Y - Y * Value.X + Z * Value.W,
		// Calculate the new W
		W * Value.W - X * Value.X - Y * Value.Y - Z * Value.Z);
}

Quaternion& Quaternion::operator+=(const Quaternion& Value)
{
	// Add and return ourself
	X += Value.X;
	Y += Value.Y;
	Z += Value.Z;
	W += Value.W;
	return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion& Value)
{
	// Subtract and return ourself
	X -= Value.X;
	Y -= Value.Y;
	Z -= Value.Z;
	W -= Value.W;
	return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& Value)
{
	// Multiply and return ourself
	auto Result = (*this) * Value;
	// Set
	X = Result.X;
	Y = Result.Y;
	Z = Result.Z;
	W = Result.W;
	return *this;
}

bool Quaternion::operator==(const Quaternion& Value) const
{
	// Check if they match
	return (std::fabs(X - Value.X) < EpsilonValue) && (std::fabs(Y - Value.Y) < EpsilonValue) && (std::fabs(Z - Value.Z) < EpsilonValue) && (std::fabs(W - Value.W) < EpsilonValue);
}

bool Quaternion::operator!=(const Quaternion& Value) const
{
	// Check if they match
	return !(*this == Value);
}

Quaternion Quaternion::operator-() const
{
	// Invert
	return Quaternion(-X, -Y, -Z, -W);
}

Quaternion Quaternion::operator~() const
{
	// Conjugate
	return Quaternion(-X, -Y, -Z, W);
}

float Quaternion::Length() const
{
	// Calculate the length
	return (float)std::sqrt(X * X + Y * Y + Z * Z + W * W);
}

float Quaternion::LengthSq() const
{
	// Calculate the length sq
	return X * X + Y * Y + Z * Z + W * W;
}

void Quaternion::Normalize()
{
	// Normalize it
	auto Len = Length();
	// Divide out
	X /= Len;
	Y /= Len;
	Z /= Len;
	W /= Len;
}

Quaternion Quaternion::GetNormalized() const
{
	// Normalize it
	auto Len = Length();

	// Return it
	return Quaternion(X / Len, Y / Len, Z / Len, W / Len);
}

Vector3 Quaternion::ToEulerAngles() const
{
	// The result
	Vector3 Result;
	// Convert the quaternion to a matrix array
	float Matrix[4][4];
	// Set it up
	{
		float num = X * X + Y * Y + Z * Z + W * W;
		float num2 = 0;
		// Check
		if (num > 0.0)
		{
			num2 = 2.0f / num;
		}
		// Calculate
		float num3 = X * num2;
		float num4 = Y * num2;
		float num5 = Z * num2;
		float num6 = W * num3;
		float num7 = W * num4;
		float num8 = W * num5;
		float num9 = X * num3;
		float num10 = X * num4;
		float num11 = X * num5;
		float num12 = Y * num4;
		float num13 = Y * num5;
		float num14 = Z * num5;
		// Set
		Matrix[0][0] = 1.0f - (num12 + num14);
		Matrix[0][1] = num10 - num8;
		Matrix[0][2] = num11 + num7;
		Matrix[1][0] = num10 + num8;
		Matrix[1][1] = 1.0f - (num9 + num14);
		Matrix[1][2] = num13 - num6;
		Matrix[2][0] = num11 - num7;
		Matrix[2][1] = num13 + num6;
		Matrix[2][2] = 1.0f - (num9 + num12);
		Matrix[3][3] = 1.0f;
	}
	// Now, convert the matrix to euler angles
	{
		uint32_t i = 0, j = 1, k = 2;
		// Temporary buffers
		float TempX = 0, TempY = 0, TempZ = 0;
		// Calculate the square sum
		float SqSum = std::sqrt(Matrix[i][i] * Matrix[i][i] + Matrix[j][i] * Matrix[j][i]);
		// Check against epsilon
		if (SqSum > 0.00016)
		{
			// We aren't basically 0
			TempX = std::atan2(Matrix[k][j], Matrix[k][k]);
			TempY = std::atan2(-Matrix[k][i], SqSum);
			TempZ = std::atan2(Matrix[j][i], Matrix[i][i]);
		}
		else
		{
			// Sum is basically 0
			TempX = std::atan2(-Matrix[j][k], Matrix[j][j]);
			TempY = std::atan2(-Matrix[k][i], SqSum);
			TempZ = 0.0;
		}
		// Convert to resulting degrees
		Result.X = VectorMath::RadiansToDegrees(TempX);
		Result.Y = VectorMath::RadiansToDegrees(TempY);
		Result.Z = VectorMath::RadiansToDegrees(TempZ);
	}
	// Return it
	return Result;
}

Quaternion Quaternion::Inverse() const
{
	// Buffer
	Quaternion Result;

	// Calculate length
	auto LenSq = LengthSq();
	auto HalfLen = 1.0f / LenSq;

	// Set values
	Result.X = -X * HalfLen;
	Result.Y = -Y * HalfLen;
	Result.Z = -Z * HalfLen;
	Result.W = W * HalfLen;

	// Return it
	return Result;
}

Quaternion Quaternion::FromEulerAngles(float XCoord, float YCoord, float ZCoord)
{
	// Get a quaternion from axis
	auto Result = FromAxisRotation(Vector3(1, 0, 0), XCoord) * FromAxisRotation(Vector3(0, 1, 0), YCoord) * FromAxisRotation(Vector3(0, 0, 1), ZCoord);
	// Return us
	return Result;
}

Quaternion Quaternion::FromAxisRotation(Vector3 Axis, float Angle)
{
	// Convert to radians
	auto Rad = VectorMath::DegreesToRadians(Angle);
	// Calculate and retern a new quat
	auto AngleRes = std::sin(Rad / 2.0f);
	auto WCalc = std::cos(Rad / 2.0f);
	// Get vector
	auto Result = Axis * AngleRes;
	// Return
	return Quaternion(Result.X, Result.Y, Result.Z, WCalc);
}

Quaternion Quaternion::Identity()
{
	// Identity
	return Quaternion(0, 0, 0, 1);
}

// -- VectorPacking

Quaternion VectorPacking::QuatPackingA(const uint64_t PackedData)
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

Quaternion VectorPacking::QuatPacking2DA(const uint32_t PackedData)
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