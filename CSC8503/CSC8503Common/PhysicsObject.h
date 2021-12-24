#pragma once
#include "../../Common/Vector3.h"
#include "../../Common/Matrix3.h"

using namespace NCL::Maths;

namespace NCL {
	class CollisionVolume;
	
	namespace CSC8503 {
		class Transform;

		class PhysicsObject	{
		public:
			PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume);
			~PhysicsObject();

			Vector3 GetLinearVelocity() const {
				return linearVelocity;
			}

			Vector3 GetAngularVelocity() const {
				return angularVelocity;
			}

			Vector3 GetTorque() const {
				return torque;
			}

			Vector3 GetForce() const {
				return force;
			}

			void SetInverseMass(float invMass) {
				inverseMass = invMass;
				if (invMass == 0.0f) {
					isStatic = true;
				}
			}

			float GetInverseMass() const {
				return inverseMass;
			}

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);
			
			void AddForce(const Vector3& force);

			void AddForceAtPosition(const Vector3& force, const Vector3& position);
			void AddForceAtLocalPosition(const Vector3& force, const Vector3& localPos);

			void AddTorque(const Vector3& torque);


			void ClearForces();

			void SetLinearVelocity(const Vector3& v) {
				linearVelocity = v;
			}

			void SetAngularVelocity(const Vector3& v) {
				angularVelocity = v;
			}

			float GetElasticity() {
				return elasticity;
			}

			void SetElasticity(float elast) {
				elasticity = elast;
			}

			float GetFriction() {
				return friction;
			}

			void SetFriction(float f) {
				friction = f;
			}

			float GetStiffness() {
				return stiffness;
			}

			void SetStiffness(float s) {
				stiffness = s;
			}

			float GetLinearDamping() {
				return linearDamping;
			}

			void SetLinearDamping(float d) {
				linearDamping = d;
			}
			
			float GetAngularDamping() {
				return angularDamping;
			}

			void SetAngularDamping(float d) {
				angularDamping = d;
			}

			void UpdateWeightedAverageMotion() {
				float bias = 0.94f;
				float motion = linearVelocity.LengthSquared() + angularVelocity.LengthSquared();
				rwaMotion = bias * rwaMotion + (1 - bias) * motion;
			}

			void SetWeightedAverageMotion(float m) {
				rwaMotion = m;
			}

			float GetWeightedAverageMotion() {
				return rwaMotion;
			}

			bool IsStatic() {
				return isStatic;
			}

			void SetIsStatic(bool val) {
				isStatic = val;
			}

			

			void InitCubeInertia();
			void InitSphereInertia(bool hollow = false);

			void UpdateInertiaTensor();

			Matrix3 GetInertiaTensor() const {
				return inverseInteriaTensor;
			}

		protected:
			const CollisionVolume* volume;
			Transform*		transform;

			float inverseMass;
			float elasticity;
			float friction;
			float stiffness;
			float linearDamping;
			float angularDamping;

			bool isStatic;

			//linear stuff
			Vector3 linearVelocity;
			Vector3 force;
			float rwaMotion;

			//angular stuff
			Vector3 angularVelocity;
			Vector3 torque;
			Vector3 inverseInertia;
			Matrix3 inverseInteriaTensor;
		};
	}
}

