#include "EnemyObject.h"
#include "NavigationGrid.h"
#include "BehaviourNodeWithChildren.h"
#include "BehaviourAction.h"
#include "BehaviourSelector.h"

NCL::CSC8503::EnemyObject::EnemyObject(PlayerObject* player) : GameObject("enemy") {
	speed = 10.0f;
	this->player = player;
	layer = Layer::Default | Layer::Player;
	CreateBehaviourTree();
}



void NCL::CSC8503::EnemyObject::CreateBehaviourTree() {
	float behaviourTimer;
	float distanceToTarget;
	BehaviourAction* findPlayer = new BehaviourAction("Go to Player",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				FindPath(player->GetTransform().GetPosition());
				state = Ongoing;
			}
			else if (state == Ongoing) {
				FollowPath();
				if (path.size() < 27) {
					bool a = true;
				}
				if (path.empty()) {
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);

	/*BehaviourAction* findPlayer = new BehaviourAction("Go to Bonus",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				FindPath(player->GetTransform().GetPosition());
				state = Ongoing;
			}
			else if (state == Ongoing) {
				FollowPath();
				if (path.size() < 27) {
					bool a = true;
				}
				if (path.empty()) {
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);*/

	BehaviourSequence* sequence =
		new BehaviourSequence("Player Sequence");
	sequence->AddChild(findPlayer);

	rootSequence =
		new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
}

bool NCL::CSC8503::EnemyObject::UpdateObject(float dt) {
	//rootSequence->Reset();
	BehaviourState state = Ongoing;
	state = rootSequence->Execute(dt);
	if (state == Success) {
		std::cout << "Success \n";
	}
	else if (state == Failure) {
		std::cout << " What a waste of time !\n";
	}
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
		float distance = pathDir.Length();

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

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1), 20.0f);

	}

}

void NCL::CSC8503::EnemyObject::OnCollisionBegin(GameObject* otherObj) {
}
