#pragma once
#include "Common/Math/Vector2.h"
#include "../CSC8503Common/CollisionDetection.h"
#include "Debug.h"
#include <list>
#include <functional>
#include "Ray.h"
#include <set>


namespace NCL {
	using namespace NCL::Maths;
	using std::vector;
	namespace CSC8503 {
		template<class T>
		class QuadTree;

		template<class T>
		struct QuadTreeEntry {
			Vector3 pos;
			Vector3 size;
			T object;

			QuadTreeEntry(T obj, Vector3 pos, Vector3 size) {
				object		= obj;
				this->pos	= pos;
				this->size	= size;
			}
		};

		template<class T>
		class QuadTreeNode	{
		public:
			typedef std::function<void(std::list<QuadTreeEntry<T>>&)> QuadTreeFunc;

			void OperateOnContents(QuadTreeFunc& func) {
				if (children) { // !isAsleep Need to debug
					for (int i = 0; i < 4; ++i) {
						children[i].OperateOnContents(func);
					}
				}
				else {
					if (!contents.empty() && !isAsleep) {
						func(contents);
					}

				}
			}

			int ContentsSize() {
				return contents.size();
			}

			std::list< QuadTreeEntry<T> > GetContents() {
				return contents;
			}
		protected:
			friend class QuadTree<T>;

			QuadTreeNode() {}

			QuadTreeNode(Vector2 pos, Vector2 size) {
				children		= nullptr;
				this->position	= pos;
				this->size		= size;
				sleepCount = 0;
				isAsleep = true;
			}

			~QuadTreeNode() {
				delete[] children;
			}

			void Insert(T& object, const Vector3& objectPos, const Vector3& objectSize, int depthLeft, int maxSize, bool objectState = false) {
				if (!CollisionDetection::AABBTest(objectPos,
					Vector3(position.x, 0, position.y), objectSize,
					Vector3(size.x, 1000.0f, size.y))) {
					return;
				}
				if (children) { //not a leaf node , just descend the tree
					for (int i = 0; i < 4; ++i) {
						children[i].Insert(object, objectPos, objectSize,
							depthLeft - 1, maxSize);
					}
				}
				else { // currently a leaf node , can just expand
					contents.push_back(QuadTreeEntry <T>(object, objectPos, objectSize));
					//isAsleep = sleepCount >= maxSize ? true : false;
					isAsleep = isAsleep && objectState;
					if ((int)contents.size() > maxSize && depthLeft > 0) {
						isAsleep = isAsleep && objectState; // Wakeup the node if the final insert is not asleep
						if (!children) {
							Split();
							//we need to reinsert the contents so far!
							for (const auto& i : contents) {
								for (int j = 0; j < 4; ++j) {
									auto entry = i;
									children[j].Insert(entry.object, entry.pos,
										entry.size, depthLeft - 1, maxSize);
								}
							}
							contents.clear(); // contents now distributed!
						}
					}
				}
			}

			void GetNodeToInsert(T object, QuadTreeNode<T>*& node) {
				if (node != nullptr) {
					return;
				}
				if (children) {
					for (int i = 0; i < 4; ++i) {
						children[i].GetNodeToInsert(object, node);
					}
				}
				else if (node == nullptr) {
					node = this;
				}
			}

			void BuildRayCollisionList(Ray& r, vector<T>& collisions) {
				RayCollision c;
				if (CollisionDetection::RayBoxIntersection(r, Vector3(position.x, 0, position.y), Vector3(size.x, 1000, size.y), c, true)) {
					if (children) {
						for (int i = 0; i < 4; ++i) {
							children[i].BuildRayCollisionList(r, collisions);
						}
					}
					else {
						for (const auto& i : contents) {
							collisions.push_back(i.object);
						}
					}
				}
			}

			void Split() {
				Vector2 halfSize = size / 2.0f;
				children = new QuadTreeNode <T >[4];
				children[0] = QuadTreeNode <T>(position +
					Vector2(-halfSize.x, halfSize.y), halfSize);
				children[1] = QuadTreeNode <T>(position +
					Vector2(halfSize.x, halfSize.y), halfSize);
				children[2] = QuadTreeNode <T>(position +
					Vector2(-halfSize.x, -halfSize.y), halfSize);
				children[3] = QuadTreeNode <T>(position +
					Vector2(halfSize.x, -halfSize.y), halfSize);
				for (int i = 0; i < 4; ++i) {
					children[i].isAsleep = this->isAsleep;
				}
			}

			void Clear() {
				delete[] children;
				children = nullptr;
				contents.clear();
			}

			void DebugDraw() {
				Vector2 halfSizes = size / 2.0f;
				Vector4 colour = Debug::GREEN;
				Debug::DrawLine(Vector3(position.x + halfSizes.x, 0, position.y + halfSizes.y),
					Vector3(position.x + halfSizes.x, 0, position.y - halfSizes.y),
					colour);
				Debug::DrawLine(Vector3(position.x + halfSizes.x, 0, position.y - halfSizes.y),
					Vector3(position.x - halfSizes.x, 0, position.y - halfSizes.y),
					colour);
				Debug::DrawLine(Vector3(position.x - halfSizes.x, 0, position.y - halfSizes.y),
					Vector3(position.x - halfSizes.x, 0, position.y + halfSizes.y),
					colour);
				Debug::DrawLine(Vector3(position.x - halfSizes.x, 0, position.y + halfSizes.y),
					Vector3(position.x + halfSizes.x, 0, position.y + halfSizes.y),
					colour);
				if (children) {
					for (int i = 0; i < 4; ++i) {
						children[i].DebugDraw();
					}

				}
			}

			

		protected:
			std::list< QuadTreeEntry<T> >	contents;

			Vector2 position;
			Vector2 size;
			int sleepCount;
			bool isAsleep;
			QuadTreeNode<T>* children;
		};
	}
}


namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree
		{
		public:
			QuadTree() {

			}

			QuadTree(Vector2 size, int maxDepth = 6, int maxSize = 5){
				root = QuadTreeNode<T>(Vector2(), size);
				this->maxDepth	= maxDepth;
				this->maxSize	= maxSize;
			}
			~QuadTree() {
			}

			void Insert(T object, const Vector3& pos, const Vector3& size, bool objectState = false) {
				root.Insert(object, pos, size, maxDepth, maxSize, objectState);
			}

			void DebugDraw() {
				root.DebugDraw();
			}
			
			QuadTreeNode<T>* GetNodeToInsert(T object) {
				QuadTreeNode<T>* node = nullptr;
				root.GetNodeToInsert(object, node);
				return node;
			}

			void Clear() {
				root.Clear();
			}

			vector<T> BuildRayCollisonList(Ray& r) {
				vector<T> collisions;
				root.BuildRayCollisionList(r, collisions);
				return collisions;
			}

			void OperateOnContents(typename QuadTreeNode<T>::QuadTreeFunc  func) {
				root.OperateOnContents(func);
			}

		protected:
			QuadTreeNode<T> root;
			//QuadTreeNode<T>* insertionNode;
			int maxDepth;
			int maxSize;
		};
	}
}