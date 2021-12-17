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

		private:
			int magicCount;
			
		};
	}
}
