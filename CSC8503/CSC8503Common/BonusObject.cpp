#include "BonusObject.h"
#include "PlayerObject.h"
#include "EnemyObject.h"

bool NCL::CSC8503::BonusObject::UpdateObject(float dt) {
	return true;
}

void NCL::CSC8503::BonusObject::OnCollisionBegin(GameObject* otherObj) {
	if (otherObj->GetName() == "enemy") {
		EnemyObject* enemy = dynamic_cast<EnemyObject*> (otherObj);
		enemy->AddScore(score);
		isActive = false;
	}
	else if (otherObj->GetName() == "player") {
		PlayerObject* player = dynamic_cast<PlayerObject*> (otherObj);
		player->AddScore(score);
		isActive = false;
	}
}
