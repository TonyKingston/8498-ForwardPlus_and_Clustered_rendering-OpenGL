#pragma once
#include "CollisionVolume.h"
#include "../../Common/Vector4.h"
#include "Debug.h"

namespace NCL {
	class AABBVolume : public CollisionVolume
	{
	public:
		AABBVolume(const Vector3& halfDims) {
			type		= VolumeType::AABB;
			halfSizes	= halfDims;
		}
		~AABBVolume() {
		
		}

		Vector3 GetHalfDimensions() const {
			return halfSizes;
		}

		virtual void DrawVolume(const Vector3& position, const Vector4& colour = Vector4(1, 0, 0, 1)) {
			Debug::DrawLine(Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z),
				Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z),
				colour);
			
			Debug::DrawLine
			(
				Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z),
				Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z),
				colour
			);

			Debug::DrawLine
			(
				Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z),
				Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z),
				colour
			);


			Debug::DrawLine
			(
				Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z),
				Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z),
				colour
			);

			
			Debug::DrawLine
			(
				Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z),
				Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z),
				colour
			);

			Debug::DrawLine
			(
				Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z),
				Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z),
				colour
			);

			Debug::DrawLine
			(
				Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z),
				Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z),
				colour
			);
			Debug::DrawLine
			(
				Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z),
				Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z),
				colour
			);

		
			Debug::DrawLine(
				Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z),
				Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z),
				colour
			);

			Debug::DrawLine
			(
				Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z),
				Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z),
				colour
			);

			Debug::DrawLine(
				Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z),
				Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z),
				colour
			);

			Debug::DrawLine
			(
				Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z),
				Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z),
				colour
			);

			// Use a small line to draw centre of AABB
			Debug::DrawLine(Vector3(position.x, position.y, position.z),
				Vector3(position.x + 0.01, position.y + 0.01, position.z + 0.01),
				colour);
		}
	protected:
		Vector3 halfSizes;
	};
}
