#include "GameObject.h"
#include "CollisionDetection.h" 
#include <sstream>

using namespace NCL::CSC8503;
using std::string;

//std to_string can be problematic with floats so I've defined my own function
template <typename T> string tostr(const T& t) {
	std::ostringstream os;
	os << t;
	return os.str();
}

GameObject::GameObject(string objectName)	{
	name			= objectName;
	worldID			= -1;
	isActive		= true;
	isAsleep = false;
	isTrigger = false;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	layer = Layer::Default;
}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
}

/*void GameObject::InitObjects(TutorialGame* g) {
	game = g;
}*/

void GameObject::PrintDebugInfo() {
	string selected = "Selected: " + tostr(worldID) + " " + name;
	Debug::Print(selected, Vector2(60, 5), Debug::BLACK);
	const Vector3 pos = transform.GetPosition();
    Quaternion orient = transform.GetOrientation();
	string position = "Position: " + tostr(pos.x) + ", " + tostr(pos.y) + ", " + tostr(pos.z);
	Debug::Print(position, Vector2(60, 10), Debug::BLACK);
	string orientation = "Orientation: " + tostr(orient.x) + ", " + tostr(orient.y) + ", " + tostr(orient.z);
	Debug::Print(orientation, Vector2(60, 15), Debug::BLACK);
	//string volume = "Collision Volume: " + tostr(boundingVolume->type);
	//Debug::Print(volume, Vector2(60, 20), Debug::BLACK);


}
