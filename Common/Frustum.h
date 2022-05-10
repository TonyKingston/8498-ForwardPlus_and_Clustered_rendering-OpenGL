#pragma once
#include "Plane.h"

namespace NCL {

	namespace Maths {
		class Matrix4;
		class Plane;
	}

	namespace CSC8503 {
		class RenderObject;
	}

	using namespace Maths;
	using namespace CSC8503;

	namespace Rendering {

		class Frustum {
		public:
			Frustum(void) {};
			~Frustum(void) {};

			void FromMatrix(const Matrix4& mvp);
			bool InsideFrustum(RenderObject& n);
		protected:
			Plane planes[6];
		};

	}
}

