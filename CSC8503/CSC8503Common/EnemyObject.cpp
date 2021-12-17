#include "EnemyObject.h"
#include "NavigationGrid.h"

NCL::CSC8503::EnemyObject::EnemyObject() : GameObject("enemy") {
	node = nullptr;
}



bool NCL::CSC8503::EnemyObject::UpdateObject(float dt) {
	return true;
}

bool NCL::CSC8503::EnemyObject::FindPath(Vector3 dest) {
	path.clear();
	NavigationGrid grid("MazeGrid.txt");

	NavigationPath outPath;

	Vector3 startPos = GetTransform().GetPosition();
	bool found = grid.FindPath(startPos, dest, outPath);
	if (found) {
		Vector3 pos;
		while (outPath.PopWaypoint(pos)) {
			path.push_back(pos);
		}
		return true;
	}
	
	return false;
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
