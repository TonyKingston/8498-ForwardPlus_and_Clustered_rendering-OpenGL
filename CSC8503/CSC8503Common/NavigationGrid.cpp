#include "NavigationGrid.h"
#include "../../Common/Assets.h"

#include <fstream>
#include <list>
#include <queue>
#include "Debug.h"
#include "../../Common/Quaternion.h"
#include "Transform.h"

using namespace NCL;
using namespace CSC8503;

const int LEFT_NODE		= 0;
const int RIGHT_NODE	= 1;
const int TOP_NODE		= 2;
const int BOTTOM_NODE	= 3;

const char WALL_NODE	= 'x';
const char FLOOR_NODE	= '.';

NavigationGrid::NavigationGrid()	{
	nodeSize	= 0;
	gridWidth	= 0;
	gridHeight	= 0;
	allNodes	= nullptr;
}

NavigationGrid::NavigationGrid(const std::string&filename) : NavigationGrid() {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	allNodes = new GridNode[gridWidth * gridHeight];

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode&n = allNodes[(gridWidth * y) + x];
			char type = 0;
			infile >> type;
			n.type = type;
			n.position = Vector3(x * nodeSize, 0, y * nodeSize);
		}
	}
	
	//now to build the connectivity between the nodes
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode&n = allNodes[(gridWidth * y) + x];		

			if (y > 0) { //get the above node
				n.connected.push_back(&allNodes[(gridWidth * (y - 1)) + x]);
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected.push_back(&allNodes[(gridWidth * (y + 1)) + x]);
			}
			if (x > 0) { //get left node
				n.connected.push_back(&allNodes[(gridWidth * (y)) + (x - 1)]);
			}
			if (x < gridWidth - 1) { //get right node
				n.connected.push_back(&allNodes[(gridWidth * (y)) + (x + 1)]);
			}
			if (n.type == '.') {
				n.cost = 1;
			}
			else if (n.type == 'x' || n.type == 'W' || n.type == 'w') {
				n.obstacle = true;
				n.cost = 100;
			}
			if (n.obstacle) {
				Vector4 colour = Debug::RED;
				float height = 35.0f;
				float time = 60.0f;

				// Draw obstacles for debugging
				/*Debug::DrawLine(Vector3(n.position.x + nodeSize/2, height, n.position.z - nodeSize/2),
					Vector3(n.position.x + nodeSize / 2, height, n.position.z + nodeSize / 2),
					colour, time);

				Debug::DrawLine(Vector3(n.position.x + nodeSize / 2, height, n.position.z + nodeSize / 2),
					Vector3(n.position.x - nodeSize / 2, height, n.position.z + nodeSize / 2),
					colour, time);

				Debug::DrawLine(Vector3(n.position.x - nodeSize / 2, height, n.position.z + nodeSize / 2),
					Vector3(n.position.x - nodeSize / 2, height, n.position.z - nodeSize / 2),
					colour, time);
				Debug::DrawLine(Vector3(n.position.x - nodeSize / 2, height, n.position.z - nodeSize / 2),
					Vector3(n.position.x + nodeSize / 2, height, n.position.z - nodeSize / 2),
					colour, time);*/



			}
		}	
	}
}

NavigationGrid::~NavigationGrid()	{
	delete[] allNodes;
}

bool NavigationGrid::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	//need to work out which node 'from' sits in, and 'to' sits in
	int fromX = ((int)from.x / nodeSize);
	int fromZ = ((int)from.z / nodeSize);

	int toX = ((int)to.x / nodeSize);
	int toZ = ((int)to.z / nodeSize);

	if (fromX < 0 || fromX > gridWidth - 1 ||
		fromZ < 0 || fromZ > gridHeight - 1) {
		return false; //outside of map region!
	}

	if (toX < 0 || toX > gridWidth - 1 ||
		toZ < 0 || toZ > gridHeight - 1) {
		return false; //outside of map region!
	}

	GridNode* startNode = &allNodes[(fromZ * gridWidth) + fromX];
	GridNode* endNode	= &allNodes[(toZ * gridWidth) + toX];

	//std::priority_queue<GridNode*>  openList;
	std::list<GridNode*> openList;
	GridNode* currentNode = startNode;
	currentNode->localGoal = 0.0f;
	currentNode->globalGoal = Heuristic(startNode, endNode);
	currentNode->bestParent = nullptr;
	openList.push_back(startNode);

	while (!openList.empty()) {

		openList.sort([](const GridNode* lhs, const GridNode* rhs) { // Try best paths first
			return lhs->globalGoal < rhs->globalGoal; });

		while (!openList.empty() && openList.front()->visited) {
			openList.pop_front();
		}

		if (openList.empty())
			break;

		currentNode = openList.front();
		currentNode->visited = true;

		for (int i = 0; i < currentNode->connected.size(); i++) {
			GridNode* neighbour = currentNode->connected[i];
			if (!neighbour->visited && !neighbour->obstacle) {
				openList.push_back(neighbour);
			}

			float newGoal = currentNode->localGoal + neighbour->cost;

			if (newGoal < neighbour->localGoal) {
				neighbour->bestParent = currentNode;
				neighbour->localGoal = newGoal;

				neighbour->globalGoal = neighbour->localGoal + Heuristic(neighbour, endNode);
			}

		}
	}

	GridNode* node = endNode;
	while (node->bestParent != NULL) {
		outPath.PushWaypoint(node->position);
		node = node->bestParent;
	}
	if (node->bestParent == NULL) { // Set staring node's parent to itself.
		node->bestParent == node;
		return true;
	}
	return false; //open list emptied out with no path!
}

bool NavigationGrid::NodeInList(GridNode* n, std::vector<GridNode*>& list) const {
	std::vector<GridNode*>::iterator i = std::find(list.begin(), list.end(), n);
	return i == list.end() ? false : true;
}

/*GridNode*  NavigationGrid::RemoveBestNode(std::vector<GridNode*>& list) const {
	std::vector<GridNode*>::iterator bestI = list.begin();

	GridNode* bestNode = *list.begin();

	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i)->f < bestNode->f) {
			bestNode	= (*i);
			bestI		= i;
		}
	}
	list.erase(bestI);

	return bestNode;
}*/

float NavigationGrid::Heuristic(GridNode* hNode, GridNode* endNode) const {
	//return (hNode->position - endNode->position).Length();
	Vector3 a = hNode->position;
	Vector3 b = endNode->position;
	return std::abs(a.x - b.x) + std::abs(a.z - b.z);
}