#pragma once
#include "GameObject.h"
#include "BehaviourNodeWithChildren.h"
#include "BehaviourSequence.h"
#include "PlayerObject.h"
#include "NavigationGrid.h"
namespace NCL {
	namespace CSC8503 {
		class EnemyObject : public GameObject {
		public:
			EnemyObject(PlayerObject* player);
			~EnemyObject() {
				delete rootSequence;
			}

			void CreateBehaviourTree();
			bool UpdateObject(float dt) override;

			bool FindPath(Vector3 dest);
			void FollowPath();
			void DisplayPathfinding();
			void SetNavigationGrid(NavigationGrid* grid) { navGrid = grid; }


		private:
			PlayerObject* player;
			BehaviourSequence* rootSequence;
			std::vector<Vector3> path;
			void OnCollisionBegin(GameObject* otherObj) override;

			const Vector3 defaultSpawnPos = Vector3(160, 10, 130);
			const float threshold = 10.0f;
			float speed;
			NavigationGrid* navGrid;
		};
	}
}

