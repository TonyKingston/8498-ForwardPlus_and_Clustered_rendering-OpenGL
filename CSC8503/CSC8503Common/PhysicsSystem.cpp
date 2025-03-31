#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "Common/Math/Quaternion.h"
#include "Common/Math/Maths.h"
#include "Core/GameTimer.h"
#include "Constraint.h"

#include "Debug.h"

#include <functional>
using namespace NCL;
using namespace CSC8503;

/*

These two variables help define the relationship between positions
and the forces that are added to objects to change those positions

*/

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g)	{
	applyGravity	= false;
	useBroadPhase	= true;	
	useSleep = false;
	usingPenalty = true;
	if (useBroadPhase) {
		tree = new QuadTree<GameObject*>(Vector2(1024, 1024), 7, 6);

	}
	dTOffset		= 0.0f;
	globalDamping	= 0.995f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
	linearDamping = 0.4f;
	sleepEpsilon = 0.05f;
	g.SetPhysicsSystem(this);
}

PhysicsSystem::~PhysicsSystem()	{
}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
	allCollisions.clear();
}

/*

This is the core of the physics engine update

*/
int constraintIterationCount = 10;

//This is the fixed timestep we'd LIKE to have
const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;

/*
This is the fixed update we actually have...
If physics takes too long it starts to kill the framerate, it'll drop the 
iteration count down until the FPS stabilises, even if that ends up
being at a low rate. 
*/
int realHZ		= idealHZ;
float realDT	= idealDT;

void PhysicsSystem::Update(float dt) {	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B)) {
		useBroadPhase = !useBroadPhase;
		std::cout << "Setting broadphase to " << useBroadPhase << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::I)) {
		constraintIterationCount--;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::O)) {
		constraintIterationCount++;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
		useSleep = !useSleep;
		std::cout << "Setting sleeping to " << useSleep << std::endl;
	}

	dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	GameTimer t;
	t.GetTimeDeltaSeconds();

	if (useBroadPhase) {
		UpdateObjectAABBs();
	}

	while(dTOffset >= realDT) {
		IntegrateAccel(realDT); //Update accelerations from external forces
		if (useBroadPhase) {
			BroadPhase();
			NarrowPhase();
		}
		else {
			BasicCollisionDetection();
		}
		if (usingPenalty) {
			IntegrateAccel(realDT, true);
		}
		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = realDT /  (float)constraintIterationCount;
		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);	
		}
		
		IntegrateVelocity(realDT); //update positions from new velocity changes

		dTOffset -= realDT;
	}
	if (useSleep) {
		UpdateSleepingObjects();
	}

	ClearForces();	//Once we've finished with the forces, reset them to zero

	UpdateCollisionList(); //Remove any old collisions

	t.Tick();
	float updateTime = t.GetTimeDeltaSeconds();

	//Uh oh, physics is taking too long...
	if (updateTime > realDT) {
		realHZ /= 2;
		realDT *= 2;
		std::cout << "Dropping iteration count due to long physics time...(now " << realHZ << ")\n";
	}
	else if(dt*2 < realDT) { //we have plenty of room to increase iteration count!
		int temp = realHZ;
		realHZ *= 2;
		realDT /= 2;

		if (realHZ > idealHZ) {
			realHZ = idealHZ;
			realDT = idealDT;
		}
		if (temp != realHZ) {
			std::cout << "Raising iteration count due to short physics time...(now " << realHZ << ")\n";
		}
	}
}

/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a 
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
			if (i->a->IsTrigger()) {
				i->a->OnTrigger(i->b);
			}
			else if (i->b->IsTrigger()) {
				i->b->OnTrigger(i->a);
			}
			else {
				i->a->OnCollisionBegin(i->b);
				i->b->OnCollisionBegin(i->a);
			}
		}
		(*i).framesLeft = (*i).framesLeft - 1;
		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			if (!i->a->IsActive()) {
				gameWorld.RemoveGameObject(i->a, true);
			}
			if (!i->b->IsActive()) {
				gameWorld.RemoveGameObject(i->b, true);
			}
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {
	gameWorld.OperateOnContents(
		[](GameObject* g) {
			g->UpdateBroadphaseAABB();
		}
	);
}

void NCL::CSC8503::PhysicsSystem::UpdateSleepingObjects() {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object) {
			object->UpdateWeightedAverageMotion();

			if (object->GetWeightedAverageMotion() < sleepEpsilon) {
				(*i)->PutToSleep();
				object->SetLinearVelocity(Vector3(0, 0, 0));
				object->SetAngularVelocity(Vector3(0, 0, 0));
			}
			else if (object->GetWeightedAverageMotion() > 10 * sleepEpsilon) { // multiply so the object doesn't oscillate between awake/asleep
				(*i)->Wake();
				object->SetWeightedAverageMotion(10 * sleepEpsilon);
			}
		}
	}
	
}

void NCL::CSC8503::PhysicsSystem::BuildStaticList() {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last, true);

	for (auto i = first; i != last; ++i) {

	}

}

/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset 
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
void PhysicsSystem::BasicCollisionDetection() {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		if ((*i)->GetPhysicsObject() == nullptr) {
			continue;
		}
		for (auto j = i + 1; j != last; ++j) {
			if ((*j)->GetPhysicsObject() == nullptr) {
				continue;
			}
			CollisionDetection::CollisionInfo info;
			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {
				std::cout << " Collision between " << (*i)->GetName()
					<< " and " << (*j)->GetName() << std::endl;
				if ((*i)->IsSpring() || (*j)->IsSpring()) {
					PenaltyResolveCollision(*info.a, *info.b, info.point);
				}
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);
				
			}
		}
	}
}

/*

In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out. 

*/
void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject * physA = a.GetPhysicsObject();
	PhysicsObject * physB = b.GetPhysicsObject();

	if (physA == nullptr || physB == nullptr) {
		return;
	}
	
	Transform & transformA = a.GetTransform();
	Transform & transformB = b.GetTransform();
	
	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();
	
	if (totalMass == 0) {
		return; //two static objects ??
	}

	const float allowedPenetration = 0.1f;

	const float biasFactor = 0.1f;
	float bias = biasFactor * (std::max)(0.0f, p.penetration - allowedPenetration);

	// Separate them out using projection
	transformA.SetPosition(transformA.GetPosition() -
			(p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));
	transformB.SetPosition(transformB.GetPosition() +
			(p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));

	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;
	
	Vector3 angVelocityA = Vector3::Cross(physA->GetAngularVelocity(), relativeA);
	Vector3 angVelocityB = Vector3::Cross(physB->GetAngularVelocity(), relativeB);
	
	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;


	Vector3 contactVelocity = fullVelocityB - fullVelocityA;

	float impulseForce = Vector3::Dot(contactVelocity, p.normal);
	
	if (-impulseForce < -0.01f) {
		return;
	}

	//now to work out the effect of inertia ....
	Vector3 inertiaA = Vector3::Cross(physA->GetInertiaTensor() * Vector3::Cross(relativeA, p.normal), relativeA);
	Vector3 inertiaB = Vector3::Cross(physB->GetInertiaTensor() * Vector3::Cross(relativeB, p.normal), relativeB);
	float angularEffect = Vector3::Dot(inertiaA + inertiaB, p.normal);
	float cRestitution = (physA->GetElasticity() + physB->GetElasticity()) / 2; // disperse some kinectic energy
	//float cFriction = physA->GetFriction() * physB->GetFriction();
	float cFriction = (physA->GetFriction() + physB->GetFriction()) / 2;

	//float massNormal = 1.0f / (totalMass + angularEffect);
	//float dPn = massNormal * (-impulseForce + bias);
	//dPn = max(dPn, 0.0f);
	float j = ((-(1.0f + cRestitution) * impulseForce)) / (totalMass + angularEffect);
	//j = max(j, 0.0f);
	//j = j + (p.penetration * 1.5);

	Vector3 tangent = (contactVelocity - (p.normal * impulseForce)).Normalised();
	float frictionForce = Vector3::Dot(contactVelocity, tangent);
	
	Vector3 frictionInertiaA = Vector3::Cross(physA->GetInertiaTensor() * Vector3::Cross(relativeA, tangent), relativeA);
	Vector3 frictionInertiaB = Vector3::Cross(physB->GetInertiaTensor() * Vector3::Cross(relativeB, tangent), relativeB);
	float frictionAngularEffect = Vector3::Dot(frictionInertiaA + frictionInertiaB, tangent);

	float jt = (-cFriction * frictionForce) / (totalMass + frictionAngularEffect);
	float maxJt = cFriction * j;
	jt = Maths::Clamp(jt, -maxJt, maxJt);  // Friction is proportional to impulse
	
	Vector3 frictionImpulse = tangent * jt;
	Vector3 fullImpulse = (p.normal * j);
	fullImpulse += frictionImpulse;

	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);
	physA->ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector3::Cross(relativeB, fullImpulse));
}


void PhysicsSystem::PenaltyResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	Vector3 springPosA = p.localA;
	Vector3 springPosB = p.localB;

	Vector3 springExtensionDir = p.normal;
	float springExtensionLength = p.penetration;

	float k = (physA->GetStiffness() + physB->GetStiffness()) / 2.0f;
	Vector3 hookeX = springExtensionDir * springExtensionLength;

	Vector3 fullforce = hookeX * -k;

	physA->AddForceAtLocalPosition(fullforce, springPosA);
	physB->AddForceAtLocalPosition(-fullforce, springPosB);
}

/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to. 

*/

void PhysicsSystem::BroadPhase() {
	broadphaseCollisions.clear();
	tree->Clear();
//	QuadTree <GameObject*> tree(Vector2(1024, 1024), 7, 6);
	std::vector <GameObject*>::const_iterator first;
	std::vector <GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;
		if (!(*i)->GetBroadphaseAABB(halfSizes)) {
			continue;
		}
		/*if ((*i)->GetPhysicsObject()) {
			if ((*i)->GetPhysicsObject()->IsStatic()) { // Possible move IsStatic to GameObject
				continue;
			}
		}*/
		Vector3 pos = (*i)->GetTransform().GetPosition();
		tree->Insert(*i, pos, halfSizes, (*i)->IsAsleep());
	}
	//tree.DebugDraw();
	if (useSleep) {
		bool a = true;
	}
	tree->OperateOnContents(
		[&](std::list <QuadTreeEntry <GameObject*>>& data) {
			CollisionDetection::CollisionInfo info;
			for (auto i = data.begin(); i != data.end(); ++i) {
				for (auto j = std::next(i); j != data.end(); ++j) {
					//is this pair of items already in the collision set -
					//if the same pair is in another quadtree node together etc
					info.a = (std::min)((*i).object, (*j).object);
					info.b = (std::max)((*i).object, (*j).object);
					int layerMask = (*i).object->GetLayerMask() & (*j).object->GetLayerMask();
					if (layerMask == 0) {
						continue;
					}
					broadphaseCollisions.insert(info);
				}
			}
	});

	// Static Objects
	/*std::vector <GameObject*>::const_iterator first2;
	std::vector <GameObject*>::const_iterator last2;
	gameWorld.GetObjectIterators(first2, last2, true);
	for (auto i = first2; i != last2; ++i) {
		QuadTreeNode<GameObject*>* nodePointer = tree->GetNodeToInsert((*i));

		//QuadTreeNode<GameObject*> node = nodePointer;
		/*if (nodePointer != nullptr) {
			nodePointer->OperateOnContents(
				[&](std::list <QuadTreeEntry <GameObject*>>& data) {
					CollisionDetection::CollisionInfo info;
					for (auto j = data.begin(); j != data.end(); ++j) {
						info.a = min((*i), (*j).object);
						info.b = max((*i), (*j).object);
						int layerMask = (*i)->GetLayerMask() & (*j).object->GetLayerMask();
						if (layerMask == 0) {
							continue;
						}
						broadphaseCollisions.insert(info);
					}
				
			});

		}
		if (nodePointer != nullptr) {
			std::list< QuadTreeEntry<GameObject*>> objects = nodePointer->GetContents();
			CollisionDetection::CollisionInfo info;
			for (auto j = objects.begin(); j != objects.end(); ++j) {
				info.a = min((*i), (*j).object);
				info.b = max((*i), (*j).object);
				int layerMask = (*i)->GetLayerMask() & (*j).object->GetLayerMask();
				if (layerMask == 0) {
					continue;
				}
				broadphaseCollisions.insert(info);
			}
		}
	}*/
	
}

/*

The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/
void PhysicsSystem::NarrowPhase() {
	for (std::set <CollisionDetection::CollisionInfo >::iterator
		i = broadphaseCollisions.begin();
		i != broadphaseCollisions.end(); ++i) {

		CollisionDetection::CollisionInfo info = *i;
		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			info.framesLeft = numCollisionFrames;
			if (info.a->IsSpring() || info.b->IsSpring()) {
				PenaltyResolveCollision(*info.a, *info.b, info.point);
			}
			else {
				ImpulseResolveCollision(*info.a, *info.b, info.point);
			}
			allCollisions.insert(info); // insert into our main set
			
		}
	}
}

/*
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc. 

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt, bool penalty) {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	
	for (auto i = first; i != last; ++i) {
		/*if (penalty && !(*i)->IsSpring()) {
			continue;
		}*/
		PhysicsObject * object = (*i)->GetPhysicsObject();
		if (object == nullptr || (*i)->IsAsleep()) {
			continue; 
		}
		float inverseMass = object->GetInverseMass();
		
	    Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;
		
		if (applyGravity && inverseMass > 0 && !(*i)->IsAsleep()) {
			accel += gravity; // don't move infinitely heavy things
			
		}
		
		linearVel += accel * dt; // integrate accel !
		object->SetLinearVelocity(linearVel);

		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();
		
		object->UpdateInertiaTensor(); // update tensor vs orientation
		
		Vector3 angAccel = object->GetInertiaTensor() * torque;
	
		angVel += angAccel * dt; // integrate angular accel !
		object->SetAngularVelocity(angVel);
	}
}
/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt) {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	
	for (auto i = first; i != last; ++i) {
		PhysicsObject * object = (*i)->GetPhysicsObject();
		if (object == nullptr || (*i)->IsAsleep()) {
			continue;
		}
		float frameLinearDamping = 1.0f - (object->GetLinearDamping() * dt);

		Transform & transform = (*i)->GetTransform();
		// Position Stuff
		Vector3 position = transform.GetPosition();
		Vector3 linearVel = object->GetLinearVelocity();
		position += linearVel * dt;
		transform.SetPosition(position);
		// Linear Damping
		linearVel = linearVel * frameLinearDamping;
		object->SetLinearVelocity(linearVel);
		
		Quaternion orientation = transform.GetOrientation();
		Vector3 angVel = object->GetAngularVelocity();

		if (angVel.x > 0 || angVel.y > 0 || angVel.z > 0) {
			//std::cout << "Hello";
		}
		
		orientation = orientation + 
			(Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();
		
		transform.SetOrientation(orientation);
		
		// Damp the angular velocity too
		float frameAngularDamping = 1.0f - (object->GetAngularDamping() * dt);
		angVel = angVel * frameAngularDamping;
		object->SetAngularVelocity(angVel);
	}
}

/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces() {
	gameWorld.OperateOnContents(
		[](GameObject* o) {
			PhysicsObject* obj = o->GetPhysicsObject();
			if (obj != nullptr) {
				obj->ClearForces();
			}
		}
	);
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc. 

*/
void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}