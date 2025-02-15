#pragma once
#include <array>
#include "Plane.h"

namespace NCL {

	namespace Maths {
		class Matrix4;
		class Plane;
	}

	namespace CSC8503 {
		class RenderObject;
		class Transform;
	}

	using namespace Maths;
	using namespace CSC8503;

	namespace Rendering {

		class Frustum {
		public:
			Frustum() = default;
			~Frustum() = default;

			void FromMatrix(const Matrix4& mvp);
			bool IsInsideFrustum(const Vector3& position, float boundingRadius);
		protected:
			static constexpr size_t PlaneCount = 6;
			std::array<Plane, PlaneCount> planes;
		};

	}
}

