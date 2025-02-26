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

namespace NCL {
	namespace Maths {

		template <typename T>
		class TVector3 {
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
			constexpr TVector3(void) : x(0.0f), y(0.0f), z(0.0f) {}

			constexpr TVector3(T xVal, T yVal, T zVal) : x(xVal), y(yVal), z(zVal) {}

			TVector3(const Vector2& v2, T newZ = 0.0f) : x(v2.x), y(v2.y), z(newZ) {};
			TVector3(const Vector4& v4) : x(v4.x), y(v4.y), z(v4.z) {};

			~TVector3(void) = default;

			TVector3 Normalised() const {
				TVector3 temp(x, y, z);
				temp.Normalise();
				return temp;
			}

			void Normalise() {
				T length = Length();

				if (length != 0.0f) {
					length = 1.0f / length;
					x = x * length;
					y = y * length;
					z = z * length;
				}
			}

			T Length() const {
				return sqrt((x*x) + (y*y) + (z*z));
			}

			constexpr T	LengthSquared() const {
				return ((x*x) + (y*y) + (z*z));
			}

			constexpr T	GetMaxElement() const {
				T v = x;
				if (y > v) {
					v = y;
				}
				if (z > v) {
					v = z;
				}
				return v;
			}

			T GetAbsMaxElement() const {
				T v = abs(x);
				if (abs(y) > v) {
					v = abs(y);
				}
				if (abs(z) > v) {
					v = abs(z);
				}
				return v;
			}

			static constexpr T	Dot(const TVector3 &a, const TVector3 &b) {
				return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
			}

			static TVector3	Cross(const TVector3 &a, const TVector3 &b) {
				return TVector3((a.y*b.z) - (a.z*b.y), (a.z*b.x) - (a.x*b.z), (a.x*b.y) - (a.y*b.x));
			}

			static TVector3 Lerp(const TVector3& a, const TVector3& b, float t) {
				return (a * t) + (b * (1.0f - t));
			}

			inline TVector3  operator+(const TVector3  &a) const {
				return TVector3(x + a.x, y + a.y, z + a.z);
			}

			inline TVector3  operator-(const TVector3  &a) const {
				return TVector3(x - a.x, y - a.y, z - a.z);
			}

			inline TVector3  operator-() const {
				return TVector3(-x, -y, -z);
			}

			inline TVector3  operator*(T a)	const {
				return TVector3(x * a, y * a, z * a);
			}

			inline TVector3  operator*(const TVector3  &a) const {
				return TVector3(x * a.x, y * a.y, z * a.z);
			}

			inline TVector3  operator/(const TVector3  &a) const {
				return TVector3(x / a.x, y / a.y, z / a.z);
			};

			inline constexpr TVector3  operator/(T v) const {
				return TVector3(x / v, y / v, z / v);
			};

			inline constexpr void operator+=(const TVector3  &a) {
				x += a.x;
				y += a.y;
				z += a.z;
			}

			inline void operator-=(const TVector3  &a) {
				x -= a.x;
				y -= a.y;
				z -= a.z;
			}

			inline void operator*=(const TVector3  &a) {
				x *= a.x;
				y *= a.y;
				z *= a.z;
			}

			inline void operator/=(const TVector3  &a) {
				x /= a.x;
				y /= a.y;
				z /= a.z;
			}

			inline void operator*=(T f) {
				x *= f;
				y *= f;
				z *= f;
			}

			inline void operator/=(T f) {
				x /= f;
				y /= f;
				z /= f;
			}

			inline T operator[](int i) const {
				return array[i];
			}

			inline T& operator[](int i) {
				return array[i];
			}

			inline bool	operator==(const TVector3 &A)const { return (A.x == x && A.y == y && A.z == z) ? true : false; };
			inline bool	operator!=(const TVector3 &A)const { return (A.x == x && A.y == y && A.z == z) ? false : true; };

			inline friend std::ostream& operator<<(std::ostream& o, const TVector3& v) {
				o << "TVector3(" << v.x << "," << v.y << "," << v.z << ")" << std::endl;
				return o;
			}
		};
	}
}
