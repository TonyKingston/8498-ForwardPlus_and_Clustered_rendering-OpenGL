#include "Frustum.h"
#include "Matrix4.h"
#include <algorithm>

using namespace NCL::CSC8503;

void NCL::Rendering::Frustum::FromMatrix(const Matrix4& mat) {
	Vector3 xaxis = Vector3(mat.array[0], mat.array[4], mat.array[8]);
	Vector3 yaxis = Vector3(mat.array[1], mat.array[5], mat.array[9]);
	Vector3 zaxis = Vector3(mat.array[2], mat.array[6], mat.array[10]);
	Vector3 waxis = Vector3(mat.array[3], mat.array[7], mat.array[11]);

	// RIGHT
	planes[0] = Plane(waxis - xaxis,
		(mat.array[15] - mat.array[12]), true);
	// LEFT
	planes[1] = Plane(waxis + xaxis,
		(mat.array[15] + mat.array[12]), true);
	// BOTTOM
	planes[2] = Plane(waxis + yaxis,
		(mat.array[15] + mat.array[13]), true);
	// TOP
	planes[3] = Plane(waxis - yaxis,
		(mat.array[15] - mat.array[13]), true);
	// NEAR
	planes[4] = Plane(waxis + zaxis,
		(mat.array[15] + mat.array[14]), true);
	// FAR
	planes[5] = Plane(waxis - zaxis,
		(mat.array[15] - mat.array[14]), true);
}

bool NCL::Rendering::Frustum::IsInsideFrustum(const Vector3& position, float boundingRadius) {
	return std::all_of(planes.begin(), planes.end(), [&](const auto& plane) {
		return plane.SphereInPlane(position, boundingRadius);
	});
}
