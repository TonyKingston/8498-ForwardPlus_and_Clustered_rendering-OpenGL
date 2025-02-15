#include "PositionOrientationConstraint.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "Common/Math/Maths.h"

void NCL::CSC8503::PositionOrientationConstraint::UpdateConstraint(float dt)
{

	PhysicsObject* physA = objectA->GetPhysicsObject();
	PhysicsObject* physB = objectB->GetPhysicsObject();

	const Vector3 direction = (objectA->GetTransform().GetPosition() - objectB->GetTransform().GetPosition()).Normalised();

	Vector3 currentOrientation = (objectA->GetTransform().GetOrientation() * axis).Normalised();

	Vector3 rotationAxis = Vector3::Cross(direction, currentOrientation);


	if (rotationAxis.LengthSquared() > 0.0f) {
		float constraintMass = physA->GetInverseMass();

		if (constraintMass > 0.0f) {

			float velocityDot = Vector3::Dot(physA->GetAngularVelocity(), rotationAxis.Normalised());
			float biasFactor = 0.01f;
			float bias = (biasFactor / dt) * rotationAxis.Length();
			float lambda = -(velocityDot + bias) / constraintMass;
			Vector3 aImpulse = rotationAxis * lambda;

			physA->ApplyAngularImpulse(aImpulse);
		}
	}

	PositionConstraint::UpdateConstraint(dt);
	

	
	// Hinge
	/*Vector3 relativePos =
		objectA->GetTransform().GetPosition() -
		objectB->GetTransform().GetPosition();

	const Vector3 direction = relativePos.Normalised();

	objectA->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, Maths::RadiansToDegrees(atan2f(-direction.z, direction.x)), 0));
	objectB->GetTransform().SetOrientation(objectA->GetTransform().GetOrientation());

	PositionConstraint::UpdateConstraint(dt);*/
}
