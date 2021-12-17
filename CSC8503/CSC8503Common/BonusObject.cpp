#include "BonusObject.h"
#include "PlayerObject.h"

void NCL::CSC8503::BonusObject::OnCollisionBegin(GameObject* otherObj) {
	isActive = false;
	layer = Layer::Player;
}
