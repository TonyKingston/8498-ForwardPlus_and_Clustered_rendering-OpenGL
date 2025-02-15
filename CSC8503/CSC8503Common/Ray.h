#pragma once
#include "Common/Math/Vector3.h"
#include "Common/Math/Plane.h"
#include "../CSC8503Common/Layer.h"
namespace NCL {
	namespace Maths {
		struct RayCollision {
			void*		node;			//Node that was hit
			Vector3		collidedAt;		//WORLD SPACE position of the collision!
			float		rayDistance;

			RayCollision() {
				node			= nullptr;
				rayDistance		= FLT_MAX;
			}
		};

		class Ray {
		public:
			Ray(Vector3 position, Vector3 direction) {
				this->position  = position;
				this->direction = direction;
				layerMask = Layer::Default;
			}
			Ray(Vector3 position, Vector3 direction, int layer) {
				this->position = position;
				this->direction = direction;
				layerMask = layer;
			}
			~Ray(void) {}

			Vector3 GetPosition() const {return position;	}

			Vector3 GetDirection() const {return direction;	}

			int GetLayerMask() const { return layerMask; }

		protected:
			Vector3 position;	//World space position
			Vector3 direction;	//Normalised world space direction
			int layerMask;
		};
	}
}