#pragma once
#include "NavigationMap.h"
#include <string>
#include <vector>
namespace NCL {
	namespace CSC8503 {
		struct GridNode {
			GridNode* bestParent;

		//	GridNode* connected[4];
			std::vector<GridNode*> connected;
			//int		  costs[4];
			int cost;

			Vector3		position;
			float    globalGoal;
			float    localGoal;
			bool     visited;
			bool     obstacle;
			int type;

			GridNode() {
				/*for (int i = 0; i < 4; ++i) {
					//connected[i] = nullptr;
					costs[i] = 0;
				}*/
				cost = 0;
				localGoal = INFINITY;
				globalGoal = INFINITY;
				type = 0;
				bestParent = nullptr;
				obstacle = false;
				visited = false;
			}
			~GridNode() {	}
		};

		class NavigationGrid : public NavigationMap	{
		public:
			NavigationGrid();
			NavigationGrid(const std::string&filename);
			~NavigationGrid();

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
				
		protected:
			bool		NodeInList(GridNode* n, std::vector<GridNode*>& list) const;
			GridNode*	RemoveBestNode(std::vector<GridNode*>& list) const;
			void ResetGrid();
			inline float		Heuristic(GridNode* hNode, GridNode* endNode) const;
			int nodeSize;
			int gridWidth;
			int gridHeight;

			GridNode* allNodes;
		};
	}
}

