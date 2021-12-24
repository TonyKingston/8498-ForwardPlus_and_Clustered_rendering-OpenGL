#pragma once
#include "PositionConstraint.h"
#include "../../Common/Vector3.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class PositionOrientationConstraint : public PositionConstraint
		{
		public:
			PositionOrientationConstraint(GameObject* a, GameObject* b, float d, Vector3 axis) : PositionConstraint(a, b, d) {
				this->axis = axis;
			}
			~PositionOrientationConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			Vector3 axis; // Axis to constrain orientation on;
		};
	}
}

