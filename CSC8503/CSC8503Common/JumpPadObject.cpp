#include "JumpPadObject.h"

void NCL::CSC8503::JumpPadObject::OnClick() {
	if ((GetTransform().GetPosition() - player->GetTransform().GetPosition()).Length() <= range) {
		player->GetPhysicsObject()->AddForce((direction)* force);
	}
}

bool NCL::CSC8503::JumpPadObject::UpdateObject(float dt) {
	return true;
}
