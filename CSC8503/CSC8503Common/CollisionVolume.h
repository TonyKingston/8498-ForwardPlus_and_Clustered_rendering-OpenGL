#pragma once
#include "../../Common/Vector4.h"
#include "../../Common/MeshGeometry.h"

namespace NCL {

	enum class VolumeType {
		AABB = 1,
		OBB = 2,
		Sphere = 4,
		Mesh = 8,
		Capsule = 16,
		Compound = 32,
		Invalid = 256
	};

	class CollisionVolume
	{
	public:
		CollisionVolume() {
			type = VolumeType::Invalid;
			mesh = NULL;
		}
		~CollisionVolume() {}

		VolumeType type;

		virtual void DrawVolume(const Vector3& position, const Vector4& colour = Vector4(0, 0, 1, 1)) {};
		void SetVolumeMesh(MeshGeometry* m) { mesh = m; }
		MeshGeometry* GetVolumeMesh() { return mesh; }
	private:
		MeshGeometry* mesh;
	};
}