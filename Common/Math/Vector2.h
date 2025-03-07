/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <iostream>
#include "MathsFwd.h"
#include "Macros.h"

namespace NCL {
	namespace Maths {
		class Vector2 {
		public:
			union {
				struct {
					float x;
					float y;
				};
				struct {
					float r;
					float g;
				};
				float array[2];
			};

		public:
			Vector2() = default;
			constexpr Vector2(const Vector2& other) = default;
			Vector2& operator=(const Vector2 & other) = default;
		
			NCL_INLINE constexpr Vector2(float xVal, float yVal) : x(xVal), y(yVal){}

			Vector2(const Vector3& v3);
			Vector2(const Vector4& v4);

			~Vector2() = default;

			Vector2 Normalised() const {
				return *this / Length();
			}

			void Normalise() {
				*this = Normalised();
			}

			NCL_INLINE float	Length() const {
				return sqrt(LengthSquared());
			}

			NCL_INLINE constexpr float	LengthSquared() const {
				return ((x*x) + (y*y));
			}

			inline constexpr float		GetMaxElement() const {
				return std::max(x, y);
			}

			inline constexpr float		GetAbsMaxElement() const {
				return std::max(abs(x), abs(y));
			}

			inline static constexpr float	Dot(const Vector2 &a, const Vector2 &b) {
				return (a.x*b.x) + (a.y*b.y);
			}

			NCL_INLINE constexpr Vector2  operator+(const Vector2  &a) const {
				return Vector2(x + a.x, y + a.y);
			}

			NCL_INLINE constexpr Vector2  operator-(const Vector2  &a) const {
				return Vector2(x - a.x, y - a.y);
			}

			NCL_INLINE constexpr Vector2  operator-() const {
				return Vector2(-x, -y);
			}

			NCL_INLINE constexpr Vector2  operator*(float a)	const {
				return Vector2(x * a, y * a);
			}

			NCL_INLINE constexpr Vector2  operator*(const Vector2  &a) const {
				return Vector2(x * a.x, y * a.y);
			}

			NCL_INLINE constexpr Vector2  operator/(const Vector2  &a) const {
				return Vector2(x / a.x, y / a.y);
			};

			NCL_INLINE constexpr Vector2  operator/(float v) const {
				return Vector2(x / v, y / v);
			};

			NCL_INLINE constexpr void operator+=(const Vector2  &a) {
				x += a.x;
				y += a.y;
			}

			NCL_INLINE constexpr void operator-=(const Vector2  &a) {
				x -= a.x;
				y -= a.y;
			}

			NCL_INLINE constexpr void operator*=(const Vector2  &a) {
				x *= a.x;
				y *= a.y;
			}

			NCL_INLINE constexpr void operator/=(const Vector2  &a) {
				x /= a.x;
				y /= a.y;
			}

			NCL_INLINE constexpr void operator*=(float f) {
				x *= f;
				y *= f;
			}

			NCL_INLINE constexpr void operator/=(float f) {
				x /= f;
				y /= f;
			}

			NCL_INLINE float operator[](int i) const {
				return array[i];
			}

			NCL_INLINE float& operator[](int i) {
				return array[i];
			}

			NCL_INLINE bool	operator==(const Vector2 &A)const { return (A.x == x && A.y == y) ? true : false; };
			NCL_INLINE bool	operator!=(const Vector2 &A)const { return (A.x == x && A.y == y) ? false : true; };

			NCL_INLINE friend std::ostream& operator<<(std::ostream& o, const Vector2& v) {
				o << "Vector2(" << v.x << "," << v.y << ")" << std::endl;
				return o;
			}
		};
	}
}
