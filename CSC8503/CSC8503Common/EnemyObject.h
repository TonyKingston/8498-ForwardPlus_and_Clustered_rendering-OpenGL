#pragma once
#include "GameObject.h"
#include "BehaviourNodeWithChildren.h"
namespace NCL {
	namespace CSC8503 {
		class EnemyObject : public GameObject {
		public:
			EnemyObject();
			~EnemyObject() {
				delete node;
			}

			bool UpdateObject(float dt) override;

			bool FindPath(Vector3 dest);
			void DisplayPathfinding();


		private:

			BehaviourNode* node;
			std::vector<Vector3> path;
			void OnCollisionBegin(GameObject* otherObj) override;

			const Vector3 defaultSpawnPos = Vector3(160, 10, 130);
		};
	}
}

