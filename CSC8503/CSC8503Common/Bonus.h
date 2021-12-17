#pragma once
#include "GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class Bonus : public GameObject {
		public:
			Bonus(int val) {
				score = val;
			}
			~Bonus();

			int GetScore() { return score; }
			int SetScore(int val) { score = val; }
		private:
			int score;
		};
	}
}
