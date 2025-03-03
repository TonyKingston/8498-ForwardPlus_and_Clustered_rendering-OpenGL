/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <assert.h>
#include <algorithm>
#include <iostream>
#include "MathsFwd.h"

namespace NCL {
	namespace Maths {

		class Matrix3
		{
		public:
			Matrix3(void);
			Matrix3(float elements[9]);
			Matrix3(const Matrix2 &m4);
			Matrix3(const Matrix4 &m4);
			Matrix3(const Quaternion& quat);
			Matrix3(const Vector3& i, const Vector3& j, const Vector3& k);

			~Matrix3(void);

			//Set all matrix values to zero
			void	ToZero();

			Vector3 GetRow(unsigned int row) const;
			void	SetRow(unsigned int row, const Vector3 &val);

			Vector3 GetColumn(unsigned int column) const;
			void	SetColumn(unsigned int column, const Vector3 &val);

			Vector3 GetDiagonal() const;
			void	SetDiagonal(const Vector3 &in);

			Vector3 ToEuler() const;

			inline Matrix3 Absolute() const {
				Matrix3 m;

				for (int i = 0; i < 9; ++i) {
					m.array[i] = std::abs(array[i]);
				}

				return m;
			}

			inline Matrix3 Transposed() const {
				Matrix3 temp = *this;
				temp.Transpose();
				return temp;
			}

			inline void Transpose() {
				float tempValues[3];

				tempValues[0] = array[3];
				tempValues[1] = array[6];
				tempValues[2] = array[7];

				array[3] = array[1];
				array[6] = array[2];
				array[7] = array[5];

				array[1] = tempValues[0];
				array[2] = tempValues[1];
				array[5] = tempValues[2];
			}

			Vector3 operator*(const Vector3 &v) const;

			inline Matrix3 operator*(const Matrix3 &a) const {
				Matrix3 out;
				out.ToZero();
				for (unsigned int r = 0; r < 3; ++r) {
					for (unsigned int k = 0; k < 3; k++) {
						for (unsigned int c = 0; c < 3; c++) {
							out.m[r][c] += this->m[k][c] * a.m[r][k];
						}
					}
				}
				return out;
			}

			//Creates a rotation matrix that rotates by 'degrees' around the 'axis'
			//Analogous to glRotatef
			static Matrix3 Rotation(float degrees, const Vector3 &axis);

			// Creates a rotation matrix to align vector "heading" to the "axis" 
			static Matrix3 Rotation(const Vector3& heading, const Vector3& axis);

			//Creates a scaling matrix (puts the 'scale' vector down the diagonal)
			//Analogous to glScalef
			static Matrix3 Scale(const Vector3 &scale);

			static Matrix3 FromEuler(const Vector3 &euler);
		public:
			union {
				struct {
					float array[9];
				};
				struct {
					float m[3][3];
				};
				struct {
					float
						m00, m01, m02,
						m10, m11, m12,
						m20, m21, m22;
				};
			};
		};

		//Handy string output for the matrix. Can get a bit messy, but better than nothing!
		inline std::ostream& operator<<(std::ostream& o, const Matrix3& m) {
			o << m.array[0] << "," << m.array[1] << "," << m.array[2] << std::endl;
			o << m.array[3] << "," << m.array[4] << "," << m.array[5] << std::endl;
			o << m.array[6] << "," << m.array[7] << "," << m.array[8];
			return o;
		}

		inline std::istream& operator >> (std::istream& i, Matrix3& m) {
			char ignore;
			i >> std::skipws;
			i >> m.array[0] >> ignore >> m.array[1] >> ignore >> m.array[2];
			i >> m.array[3] >> ignore >> m.array[4] >> ignore >> m.array[5];
			i >> m.array[6] >> ignore >> m.array[7] >> ignore >> m.array[8];

			return i;
		}
	}
}