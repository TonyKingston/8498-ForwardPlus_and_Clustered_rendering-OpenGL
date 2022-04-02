#pragma once
#include <vector>
#include "Ray.h"
#include "CollisionDetection.h"

namespace NCL {
		class Camera;
		using Maths::Ray;
	namespace CSC8503 {
		class GameObject;
		class Constraint;
		class PhysicsSystem;

		typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

		class GameWorld	{
		public:
			GameWorld();
			GameWorld(PhysicsSystem* physics);
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o, bool andDelete = false);

			void AddConstraint(Constraint* c);
			void RemoveConstraint(Constraint* c, bool andDelete = false);

			Camera* GetMainCamera() const {
				return mainCamera;
			}

			void ShuffleConstraints(bool state) {
				shuffleConstraints = state;
			}

			void ShuffleObjects(bool state) {
				shuffleObjects = state;
			}

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false) const;

			virtual void UpdateWorld(float dt);

			void DrawBoundingVolumes();

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(
				GameObjectIterator& first,
				GameObjectIterator& last, bool isStatic = false) const;

			void GetConstraintIterators(
				std::vector<Constraint*>::const_iterator& first,
				std::vector<Constraint*>::const_iterator& last) const;

			void SetPhysicsSystem(PhysicsSystem* physics) {
				this->physics = physics;
			}

		protected:
			std::vector<GameObject*> gameObjects;
			std::vector<GameObject*> staticObjects;
			std::vector<Constraint*> constraints;

			PhysicsSystem* physics;

			Camera* mainCamera;

			bool	shuffleConstraints;
			bool	shuffleObjects;
			int		worldIDCounter;
		};
	}
}

