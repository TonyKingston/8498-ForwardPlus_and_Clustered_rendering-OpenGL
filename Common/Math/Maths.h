/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <algorithm>
#include <type_traits>
#include "MathsFwd.h"
#include "Vector4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4.h"

namespace NCL {
	namespace Maths {

		// This stuff isn't strictly necessary but I wanted practice with macros and type traits.
		//#define MAKE_VECTOR(...) MAKE_VECTOR_IMPL(__VA_ARGS__, Vector4, Vector3, Vector2)(__VA_ARGS__)
		//#define MAKE_VECTOR_IMPL(_2, _3, _4, NAME, ...) NAME
		//#define FORCE_EXPAND(X) X
		////#define NUMARGS(...)  (sizeof((int[]){__VA_ARGS__})/sizeof(int))
		//#define MAKE_VECTOR(...) \
		//	MAKE_VECTOR_IMPL()
		//#define MAKE_VECTOR_IMPL(N) \
		//Vector##N \

		template<typename T> struct IsVector : std::false_type {};
		template <typename T> struct IsVector<TVector3<T>> : std::true_type {};

		#define DECLARE_VECTOR_TYPE_TRAIT(TYPE) \
	    template <> struct IsVector<Vector##TYPE> : std::true_type {};

		DECLARE_VECTOR_TYPE_TRAIT(2);
		DECLARE_VECTOR_TYPE_TRAIT(3);
		DECLARE_VECTOR_TYPE_TRAIT(4);

		#undef DECLARE_VECTOR_TYPE_TRAITS

		//It's pi(ish)...
		static constexpr float		PI = 3.14159265358979323846f;

		//It's pi...divided by 360.0f!
		static constexpr float		PI_OVER_360 = PI / 360.0f;
		static constexpr float RAD_TO_DEG = 180.0f / PI;
		static constexpr float DEG_TO_RAD = PI / 180.0f;

		//Radians to degrees
		inline constexpr float RadiansToDegrees(float rads) {
			return rads * 180.0f / PI;
		};

		//Degrees to radians
		inline constexpr float DegreesToRadians(float degs) {
			//MAKE_VECTOR(1.0f, 2.0f);

			return degs * PI / 180.0f;
		};

		template<class T>
		inline T Clamp(T value, T min, T max) {
			if (value < min) {
				return min;
			}
			if (value > max) {
				return max;
			}
			return value;
		}

		Vector3 Clamp(const Vector3& a, const Vector3&mins, const Vector3& maxs);

		template<class T>
		inline T Lerp(const T& a, const T&b, float by) {
			return (a * (1.0f - by) + b*by);
		}
		
		static constexpr auto AngleBetweenRadians(const	auto& from, const auto& to) {
			using T = std::decay_t<decltype(from.x)>;
			return std::acos(std::clamp(TVector3<T>::Dot(from.Normalised(), to.Normalised()), T(-1.0f), T(1.0f)));
		}

		static constexpr auto AngleBetweenDegrees(const auto& from, const auto& to) {
			return RadiansToDegrees(AngleBetweenRadians(from, to));
		}

		void ScreenBoxOfTri(const Vector3& v0, const Vector3& v1, const Vector3& v2, Vector2& topLeft, Vector2& bottomRight);

		int ScreenAreaOfTri(const Vector3 &a, const Vector3 &b, const Vector3 & c);
		float FloatAreaOfTri(const Vector3 &a, const Vector3 &b, const Vector3 & c);

		float CrossAreaOfTri(const Vector3 &a, const Vector3 &b, const Vector3 & c);
	}
}