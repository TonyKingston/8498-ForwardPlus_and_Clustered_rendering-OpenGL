#pragma once
#include "GameObject.h"
#include "PlayerObject.h"
namespace NCL {
	namespace CSC8503 {
		class JumpPadObject : public GameObject {
		public:
			JumpPadObject(PlayerObject* player) : GameObject("jump pad"){
				this->target = target;
				this->player = player;
				force = Vector3(200.0f, 300.0f, 200.0f);
			}
			~JumpPadObject();
			void OnClick() override;
			bool UpdateObject(float dt) override;
			void SetRange(float r) { range = r; }
			void SetTarget(Vector3 target) {
				this->target = target;
				direction = (target - this->GetTransform().GetPosition()).Normalised();
			}
		protected:
			Vector3 target;
			PlayerObject* player;
			Vector3 direction;
			float range;
			Vector3 force;
		};
	}
}


