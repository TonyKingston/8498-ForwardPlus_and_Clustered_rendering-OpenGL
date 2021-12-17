#include "PlayerObject.h"

NCL::CSC8503::PlayerObject::PlayerObject() : GameObject("player") {
	magicCount = 0;
	layer = Layer::Default || Layer::Player;

}

void NCL::CSC8503::PlayerObject::OnCollisionBegin(GameObject* otherObject) {
}

void NCL::CSC8503::PlayerObject::OnCollisionEnd(GameObject* otherObject) {
}
