/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <iostream>
#include "Macros.h"
#include "MathsFwd.h"

namespace NCL {
	namespace Maths {

		class Vector4 {

		public:
			union {
				struct {
					float x;
					float y;
					float z;
					float w;
				};
				struct {
					float r;
					float g;
					float b;
					float a;
				};
				float array[4];
			};

		public:
			Vector4() = default;
			constexpr Vector4(const Vector4& other) = default;
			Vector4&operator=(const Vector4& other) = default;

			constexpr Vector4(float xVal, float yVal, float zVal, float wVal) : x(xVal), y(yVal), z(zVal), w(wVal) {}

			Vector4(const Vector3& v3, float w = 0.0f);
			Vector4(const Vector2& v2, float z = 0.0f, float w = 0.0f);

			~Vector4() = default;

			// Decimal colour value to RGB


#pragma warning( push )
#pragma warning( disable : 4244)
			NCL_INLINE static Vector4 ConvertDecimalColour(int colour) {
				int r = colour >> 16 & 0xff;
				int g = colour >> 8 & 0xff;
				int b = colour & 0xff;
				return Vector4(r, g, b, 1.0f);
			}
#pragma warning( pop ) 

			NCL_INLINE Vector4 Normalised() const {
				return *this / Length();
			}

			NCL_INLINE Vector4& Normalise() {
				return *this = Normalised();
			}

			NCL_INLINE float Length() const {
				return sqrt(LengthSquared());
			}

			NCL_INLINE constexpr float	LengthSquared() const {
				return ((x*x) + (y*y) + (z*z) + (w * w));
			}

			NCL_INLINE constexpr float GetMaxElement() const {
				return std::max(std::max(x, y), std::max(z, w));
			}

			NCL_INLINE constexpr float GetAbsMaxElement() const {
				return std::max(std::max(abs(x), abs(y)), std::max(abs(z), abs(w)));
			}

			static float Dot(const Vector4 &a, const Vector4 &b) {
				return (a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w);
			}

			NCL_INLINE Vector4  operator+(const Vector4  &a) const {
				return Vector4(x + a.x, y + a.y, z + a.z, w + a.w);
			}

			NCL_INLINE Vector4  operator-(const Vector4  &a) const {
				return Vector4(x - a.x, y - a.y, z - a.z, w - a.w);
			}

			NCL_INLINE Vector4  operator-() const {
				return Vector4(-x, -y, -z, -w);
			}

			NCL_INLINE Vector4  operator*(float a)	const {
				return Vector4(x * a, y * a, z * a, w * a);
			}

			NCL_INLINE Vector4  operator*(const Vector4  &a) const {
				return Vector4(x * a.x, y * a.y, z * a.z, w * a.w);
			}

			NCL_INLINE Vector4  operator/(const Vector4  &a) const {
				return Vector4(x / a.x, y / a.y, z / a.z, w / a.w);
			};

			NCL_INLINE Vector4  operator/(float v) const {
				return Vector4(x / v, y / v, z / v, w / v);
			};

			NCL_INLINE constexpr void operator+=(const Vector4  &a) {
				x += a.x;
				y += a.y;
				z += a.z;
				w += a.w;
			}

			NCL_INLINE void operator-=(const Vector4  &a) {
				x -= a.x;
				y -= a.y;
				z -= a.z;
				w -= a.w;
			}


			NCL_INLINE void operator*=(const Vector4  &a) {
				x *= a.x;
				y *= a.y;
				z *= a.z;
				w *= a.w;
			}

			NCL_INLINE void operator/=(const Vector4  &a) {
				x /= a.x;
				y /= a.y;
				z /= a.z;
				w /= a.w;
			}

			NCL_INLINE void operator*=(float f) {
				x *= f;
				y *= f;
				z *= f;
				w *= f;
			}

			NCL_INLINE void operator/=(float f) {
				x /= f;
				y /= f;
				z /= f;
				w /= f;
			}

			NCL_INLINE float operator[](int i) const {
				return array[i];
			}

			NCL_INLINE float& operator[](int i) {
				return array[i];
			}

			NCL_INLINE bool	operator==(const Vector4 &A)const { return (A.x == x && A.y == y && A.z == z && A.w == w) ? true : false; };
			NCL_INLINE bool	operator!=(const Vector4 &A)const { return (A.x == x && A.y == y && A.z == z && A.w == w) ? false : true; };

			NCL_INLINE friend std::ostream& operator<<(std::ostream& o, const Vector4& v) {
				o << "Vector4(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")" << std::endl;
				return o;
			}
		};

		static Vector4 COLOUR_BLACK(0, 0, 0, 1.0f);
		static Vector4 COLOUR_WHITE(1, 1, 1, 1.0f);
		static Vector4 HOMOGENEOUS(0, 0, 0, 1.0f);
	}
}