/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "Vector2.h"
#include "Vector4.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <type_traits>

namespace NCL {
	namespace Maths {

		template <typename T>
		class TVector3 {
		static_assert(std::is_trivial<T>(), "Only trivial types should be used with vectors");
		public:
			union {
				struct {
					T x;
					T y;
					T z;
				};
				struct {
					T r;
					T g;
					T b;
				};
				T array[3];
			};
		public:
			TVector3() = default;
			constexpr TVector3(const TVector3& other) = default;
			TVector3& operator=(const TVector3& other) = default;

			NCL_INLINE constexpr TVector3(T xVal, T yVal, T zVal) : x(xVal), y(yVal), z(zVal) {}

			explicit NCL_INLINE TVector3(const Vector2& v2, T newZ = 0.0f) : x(v2.x), y(v2.y), z(newZ) {};
			NCL_INLINE TVector3(const Vector4& v4) : x(v4.x), y(v4.y), z(v4.z) {};

			~TVector3() = default;

			NCL_INLINE TVector3 Normalised() const {
				return *this / Length();
			}

			NCL_INLINE TVector3& Normalise() {
				*this = Normalised();
				return *this;
			}

			//NCL_INLINE void NormaliseSafe() {
			//	// TODO: NormaliseSafe
			//}
			
			// TODO: sqrt is constexpr in c++26
			NCL_INLINE T Length() const {
				return sqrt(LengthSquared());
			}

			NCL_INLINE constexpr T	LengthSquared() const {
				return ((x*x) + (y*y) + (z*z));
			}

			NCL_INLINE constexpr T	GetMaxElement() const {
				return std::max(std::max(x, y), z);
			}

			NCL_INLINE constexpr T GetAbsMaxElement() const {
				return std::max(std::max(abs(x), abs(y)), abs(z));
			}

			NCL_INLINE static constexpr T	Dot(const TVector3 &a, const TVector3 &b) {
				return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
			}

			NCL_INLINE static TVector3	Cross(const TVector3 &a, const TVector3 &b) {
				return TVector3((a.y*b.z) - (a.z*b.y), (a.z*b.x) - (a.x*b.z), (a.x*b.y) - (a.y*b.x));
			}

			NCL_INLINE static TVector3 Lerp(const TVector3& a, const TVector3& b, float t) {
				return (a * t) + (b * (1.0f - t));
			}

			NCL_INLINE constexpr TVector3  operator+(const TVector3  &a) const {
				return TVector3(x + a.x, y + a.y, z + a.z);
			}

			NCL_INLINE constexpr TVector3  operator-(const TVector3  &a) const {
				return TVector3(x - a.x, y - a.y, z - a.z);
			}

			NCL_INLINE constexpr TVector3  operator-() const {
				return TVector3(-x, -y, -z);
			}

			NCL_INLINE constexpr TVector3  operator*(T a)	const {
				return TVector3(x * a, y * a, z * a);
			}

			NCL_INLINE constexpr TVector3  operator*(const TVector3  &a) const {
				return TVector3(x * a.x, y * a.y, z * a.z);
			}

			NCL_INLINE constexpr TVector3  operator/(const TVector3  &a) const {
				return TVector3(x / a.x, y / a.y, z / a.z);
			};

			NCL_INLINE constexpr TVector3  operator/(T v) const {
				return TVector3(x / v, y / v, z / v);
			};

			NCL_INLINE constexpr void operator+=(const TVector3  &a) {
				x += a.x;
				y += a.y;
				z += a.z;
			}

			NCL_INLINE void operator-=(const TVector3  &a) {
				x -= a.x;
				y -= a.y;
				z -= a.z;
			}

			NCL_INLINE void operator*=(const TVector3  &a) {
				x *= a.x;
				y *= a.y;
				z *= a.z;
			}

			NCL_INLINE void operator/=(const TVector3  &a) {
				x /= a.x;
				y /= a.y;
				z /= a.z;
			}

			NCL_INLINE void operator*=(T f) {
				x *= f;
				y *= f;
				z *= f;
			}

			NCL_INLINE void operator/=(T f) {
				x /= f;
				y /= f;
				z /= f;
			}

			NCL_INLINE T operator[](int i) const {
				return array[i];
			}

			NCL_INLINE T& operator[](int i) {
				return array[i];
			}

			NCL_INLINE bool	operator==(const TVector3 &A)const { return (A.x == x && A.y == y && A.z == z) ? true : false; };
			NCL_INLINE bool	operator!=(const TVector3 &A)const { return (A.x == x && A.y == y && A.z == z) ? false : true; };

			NCL_INLINE friend std::ostream& operator<<(std::ostream& o, const TVector3& v) {
				o << "TVector3(" << v.x << "," << v.y << "," << v.z << ")" << std::endl;
				return o;
			}
		};
	}
}
