#pragma once
#include "PositionConstraint.h"
namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class PositionOrientationConstraint : public PositionConstraint
		{
		public:
			PositionOrientationConstraint(GameObject* a, GameObject* b, float d ) : PositionConstraint(a, b, d) {


			}
			~PositionOrientationConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:

		};
	}
}

