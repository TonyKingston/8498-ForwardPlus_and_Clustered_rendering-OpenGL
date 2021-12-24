#pragma once
#include "GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class PlayerObject : public GameObject {
		public:
			PlayerObject();
			~PlayerObject();

			void OnCollisionBegin(GameObject* otherObject) override;
			void OnCollisionEnd(GameObject* otherObject) override;
			int GetMagicCount() { return magicCount; }
			int GetScore() { return score; }
			int GetLives() { return lives; }
			void AddScore(int val) { score += val; }
			void TakeLife() { lives -= 1;}

		private:
			int magicCount;
			int score;
			int lives;
			
		};
	}
}
