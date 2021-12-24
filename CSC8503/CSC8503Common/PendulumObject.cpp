#include "PendulumObject.h"

bool NCL::CSC8503::PendulumObject::UpdateObject(float dt) {
	//stateMachine->Update(dt);
	return true;
}

void NCL::CSC8503::PendulumObject::SwingRight(float dt) {
	this->GetPhysicsObject()->AddForce(Vector3(0, 0, 20));
	timer += dt;
}

void NCL::CSC8503::PendulumObject::SwingLeft(float dt) {
	this->GetPhysicsObject()->AddForce(Vector3(0, 0, -20));
	timer -= dt;
}
