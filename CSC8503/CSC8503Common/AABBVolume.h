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

		void DrawVolume(const Vector3& position, const Vector4& colour = Vector4(1, 0, 0, 1)) override{
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
		}
	protected:
		Vector3 halfSizes;
	};
}
