#pragma once
#include "CollisionVolume.h"
#include "Common/Math/Vector3.h"
namespace NCL {
	class OBBVolume : public CollisionVolume
	{
	public:
		OBBVolume(const Maths::Vector3& halfDims) {
			type		= VolumeType::OBB;
			halfSizes	= halfDims;
		}
		~OBBVolume() {}

		Maths::Vector3 GetHalfDimensions() const {
			return halfSizes;
		}

		void DrawVolume(const Vector3& position, const Vector4& colour = Vector4(1, 0, 0, 1)) override {
			Quaternion orientation = collisionObj->GetTransform().GetOrientation();

			static const Vector3 faces[3] =
			{
				Vector3(1, 0, 0),
				Vector3(0, 1, 0),
				Vector3(0, 0, 1)
			};

			Vector3 axis[3];

			for (int i = 0; i < 3; i++) {
				axis[i] = (orientation * faces[i]);
			}

			
		//	Matrix4 matrix = collisionObj->GetTransform().GetMatrix();
			
			Debug::DrawLine(DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z)) ,
				colour);

			Debug::DrawLine
			(
				DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z)),
				colour
			);

			Debug::DrawLine
			(
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z)),
				colour
			);


			Debug::DrawLine
			(
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z)),
				colour
			);


			Debug::DrawLine
			(
				DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z)),
				DebugCalculateVertex(position,Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z)),
				colour
			);

			Debug::DrawLine
			(
				DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z)),
				DebugCalculateVertex(position,Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z)),
				colour
			);

			Debug::DrawLine
			(
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z)),
				colour
			);
			Debug::DrawLine
			(
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z)),
				colour
			);


			Debug::DrawLine(
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z + halfSizes.z)),
				colour
			);

			Debug::DrawLine
			(
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z + halfSizes.z)),
				colour
			);

			Debug::DrawLine(
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y - halfSizes.y, position.z - halfSizes.z)),
				colour
			);

			Debug::DrawLine
			(
				DebugCalculateVertex(position, Vector3(position.x - halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z)),
				DebugCalculateVertex(position, Vector3(position.x + halfSizes.x, position.y + halfSizes.y, position.z - halfSizes.z)),
				colour
			);
		}
	protected:
		Vector3 DebugCalculateVertex(const Vector3 pos, Vector3 vec) {
			Quaternion orientation = collisionObj->GetTransform().GetOrientation();
			return (orientation * (vec - pos)) + pos;
		}
		Maths::Vector3 halfSizes;
	};
}

