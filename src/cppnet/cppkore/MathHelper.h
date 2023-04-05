#pragma once

#include <cstdint>

namespace Math
{
	class MathHelper
	{
	public:
		// Converts the input degrees to radian units
		constexpr static float DegreesToRadians(float Value)
		{
			return (float)((Value * MathHelper::PI) / 180.0f);
		}
		// Converts the input radians to degree units
		constexpr static float RadiansToDegrees(float Value)
		{
			return (float)(((Value * 180.0f) / MathHelper::PI));
		}

		template <typename T>
		// Clamp a number between two bounds
		constexpr static T Clamp(const T& Value, const T& Lower, const T& Upper)
		{
			// Clamp it
			return max(Lower, min(Value, Upper));
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

		// The value of PI, a constant
		constexpr static double PI = 3.14159265358979323846;
		// The value of PI * 2, a constant
		constexpr static double PI2 = (PI * 2);
		// The epsilon value for comparisions
		constexpr static double Epsilon = 4.37114e-05;
	};
}