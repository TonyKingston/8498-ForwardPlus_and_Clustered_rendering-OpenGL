#include "CollisionDetection.h"
#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "Common/Graphics/Window.h"
#include "Common/Math/Maths.h"
#include "Debug.h"

#include <list>

using namespace NCL;

const Vector3 worldUp = Vector3(0, 1, 0);

bool CollisionDetection::RayPlaneIntersection(const Ray&r, const Plane&p, RayCollision& collisions) {
	float ln = Vector3::Dot(p.GetNormal(), r.GetDirection());

	if (ln == 0.0f) {
		return false; //direction vectors are perpendicular!
	}
	
	Vector3 planePoint = p.GetPointOnPlane();

	Vector3 pointDir = planePoint - r.GetPosition();

	float d = Vector3::Dot(pointDir, p.GetNormal()) / ln;

	collisions.collidedAt = r.GetPosition() + (r.GetDirection() * d);
	collisions.rayDistance = d;

	return true;
}

bool CollisionDetection::RayIntersection(const Ray& r,GameObject& object, RayCollision& collision) {
	bool hasCollided = false;

	const Transform& worldTransform = object.GetTransform();
	const CollisionVolume* volume	= object.GetBoundingVolume();

	int layerMask = r.GetLayerMask() & object.GetLayerMask();

	if (!volume || layerMask == 0 || (object.GetLayerMask() & Layer::IgnoreRaycast) == Layer::IgnoreRaycast) {
		return false;
	}

	switch (volume->type) {
		case VolumeType::AABB:		hasCollided = RayAABBIntersection(r, worldTransform, (const AABBVolume&)*volume	, collision); break;
		case VolumeType::OBB:		hasCollided = RayOBBIntersection(r, worldTransform, (const OBBVolume&)*volume	, collision); break;
		case VolumeType::Sphere:	hasCollided = RaySphereIntersection(r, worldTransform, (const SphereVolume&)*volume	, collision); break;
		case VolumeType::Capsule:	hasCollided = RayCapsuleIntersection(r, worldTransform, (const CapsuleVolume&)*volume, collision); break;
	}

	return hasCollided;
}

bool CollisionDetection::RayBoxIntersection(const Ray&r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision, bool inside) {


	Vector3 boxMin = boxPos - boxSize;
	Vector3 boxMax = boxPos + boxSize;
	
	Vector3 rayPos = r.GetPosition();
	Vector3 rayDir = r.GetDirection();

	// Allow collision with box if ray starts inside one.
	if (inside && AABBTest(rayPos, boxPos, Vector3(0, 0, 0), boxSize)) {
		return true;
	}

	
	Vector3 tVals(-1, -1, -1);
	
	for (int i = 0; i < 3; ++i) { //get best 3 intersections
		if (rayDir[i] > 0) {
			tVals[i] = (boxMin[i] - rayPos[i]) / rayDir[i];
		}
		else if (rayDir[i] < 0) {
			tVals[i] = (boxMax[i] - rayPos[i]) / rayDir[i];
		}
		
	}
	float bestT = tVals.GetMaxElement();
	if (bestT < 0.0f) {
		return false; //no backwards rays!
	}
	Vector3 intersection = rayPos + (rayDir * bestT);
	const float epsilon = 0.0001f; //an amount of leeway in our calcs
	for (int i = 0; i < 3; ++i) {
		if (intersection[i] + epsilon < boxMin[i] ||
			intersection[i] - epsilon > boxMax[i]) {
		    return false; //best intersection doesn't touch the box!
			
		}
	}
	collision.collidedAt = intersection;
	collision.rayDistance = bestT;
	return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray&r, const Transform& worldTransform, const AABBVolume& volume, RayCollision& collision) {
	Vector3 boxPos = worldTransform.GetPosition();
	Vector3 boxSize = volume.GetHalfDimensions();
	return RayBoxIntersection(r, boxPos, boxSize, collision);
}

bool CollisionDetection::RayOBBIntersection(const Ray&r, const Transform& worldTransform, const OBBVolume& volume, RayCollision& collision) {
	Quaternion orientation = worldTransform.GetOrientation();
	Vector3 position = worldTransform.GetPosition();

	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());
	
	Vector3 localRayPos = r.GetPosition() - position;
	
	Ray tempRay(invTransform * localRayPos, invTransform * r.GetDirection());
	
	bool collided = RayBoxIntersection(tempRay, Vector3(),
			volume.GetHalfDimensions(), collision);
	
	if (collided) {
		collision.collidedAt = transform * collision.collidedAt + position;
		
	}
	return collided;
}

bool CollisionDetection::RayCapsuleIntersection(const Ray& r, const Transform& worldTransform, const CapsuleVolume& volume, RayCollision& collision) {
	Vector3 rayPos = r.GetPosition();
	Vector3 rayDir = r.GetDirection();
	float radius = volume.GetRadius();
	float halfHeight = volume.GetHalfHeight();
	
	Vector3 position = worldTransform.GetPosition();
	Vector3 project = Vector3::Cross(position + worldUp, rayDir);

	Vector3 topPosition = position + (worldUp * (halfHeight - radius));
	Vector3 point3 = position + Vector3::Cross(rayDir, (topPosition - position).Normalised());
	Plane plane = Plane::PlaneFromTri(position, topPosition - position, point3);

	RayPlaneIntersection(r, plane, collision);

	Vector3 vA = topPosition - collision.collidedAt;
	Vector3 vB = topPosition - position;
	float dotPos = Vector3::Dot(vA, vB);

	if (dotPos < 0) {
		float distance = (collision.collidedAt - topPosition).Length();
		if (distance <= radius) {
			return true;
		}
		return false;
	}

	Vector3 bottomPosition = position - (worldUp * (halfHeight - radius));
	vA = bottomPosition - collision.collidedAt;
	vB = bottomPosition - position;
	dotPos = Vector3::Dot(vA, vB);
	if (dotPos < 0) {
		float distance = (collision.collidedAt - bottomPosition).Length();
		if (distance <= radius) {
			return true;
		}
		return false;
	}

	Vector3 d = position + (worldUp * (Vector3::Dot(collision.collidedAt - position, worldUp)));
	float distance = (d - collision.collidedAt).Length();

	if (distance <= radius) {
		return true;
	}
	return false;
}

bool CollisionDetection::RaySphereIntersection(const Ray&r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision) {
	Vector3 spherePos = worldTransform.GetPosition();
	float sphereRadius = volume.GetRadius();

	//Get the direction between the ray origin and the sphere origin
	Vector3 dir = (spherePos - r.GetPosition());
	
	//Then project the sphere's origin onto our ray direction vector
	float sphereProj = Vector3::Dot(dir, r.GetDirection());

	if (sphereProj < 0.0f) {
		return false; // point is behind the ray!
	}

	//Get closest point on ray line to sphere
	Vector3 point = r.GetPosition() + (r.GetDirection() * sphereProj);

	float sphereDist = (point - spherePos).Length();

	if (sphereDist > sphereRadius) {
		return false;
	}
	float offset = sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));
	
    collision.rayDistance = sphereProj - (offset);
	collision.collidedAt = r.GetPosition() + (r.GetDirection() * collision.rayDistance);
	return true;
}

Matrix4 GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix4::Translation(position) *
		Matrix4::Rotation(-yaw, Vector3(0, -1, 0)) *
		Matrix4::Rotation(-pitch, Vector3(-1, 0, 0));

	return iview;
}

Vector3 CollisionDetection::Unproject(const Vector3& screenPos, const Camera& cam) {
	Vector2 screenSize = Window::GetWindow()->GetScreenSize();

	float aspect	= screenSize.x / screenSize.y;
	float fov		= cam.GetFieldOfVision();
	float nearPlane = cam.GetNearPlane();
	float farPlane  = cam.GetFarPlane();

	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(cam) * GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(screenPos.x / (float)screenSize.x) * 2.0f - 1.0f,
		(screenPos.y / (float)screenSize.y) * 2.0f - 1.0f,
		(screenPos.z),
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

Ray CollisionDetection::BuildRayFromMouse(const Camera& cam) {
	Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
	Vector2 screenSize	= Window::GetWindow()->GetScreenSize();

	//We remove the y axis mouse position from height as OpenGL is 'upside down',
	//and thinks the bottom left is the origin, instead of the top left!
	Vector3 nearPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		-0.99999f
	);

	//We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
	//causes the unproject function to go a bit weird. 
	Vector3 farPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	//std::cout << "Ray Direction:" << c << std::endl;

	return Ray(cam.GetPosition(), c);
}

Ray CollisionDetection::BuildRayFromObject(GameObject& obj, float maxRange) {
	Vector2 screenObj = Vector2(obj.GetTransform().GetPosition().x, obj.GetTransform().GetPosition().y);
	Vector2 screenSize = Window::GetWindow()->GetScreenSize();
	
	const CollisionVolume* volume = obj.GetBoundingVolume();
	Vector3 pos = obj.GetTransform().GetPosition();
	if (volume->type == VolumeType::Sphere) {
		const SphereVolume& sphere = (const SphereVolume&)* volume;
		pos.z += sphere.GetRadius();
		
	}
	return Ray(pos, obj.GetForwordDirection(), obj.GetLayerMask());
}

//http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	Matrix4 m;

	float t = tan(fov*PI_OVER_360);

	float neg_depth = nearPlane - farPlane;

	const float h = 1.0f / t;

	float c = (farPlane + nearPlane) / neg_depth;
	float e = -1.0f;
	float d = 2.0f*(nearPlane*farPlane) / neg_depth;

	m.array[0]  = aspect / h;
	m.array[5]  = tan(fov*PI_OVER_360);

	m.array[10] = 0.0f;
	m.array[11] = 1.0f / d;

	m.array[14] = 1.0f / e;

	m.array[15] = -c / (d*e);

	return m;
}

/*
And here's how we generate an inverse view matrix. It's pretty much
an exact inversion of the BuildViewMatrix function of the Camera class!
*/
Matrix4 CollisionDetection::GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
Matrix4::Translation(position) *
Matrix4::Rotation(yaw, Vector3(0, 1, 0)) *
Matrix4::Rotation(pitch, Vector3(1, 0, 0));

return iview;
}


/*
If you've read through the Deferred Rendering tutorial you should have a pretty
good idea what this function does. It takes a 2D position, such as the mouse
position, and 'unprojects' it, to generate a 3D world space position for it.

Just as we turn a world space position into a clip space position by multiplying
it by the model, view, and projection matrices, we can turn a clip space
position back to a 3D position by multiply it by the INVERSE of the
view projection matrix (the model matrix has already been assumed to have
'transformed' the 2D point). As has been mentioned a few times, inverting a
matrix is not a nice operation, either to understand or code. But! We can cheat
the inversion process again, just like we do when we create a view matrix using
the camera.

So, to form the inverted matrix, we need the aspect and fov used to create the
projection matrix of our scene, and the camera used to form the view matrix.

*/
Vector3	CollisionDetection::UnprojectScreenPosition(Vector3 position, float aspect, float fov, const Camera &c) {
	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(c) * GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());

	Vector2 screenSize = Window::GetWindow()->GetScreenSize();

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(position.x / (float)screenSize.x) * 2.0f - 1.0f,
		(position.y / (float)screenSize.y) * 2.0f - 1.0f,
		(position.z) - 1.0f,
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

bool CollisionDetection::ObjectIntersection(GameObject* a, GameObject* b, CollisionInfo& collisionInfo) {
	const CollisionVolume* volA = a->GetBoundingVolume();
	const CollisionVolume* volB = b->GetBoundingVolume();

	if (!volA || !volB) {
		return false;
	}

	collisionInfo.a = a;
	collisionInfo.b = b;

	Transform& transformA = a->GetTransform();
	Transform& transformB = b->GetTransform();

	VolumeType pairType = (VolumeType)((int)volA->type | (int)volB->type);
	if (pairType == VolumeType::AABB) {
		return AABBIntersection((AABBVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}

	if (pairType == VolumeType::Sphere) {
		return SphereIntersection((SphereVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}

	if (pairType == VolumeType::OBB) {
		return OBBIntersection((OBBVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}

	if (volA->type == VolumeType::AABB && volB->type == VolumeType::Sphere) {
		return AABBSphereIntersection((AABBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBSphereIntersection((AABBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	if (volA->type == VolumeType::OBB && volB->type == VolumeType::Sphere) {
		return OBBSphereIntersection((OBBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::OBB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBSphereIntersection((OBBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}
	if ((volA->type == VolumeType::AABB && volB->type == VolumeType::OBB) || (volA->type == VolumeType::OBB && volB->type == VolumeType::AABB)) {
		return OBBIntersection((OBBVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::Sphere) {
		return SphereCapsuleIntersection((CapsuleVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::Capsule) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return SphereCapsuleIntersection((CapsuleVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	return false;
}

bool CollisionDetection::AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB) {
	Vector3 delta = posB - posA;
	Vector3 totalSize = halfSizeA + halfSizeB;
		
	if (abs(delta.x) < totalSize.x &&
		abs(delta.y) < totalSize.y &&
		abs(delta.z) < totalSize.z) {
		return true;	
	}
	return false;
}

//AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	Vector3 boxAPos = worldTransformA.GetPosition();
	Vector3 boxBPos = worldTransformB.GetPosition();
	
	Vector3 boxASize = volumeA.GetHalfDimensions();
	Vector3 boxBSize = volumeB.GetHalfDimensions();
	
	bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);
	if (overlap) {
		static const Vector3 faces[6] =
			{
			Vector3(-1, 0, 0), Vector3(1, 0, 0),
				Vector3(0, -1, 0), Vector3(0, 1, 0),
				Vector3(0, 0, -1), Vector3(0, 0, 1),
			};
		
		Vector3 maxA = boxAPos + boxASize;
		Vector3 minA = boxAPos - boxASize;
		
		Vector3 maxB = boxBPos + boxBSize;
		Vector3 minB = boxBPos - boxBSize;
		
		float distances[6] = {
			(maxB.x - minA.x),// distance of box 'b' to 'left ' of 'a '.
				(maxA.x - minB.x),// distance of box 'b' to 'right ' of 'a '.
				(maxB.y - minA.y),// distance of box 'b' to 'bottom ' of 'a '.
				(maxA.y - minB.y),// distance of box 'b' to 'top ' of 'a '.
				(maxB.z - minA.z),// distance of box 'b' to 'far ' of 'a '.
				(maxA.z - minB.z) // distance of box 'b' to 'near ' of 'a '.
		};
		float penetration = FLT_MAX;
		Vector3 bestAxis;
		
		for (int i = 0; i < 6; i++) {
			if (distances[i] < penetration) {
				penetration = distances[i];
				bestAxis = faces[i];
			}
		}
		collisionInfo.AddContactPoint(Vector3(), Vector3(),
			bestAxis, penetration);
		return true;
	}
	return false;
}

//Sphere / Sphere Collision
bool CollisionDetection::SphereIntersection(const SphereVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	float radii = volumeA.GetRadius() + volumeB.GetRadius();
	Vector3 delta = worldTransformB.GetPosition() -
	worldTransformA.GetPosition();
		
	float deltaLength = delta.Length();
	
	if (deltaLength < radii) {
		float penetration = (radii - deltaLength);
		Vector3 normal = delta.Normalised();
		Vector3 localA = normal * volumeA.GetRadius();
		Vector3 localB = -normal * volumeB.GetRadius();
			
		collisionInfo.AddContactPoint(localA, localB, normal, penetration);
		return true;// we 're colliding !
	}
	return false;
}

//AABB - Sphere Collision
bool CollisionDetection::AABBSphereIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	Vector3 boxSize = volumeA.GetHalfDimensions();
	
	Vector3 delta = worldTransformB.GetPosition() -
	                worldTransformA.GetPosition();
	
	Vector3 closestPointOnBox = Maths::Clamp(delta, -boxSize, boxSize);
	Vector3 localPoint = delta - closestPointOnBox;
	float distance = localPoint.Length();
	
	if (distance < volumeB.GetRadius()) {// yes , we 're colliding !
		Vector3 collisionNormal = localPoint.Normalised();
		float penetration = (volumeB.GetRadius() - distance);
		
		Vector3 localA = Vector3();
		Vector3 localB = -collisionNormal * volumeB.GetRadius();
		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}
	return false;
}

bool CollisionDetection::OBBIntersection(
	const OBBVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Vector3 aPos = worldTransformA.GetPosition();
	Vector3 bPos = worldTransformB.GetPosition();

	Quaternion aOrient = worldTransformA.GetOrientation();
	Quaternion bOrient = worldTransformB.GetOrientation();

	Vector3 aDims = volumeA.GetHalfDimensions();
	Vector3 bDims = volumeB.GetHalfDimensions();

	static const Vector3 faces[3] =
	{
		Vector3(1, 0, 0),
		Vector3(0, 1, 0),
		Vector3(0, 0, 1)
	};

	Vector3 axis[15];

	for (int i = 0; i < 3; i++) 	{
		axis[i] = (aOrient * faces[i]);
	}

	for (int i = 3; i < 6; i++) 	{
		axis[i] = (bOrient * faces[i - 3]);
	}

	axis[6] = Vector3::Cross(axis[0], axis[3]).Normalised();
	axis[7] = Vector3::Cross(axis[0], axis[4]).Normalised();
	axis[8] = Vector3::Cross(axis[0], axis[5]).Normalised();

	axis[9] = Vector3::Cross(axis[1], axis[3]).Normalised();
	axis[10] = Vector3::Cross(axis[1], axis[4]).Normalised();
	axis[11] = Vector3::Cross(axis[1], axis[5]).Normalised();

	axis[12] = Vector3::Cross(axis[2], axis[3]).Normalised();
	axis[13] = Vector3::Cross(axis[2], axis[4]).Normalised();
	axis[14] = Vector3::Cross(axis[2], axis[5]).Normalised();

	float minPenetration = FLT_MAX;
	int bestAxis = -1;
	Vector3 localA;
	Vector3 localB;
	
	for (int i = 0; i < 15; i++) {
		//const float epsilon = 0.001f;

		// Continue if axes are parallel. Should probably use epilon to handling floating point error.
		if (axis[i] == Vector3(0, 0, 0)) {
			continue;
		}

		Vector3 maxExtentA = OBBSupport(worldTransformA, axis[i]);
		Vector3 minExtentA = OBBSupport(worldTransformA, -axis[i]);

		Vector3 maxExtentB = OBBSupport(worldTransformB, axis[i]);
		Vector3 minExtentB = OBBSupport(worldTransformB, -axis[i]);

		float aMax = Vector3::Dot(axis[i], maxExtentA);
		float aMin = Vector3::Dot(axis[i], minExtentA);
		float bMax = Vector3::Dot(axis[i], maxExtentB);
		float bMin = Vector3::Dot(axis[i], minExtentB);

		bool left = aMin >= bMin && aMin <= bMax;
		bool right = bMin >= aMin && bMin <=	aMax;
		if (left || right) {
			float p;
			p = left ? bMax - aMin : aMax - bMin;

			if (p < minPenetration) {
				minPenetration = p;
				bestAxis = i;
				Vector3 l = axis[i] * p;
				localA = left ? minExtentA - aPos : minExtentB - aPos + l;
				localB = left ? minExtentA - bPos + l : minExtentB - bPos;
			}
		}
		else {
			return false;
		}
		
	}

	Vector3 normal = axis[bestAxis];
	if (Vector3::Dot(normal, bPos - aPos) < 0) {
		normal = -normal;
	}
	collisionInfo.AddContactPoint(localA, localB, normal, minPenetration);

	return true;
}

bool NCL::CollisionDetection::OBBSphereIntersection(
	const OBBVolume& volumeA, const Transform& worldTransformA, 
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo)
{
	AABBVolume aabb = AABBVolume(volumeA.GetHalfDimensions());
	Transform transform1;
	Transform transform2;
	const Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();
	transform2.SetPosition(worldTransformA.GetOrientation().Conjugate() * delta);

	if (AABBSphereIntersection(aabb, transform1, volumeB, transform2, collisionInfo))
	{
		Vector3 closestPointOnBox = Maths::Clamp(delta, -volumeA.GetHalfDimensions(), volumeA.GetHalfDimensions());
		collisionInfo.point.normal = worldTransformA.GetOrientation() * collisionInfo.point.normal;
		collisionInfo.point.localA = worldTransformA.GetOrientation() * closestPointOnBox;
		collisionInfo.point.localB = worldTransformA.GetOrientation() * collisionInfo.point.localB;
		return true;
	}
	return false;
}

Vector3 NCL::CollisionDetection::OBBSupport(const Transform& worldTransform, const Vector3& worldDir) {
	Vector3 localDir = worldTransform.GetOrientation().Conjugate() * worldDir;
	Vector3 vertex;
	// Form a unit cube
	/*vertex.x = localDir.x < 0 ? -1 : 1;
	vertex.y = localDir.y < 0 ? -1 : 1;
	vertex.z = localDir.z < 0 ? -1 : 1;*/
	vertex.x = localDir.x < 0 ? -0.5 : 0.5;
	vertex.y = localDir.y < 0 ? -0.5 : 0.5;
	vertex.z = localDir.z < 0 ? -0.5 : 0.5;

	return worldTransform.GetMatrix() * vertex;
}

bool CollisionDetection::SphereCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Vector3 capsPos = worldTransformA.GetPosition();
	Vector3 spherePos = worldTransformB.GetPosition();

	Vector3 A = worldTransformA.GetPosition() + (worldUp * (volumeA.GetHalfHeight() - volumeA.GetRadius())); //upPos
	Vector3 B = worldTransformA.GetPosition() - (worldUp * (volumeA.GetHalfHeight() - volumeA.GetRadius())); // downPos

	Vector3 closestPoint = ClosestPointOnALine(A, B, spherePos);
	if (Vector3::Dot(A, closestPoint) < 0) {
		SphereVolume newSphere = SphereVolume(volumeA.GetRadius());
		Transform trans = Transform();
		trans.SetPosition(A);
		return SphereIntersection(newSphere, trans, volumeB, worldTransformB, collisionInfo);
	}
	SphereVolume newSphere = SphereVolume(volumeA.GetRadius());

	return SphereIntersection(newSphere, worldTransformA, volumeB, worldTransformB, collisionInfo);

	/*Vector3 C = worldTransformB.GetPosition();
	Vector3 A = worldTransformA.GetPosition() + (worldUp * (volumeA.GetHalfHeight() - volumeA.GetRadius())); //upPos
	Vector3 B = worldTransformA.GetPosition() - (worldUp * (volumeA.GetHalfHeight() - volumeA.GetRadius())); //downPos
	Vector3 AB = B - A;
	Vector3 AC = C - A;
	// This method will be more efficient than using PointFromTriangle
	Plane plane = Plane::PlaneFromTri(A, B, C);
	plane = Plane(Vector3::Cross(AB, AC), (A - B).Length());
	float dist2 = plane.DistanceFromPlane(C);
	//float dist2 = SquaredDistancePointLine(A, B, worldTransformB.GetPosition());


	float radiiSum = volumeA.GetRadius() + volumeB.GetRadius();
	return dist2 - volumeB.GetRadius() <= volumeA.GetRadius();*/
}

float CollisionDetection::SquaredDistancePointLine(const Vector3& A, const Vector3& B, const Vector3& C)
{
	Vector3 AB = B - A;
	Vector3 AC = C - A;
	Vector3 BC = C - B;
	float norm = Vector3::Dot(AB, AC);
	if (norm <= 0.0f) {
		return AC.LengthSquared();
	}
	float f = AB.LengthSquared();
	if (norm >= f) {
		return BC.LengthSquared(); 
	}
	return AC.LengthSquared() - (norm * norm) / f;
}


Vector3 CollisionDetection::ClosestPointOnALine(const Vector3& start, const Vector3& end, const Vector3& point)
{
	Vector3 AB = end - start;

	float t = Clamp(Vector3::Dot(point - start, AB) / AB.LengthSquared(), 0.0f, 1.0f);

	return start + (AB * t);
}
