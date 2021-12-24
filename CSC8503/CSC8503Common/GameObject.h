#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"

#include <vector>
#include <functional>
#include "Ray.h"
#include "Layer.h"

using std::vector;

namespace NCL {
	namespace CSC8503 {
		class TutorialGame;
		class GameObject	{
		public:

			GameObject(string name = "");
			~GameObject();

			static void InitObjects(TutorialGame* game);
			virtual bool UpdateObject(float dt) { return true; }

			void SetBoundingVolume(CollisionVolume* vol) {
				boundingVolume = vol;
				vol->SetObject(this);
			}

			const CollisionVolume* GetBoundingVolume() const {
				return boundingVolume;
			}

			bool IsActive() const {
				return isActive;
			}

			bool IsSpring() const {
				return isSpring;
			}

			void SetAsSpring() {
				isSpring = true;
			}

			void Deactivate() {
				isActive = false;
			}

			bool IsAsleep() const {
				return isAsleep;
			}

			bool IsTrigger() const {
				return isTrigger;
			}

			void SetTrigger() { isTrigger = true; }

			void PutToSleep() {
				isAsleep = true;
			}

			void Wake() {
				isAsleep = false;
			}

			Transform& GetTransform() {
				return transform;
			}

			RenderObject* GetRenderObject() const {
				return renderObject;
			}

			PhysicsObject* GetPhysicsObject() const {
				return physicsObject;
			}

			Vector3 GetForwordDirection() {
				return transform.GetOrientation() * worldForward;
			}

			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) {
				physicsObject = newObject;
			}

			const string& GetName() const {
				return name;
			}

			virtual void OnCollisionBegin(GameObject* otherObject) {
				//std::cout << "OnCollisionBegin event occured!\n";
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

			typedef std::function<void(GameObject*)> TriggerFunc;
			void OnTrigger(GameObject* otherObject) {
				triggerFunc(otherObject);
			}

			typedef std::function<void(Ray, RayCollision&, bool)> RayFunc;
			void Raycast(Ray ray, RayCollision& c, bool closest = true) {
				rayFunc(ray, c, closest);
			}

			void SetRayFunc(RayFunc function) {
				rayFunc = function;
			}

			void SetTriggerFunc(TriggerFunc function) {
				triggerFunc = function;
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

			void DrawBoundingVolume() {
				boundingVolume->DrawVolume(transform.GetPosition());
			}

			void SetWorldID(int newID) {
				worldID = newID;
			}

			int		GetWorldID() const {
				return worldID;
			}

			void SetLayerMask(int layerMask) {
				layer = layerMask;
			}

			int GetLayerMask() {
				return layer;
			}
			void PrintDebugInfo();

			virtual void OnClick() {};

		protected:
			Transform			transform;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject* renderObject;

			bool	isActive;
			bool    isAsleep;
			bool    isTrigger;
			bool    isSpring;
			int		worldID;
			int layer;
			string	name;
			TriggerFunc triggerFunc;
			RayFunc rayFunc;
			Vector3 broadphaseAABB;
			static TutorialGame* game;
		};
	}
}

