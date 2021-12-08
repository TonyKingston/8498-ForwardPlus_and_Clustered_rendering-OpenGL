#include "PositionOrientationConstraint.h"
#include "../../Common/Vector3.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "../../Common/Maths.h"

void NCL::CSC8503::PositionOrientationConstraint::UpdateConstraint(float dt)
{
	Vector3 relativePos =
		objectA->GetTransform().GetPosition() -
		objectB->GetTransform().GetPosition();

	const Vector3 direction = relativePos.Normalised();

	objectA->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, Maths::RadiansToDegrees(atan2f(-direction.z, direction.x)), 0));
	objectB->GetTransform().SetOrientation(objectA->GetTransform().GetOrientation());

	PositionConstraint::UpdateConstraint(dt);
}
