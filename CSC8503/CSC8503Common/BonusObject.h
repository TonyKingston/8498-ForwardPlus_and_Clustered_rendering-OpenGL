#pragma once
#include "GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class BonusObject : public GameObject {
		public:
			BonusObject(int val) : GameObject("bonus") {
				score = val;
				layer = Layer::Player;
			}
			~BonusObject();

			int GetScore() { return score; }
			int SetScore(int val) { score = val; }
		private:
			void OnCollisionBegin(GameObject* otherObj) override;
			int score;
		};
	}
}
