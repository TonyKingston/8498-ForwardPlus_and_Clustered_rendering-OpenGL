#include "GameWorld.h"
#include "GameObject.h"
#include "Constraint.h"
#include "CollisionDetection.h"
#include "Common/Graphics/Camera.h"
#include <algorithm>
#include "PhysicsSystem.h"

using namespace NCL;
using namespace NCL::CSC8503;

GameWorld::GameWorld() {
	mainCamera = new Camera();
	shuffleConstraints = false;
	shuffleObjects = false;
	worldIDCounter = 0;
}


GameWorld::GameWorld(PhysicsSystem* physics)	{
	mainCamera = new Camera();
	this->physics = physics;
	shuffleConstraints	= false;
	shuffleObjects		= false;
	worldIDCounter		= 0;
}

/*GameWorld::GameWorld(TutorialGame* game)	{
	mainCamera = new Camera();
	this->game = game;
	shuffleConstraints	= false;
	shuffleObjects		= false;
	worldIDCounter		= 0;
}*/

GameWorld::~GameWorld()	{
}

void GameWorld::Clear() {
	gameObjects.clear();
	constraints.clear();
}

void GameWorld::ClearAndErase() {
	for (auto& i : gameObjects) {
		delete i;
	}
	for (auto& i : constraints) {
		delete i;
	}
	Clear();
}

void GameWorld::AddGameObject(GameObject* o) {
	if (o->GetPhysicsObject() != nullptr && o->GetPhysicsObject()->IsStatic()) {
		gameObjects.emplace_back(o);
		staticObjects.emplace_back(o);
	}
	else {
		gameObjects.emplace_back(o);
	}
	o->SetWorldID(worldIDCounter++);
}

void GameWorld::RemoveGameObject(GameObject* o, bool andDelete) {
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
	if (andDelete) {
		delete o;
	}
}

void GameWorld::GetObjectIterators(
	GameObjectIterator& first,
	GameObjectIterator& last, bool isStatic) const {

	if (isStatic) {
		first = staticObjects.begin();
		last = staticObjects.end();
	}
	else {
		first = gameObjects.begin();
		last = gameObjects.end();
	}
}

void GameWorld::OperateOnContents(GameObjectFunc f) {
	for (GameObject* g : gameObjects) {
		f(g);
	}
}

void GameWorld::UpdateWorld(float dt) {
	OperateOnContents([&](GameObject* obj)->void
		{
			RenderObject* rend = obj->GetRenderObject();
			if (rend) {
				obj->GetRenderObject()->Update(dt);
			}
			if (obj->IsToDelete()) {
				RemoveGameObject(obj, true);
			}
		}
	);
}

bool GameWorld::Raycast(Ray& r, RayCollision& closestCollision, bool closestObject) const {
	//The simplest raycast just goes through each object and sees if there's a collision
	RayCollision collision;
	
	vector<GameObject*> objects = physics->GetQuadTree()->BuildRayCollisonList(r);
	for (auto& i : objects) {
		if (!i->GetBoundingVolume()) {
			continue;
		}
		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {

			if (!closestObject) {
				closestCollision = collision;
				closestCollision.node = i;
				return true;
			}
			else {
				if (thisCollision.rayDistance < collision.rayDistance) {
					thisCollision.node = i;
					collision = thisCollision;
				}
			}
		}
	}
	/*for (auto& i : gameObjects) {
		if (!i->GetBoundingVolume()) { //objects might not be collideable etc...
			continue;
		}
		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
				
			if (!closestObject) {	
				closestCollision		= collision;
				closestCollision.node = i;
				return true;
			}
			else {
				if (thisCollision.rayDistance < collision.rayDistance) {
					thisCollision.node = i;
					collision = thisCollision;
				}
			}
		}
	}*/
	if (collision.node) {
		closestCollision		= collision;
		closestCollision.node	= collision.node;
		return true;
	}
	return false;
}


/*
Constraint Tutorial Stuff
*/

void GameWorld::AddConstraint(Constraint* c) {
	constraints.emplace_back(c);
}

void GameWorld::RemoveConstraint(Constraint* c, bool andDelete) {
	constraints.erase(std::remove(constraints.begin(), constraints.end(), c), constraints.end());
	if (andDelete) {
		delete c;
	}
}

void GameWorld::GetConstraintIterators(
	std::vector<Constraint*>::const_iterator& first,
	std::vector<Constraint*>::const_iterator& last) const {
	first	= constraints.begin();
	last	= constraints.end();
}

void GameWorld::DrawBoundingVolumes()
{
	for (auto& o : gameObjects) {
		o->DrawBoundingVolume();
		//OGLShader* shader = (OGLShader*)o->GetRenderObject()->GetShader();
		//BindShader(shader);
		//BindTextureToShader((OGLTexture*)(*i).GetDefaultTexture(), "mainTex", 0);
	}
}
