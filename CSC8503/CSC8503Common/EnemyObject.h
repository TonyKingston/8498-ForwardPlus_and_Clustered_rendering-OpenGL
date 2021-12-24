#pragma once
#include "GameObject.h"
#include "BehaviourNodeWithChildren.h"
#include "BehaviourSequence.h"
#include "PlayerObject.h"
#include "NavigationGrid.h"
#include "BonusObject.h"

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
			void SetNavigationGrid(NavigationGrid* grid) {navGrid = grid; }
			void SetBonusPositions(vector<Vector3> pos) { bonuses = pos; }
			int GetScore() { return score; }
			void AddScore(int val) { score += val; }


		private:
			PlayerObject* player;
			vector<Vector3> bonuses;
			BehaviourSequence* rootSequence;
			std::vector<Vector3> path;
			void OnCollisionEnd(GameObject* otherObj) override;

			const Vector3 defaultSpawnPos = Vector3(160, 10, 130);
			Vector3 targetDir;
			const float threshold = 100.0f;
			float speed;
			int score;
			bool getBonus = false;
			NavigationGrid* navGrid;
		};
	}
}

