#include "EnemyObject.h"
#include "NavigationGrid.h"
#include "BehaviourNodeWithChildren.h"
#include "BehaviourAction.h"
#include "BehaviourSelector.h"
#include "BonusObject.h"
#include "Ray.h"
#include "CollisionDetection.h"

NCL::CSC8503::EnemyObject::EnemyObject(PlayerObject* player) : GameObject("enemy") {
	speed = 10.0f;
	score = 0;
	this->player = player;
	layer = Layer::Default | Layer::IgnoreRaycast | Layer::Player;
	CreateBehaviourTree();
}



void NCL::CSC8503::EnemyObject::CreateBehaviourTree() {

	BehaviourAction* findPlayer = new BehaviourAction("Go to Player",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				bool found = rand() % 2;
				if (found) {
					FindPath(player->GetTransform().GetPosition());
					state = Ongoing;
				}
				else {
					return Failure;
				}
			}
			else if (state == Ongoing) {
				FollowPath();
				if (path.empty()) {
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);

	// Raycast not fully implemented yet
	BehaviourAction* rayCast = new BehaviourAction("Raycast",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				targetDir = (bonuses[0] - GetTransform().GetPosition()).Normalised();
				Ray ray = Ray(GetTransform().GetPosition(), targetDir, Layer::Default | Layer::Player);
				RayCollision c;
				Raycast(ray, c, true);
				GameObject* obj = (GameObject*)c.node;
				if (obj->GetName() == "bonus") {
					state = Ongoing;
				}
				return Failure;
			}
			else if (state == Ongoing) {
				GetPhysicsObject()->AddForce(targetDir * speed);
				if (!getBonus) {
					return Success;
				}
			}
			return state;
		}
	);

	BehaviourAction* findBonus = new BehaviourAction("Go to Bonus",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				//	bool found = rand() % 2;
				if (!bonuses.empty()) {
					getBonus = true;
					FindPath(bonuses[0]);
					state = Ongoing;
				}
				else {
					return Failure;
				}
			}
			else if (state == Ongoing) {
				FollowPath();
				if (path.empty()) {
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);

	BehaviourSequence* sequence = new BehaviourSequence("Find Bonus");
	sequence->AddChild(findBonus);
	sequence->AddChild(rayCast);

	BehaviourSelector* selection =
		new BehaviourSelector("Path Selection");
	selection->AddChild(findPlayer);
	selection->AddChild(sequence);

	rootSequence =
		new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(selection);
}

bool NCL::CSC8503::EnemyObject::UpdateObject(float dt) {
	//rootSequence->Reset();
	BehaviourState state = Ongoing;

	state = rootSequence->Execute(dt);
	if (state == Success) {
		rootSequence->Reset();
	}
	else if (state == Failure) {
		std::cout << "Fail!\n";
	}
	DisplayPathfinding();
	return true;
}

bool NCL::CSC8503::EnemyObject::FindPath(Vector3 dest) {
	path.clear();

	NavigationPath outPath;

	Vector3 startPos = GetTransform().GetPosition();
	bool found = navGrid->FindPath(startPos, dest, outPath);
	if (found) {
		Vector3 pos;
		while (outPath.PopWaypoint(pos)) {
			path.push_back(pos);
		}
		return true;
	}

	return false;
}

void NCL::CSC8503::EnemyObject::FollowPath() {
	if (!path.empty()) {
		Vector3 pos = GetTransform().GetPosition();
		Vector3 pathDir = path[0] - pos;
		float distance = pathDir.LengthSquared(); // save a bit of performance by not using sqrt

		if (distance < threshold) {
			path.erase(path.begin());
		}
		else {
			pathDir = pathDir.Normalised();
			GetPhysicsObject()->AddForce(pathDir * speed);
		}
	}
}

void NCL::CSC8503::EnemyObject::DisplayPathfinding() {
	for (int i = 1; i < path.size(); ++i) {
		Vector3 a = path[i - 1];
		Vector3 b = path[i];

		Debug::DrawLine(b + Vector3(0, 10, 0), a + Vector3(0, 10, 0), Vector4(0, 1, 0, 1));
	}

}

void NCL::CSC8503::EnemyObject::OnCollisionEnd(GameObject* otherObj) {
	if (otherObj->GetName() == "player") {
		GetTransform().SetPosition(defaultSpawnPos);
		rootSequence->Reset();
	}
	if (otherObj->GetName() == "bonus" && otherObj->GetTransform().GetPosition() == bonuses[0]) { // Don't reset if we collect a bonus on the way to the player
		bonuses.erase(bonuses.begin());
		getBonus = false;
		rootSequence->Reset();
	}
}
