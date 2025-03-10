/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

#include <iostream>
#include "MathsFwd.h"

namespace NCL {
	namespace Maths {

		class Matrix4 {
		public:
			Matrix4();
			Matrix4(float elements[16]);
			Matrix4(const Matrix3& m3);
			Matrix4(const Quaternion& quat);
			Matrix4(const Vector4& i, const Vector4& j, const Vector4& k, const Vector4& l);

			~Matrix4(void);

			union {
				struct {
					float array[16];
				};
				struct {
					float m[4][4];
				};
				struct {
					float
						m00, m01, m02, m03,
						m10, m11, m12, m13,
						m20, m21, m22, m23,
						m30, m31, m32, m33;
				};
			};

			//Set all matrix values to zero
			void	ToZero();
			void    ToIdentity();

			//Gets the OpenGL position vector (floats 12,13, and 14)
			Vector3 GetPositionVector() const;
			//Sets the OpenGL position vector (floats 12,13, and 14)
			void	SetPositionVector(const Vector3 &in);

			//Gets the scale vector (floats 1,5, and 10)
			Vector3 GetDiagonal() const;
			//Sets the scale vector (floats 1,5, and 10)
			void	SetDiagonal(const Vector3& in);

			//Creates a rotation matrix that rotates by 'degrees' around the 'axis'
			//Analogous to glRotatef
			static Matrix4 Rotation(float degrees, const Vector3& axis);
			// Creates a rotation matrix to align vector "heading" to the "axis" 
			static Matrix4 Rotation(const Vector3& heading, const Vector3& axis);

			//Creates a scaling matrix (puts the 'scale' vector down the diagonal)
			//Analogous to glScalef
			static Matrix4 Scale(const Vector3& scale);

			//Creates a translation matrix (identity, with 'translation' vector at
			//floats 12, 13, and 14. Analogous to glTranslatef
			static Matrix4 Translation(const Vector3& translation);

			//Creates a perspective matrix, with 'znear' and 'zfar' as the near and 
			//far planes, using 'aspect' and 'fov' as the aspect ratio and vertical
			//field of vision, respectively.
			static Matrix4 Perspective(float znear, float zfar, float aspect, float fov);

			//Creates an orthographic matrix with 'znear' and 'zfar' as the near and 
			//far planes, and so on. Descriptive variable names are a good thing!
			static Matrix4 Orthographic(float znear, float zfar, float right, float left, float top, float bottom);

			//Builds a view matrix suitable for sending straight to the vertex shader.
			//Puts the camera at 'from', with 'lookingAt' centered on the screen, with
			//'up' as the...up axis (pointing towards the top of the screen)
			static Matrix4 BuildViewMatrix(const Vector3& from, const Vector3& lookingAt, const Vector3& up);

			void    Invert();
			Matrix4 Inverse() const;


			Vector4 GetRow(unsigned int row) const;
			Vector4 GetColumn(unsigned int column) const;

			//Multiplies 'this' matrix by matrix 'a'. Performs the multiplication in 'OpenGL' order (ie, backwards)
			inline Matrix4 operator*(const Matrix4& a) const {
				// TODO: Default constructor should probably init everything to 0 instead of being identity matrix
				Matrix4 out;
				out.ToZero();
				for (unsigned int r = 0; r < 4; ++r) {
					for (unsigned int k = 0; k < 4; k++) {
						for (unsigned int c = 0; c < 4; c++) {
							out.m[r][c] += this->m[k][c] * a.m[r][k];
						}
					}
				}
				return out;
			}

			Vector3 operator*(const Vector3& v) const;
			Vector4 operator*(const Vector4& v) const;

			//Handy string output for the matrix. Can get a bit messy, but better than nothing!
			inline friend std::ostream& operator<<(std::ostream& o, const Matrix4& m) {
				o << "Mat4(";
				o << "\t" << m.array[0] << "," << m.array[1] << "," << m.array[2] << "," << m.array[3] << std::endl;
				o << "\t\t" << m.array[4] << "," << m.array[5] << "," << m.array[6] << "," << m.array[7] << std::endl;
				o << "\t\t" << m.array[8] << "," << m.array[9] << "," << m.array[10] << "," << m.array[11] << std::endl;
				o << "\t\t" << m.array[12] << "," << m.array[13] << "," << m.array[14] << "," << m.array[15] << " )" << std::endl;
				return o;
			}
		};
	}
}
