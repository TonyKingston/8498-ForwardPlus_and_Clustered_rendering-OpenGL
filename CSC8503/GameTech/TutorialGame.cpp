#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "..//CSC8503Common/PositionConstraint.h"
#include "..//CSC8503Common/PositionOrientationConstraint.h"
#include "../../Common/Assets.h"
#include <fstream>

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() {
	world = new GameWorld();
	
	resourceManager = new OGLResourceManager();
	renderer = new GameTechRenderer(*world, resourceManager);
	//physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = false;
	inSelectionMode = false;
	inDebugMode = false;

	Debug::SetRenderer(renderer);

	//GameObject::InitObjects(this);
	InitialiseAssets(1);
}

//TutorialGame::TutorialGame(int level) {
//	world = new GameWorld();
//	renderer = new GameTechRenderer(*world);
//	physics = new PhysicsSystem(*world);
//	GameObject::InitObjects(this);
//
//	forceMagnitude = 10.0f;
//	useGravity = true;
//	physics->UseGravity(useGravity);
//	inSelectionMode = false;
//	inDebugMode = false;
//
//	Debug::SetRenderer(renderer);
//
//	//GameObject::InitObjects(this);
//	InitialiseAssets(level);
//}
//
//

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets(int level) {
	cubeMesh = resourceManager->LoadMesh("cube.msh");
	sphereMesh = resourceManager->LoadMesh("sphere.msh");
	capsuleMesh = resourceManager->LoadMesh("capsule.msh");
	basicTex = (OGLTexture*)resourceManager->LoadTexture("checkerboard.png");
	basicShader = (OGLShader*)resourceManager->LoadShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	InitSponza();
}

NCL::CSC8503::TutorialGame::TutorialGame(GameWorld* gameWorld, GameTechRenderer* gameRenderer) {
	world = gameWorld;
	renderer = gameRenderer;
	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = false;
	inSelectionMode = false;
	inDebugMode = false;
	Debug::SetRenderer(renderer);
	//GameObject::InitObjects(this);
	InitialiseAssets();
}

TutorialGame::~TutorialGame() {
	delete resourceManager;
	delete physics;
	delete renderer;
	delete world;
	delete grid;
}

void TutorialGame::UpdateGame(float dt) {
	timeTaken += dt;

	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}

	UpdateKeys();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95));
	}

	SelectObject();
	MoveSelectedObject();

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);

		//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
	}

	world->UpdateWorld(dt);
	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	if (inDebugMode) {
		world->DrawBoundingVolumes();
		renderer->Render();
	}
	else {
		renderer->Render();
	}
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject = nullptr;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
		InitCapsuleTest();
		selectionObject = nullptr;
		lockedObject = nullptr;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
		InitOBBTest();
		selectionObject = nullptr;
		lockedObject = nullptr;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM3)) {
		InitSleepTest();
		selectionObject = nullptr;
		lockedObject = nullptr;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM4)) {
		InitSpringTest();
		selectionObject = nullptr;
		lockedObject = nullptr;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::N)) {
		inDebugMode = !inDebugMode;
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F5)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F6)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 15.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		Vector3 worldPos = selectionObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
		//lockedObject->GetPhysicsObject()->AddTorque(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0, -force, 0));
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0, force, 0));
	}
}

void NCL::CSC8503::TutorialGame::ShowPlayerScore() {
	Debug::Print("Score: " + std::to_string(player->GetScore()), Vector3(5, 5, 5));
	Debug::Print("Time: " + std::to_string(timeTaken), Vector3(5, 10, 5));
}

void NCL::CSC8503::TutorialGame::EndGame(int status) {
	gameStatus = status;
}

void TutorialGame::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM0)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-5, 0, 0));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -20));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 20));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(-20, 0, 0));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(20, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM9)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(5, 0, 0));
		}
	}

}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	//world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	world->GetMainCamera()->SetPosition(Vector3(0, 0, 50));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	InitMixedGridWorld(5, 5, 3.5f, 3.5f);
	InitGameExamples();
	InitDefaultFloor();
	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
	testStateObject = nullptr;

	//BridgeConstraintTest();
}

void TutorialGame::InitSponza() {
	sponza =  new Model("backpack.obj", resourceManager);
	for (auto obj : sponza->objects) {
		world->AddGameObject(obj);
	}
}

void NCL::CSC8503::TutorialGame::InitPhysicsLevel() {
	world->ClearAndErase();
	//physics->Clear();
	LoadWorldFromFile("PhysicsGrid.txt");
	playerSpawn = Vector3(15, 5, 15);
	AddPlayerToWorld(playerSpawn);
	
	PendulumConstraint();
	AddKillPlaneToWorld(Vector3(0, -200, 0));
	AddVictoryTriggerToWorld(Vector3(165, -95, 420), Vector3(30, 1, 30));
	GameObject* floor = AddCubeToWorld(Vector3(165, -15, 113), Vector3(20, 2, 50), Vector3(15, 0, 0), true, 0.0f, Debug::CYAN);
	floor->GetPhysicsObject()->SetElasticity(0.9);
	floor->GetPhysicsObject()->SetFriction(0.1);
	GameObject* block = AddCubeToWorld(Vector3(170, -30, 200), Vector3(4, 4, 4), true, 0.8f, Debug::GRAY);
	GameObject* block2 = AddCubeToWorld(Vector3(160, -30, 200), Vector3(4, 4, 4), true, 0.8f, Debug::GRAY);
	GameObject* target = AddCubeToWorld(Vector3(165, -35, 300), Vector3(20, 5, 10), false, 0.0f, Debug::GRAY);
	JumpPadObject* pad = AddJumpPadToWorld(target->GetTransform().GetPosition() - Vector3(0, 10, 60), Vector3(10,5,5), target->GetTransform().GetPosition() + Vector3(0,20,0));
	GameObject* sphere1 = AddSphereToWorld(Vector3(target->GetTransform().GetPosition() + Vector3(10,10,0)), 2, 5.0f, true);
	sphere1->SetAsSpring();
	//player->GetTransform().SetPosition(pad->GetTransform().GetPosition() + Vector3(0, 15, 0));
	LoadWorldFromFile("PhysicsGridF2.txt");
	LoadWorldFromFile("PhysicsGridF3.txt");
	LoadWorldFromFile("PhysicsGridF4.txt");

}

void TutorialGame::InitCapsuleTest() {
	world->ClearAndErase();
	physics->Clear();
	Vector3 position = Vector3(0, 10.0f, 0);
	AddCapsuleToWorld(position, 5, 2);
	AddSphereToWorld(position + Vector3(10, 0, 0), 2);
}

void NCL::CSC8503::TutorialGame::InitOBBTest() {
	world->ClearAndErase();
	physics->Clear();
	Vector3 position = Vector3(0, 10.0f, 0);
	Vector3 cubeDims = Vector3(5, 2, 2);
	AddCubeToWorld(position, cubeDims, true, 1.0f);
	AddCubeToWorld(position - Vector3(10.0f, 0, 10.0f), cubeDims, true, 1.0f);
	AddSphereToWorld(position + Vector3(10, 0, 0), 2, 10.0f);
}

void NCL::CSC8503::TutorialGame::InitSleepTest() {
	world->ClearAndErase();
	physics->Clear();
	InitDefaultFloor();
	Vector3 position = Vector3(0, 0, 0);
	AddSphereToWorld(position + Vector3(10, 10, 0), 2, 10.0f);
	//AddSphereToWorld(position + Vector3(15, 0, 0), 2, 10.0f);
	//AddSphereToWorld(position + Vector3(10, 0, 5), 2, 10.0f);
//	AddSphereToWorld(position + Vector3(15, 0, 5), 2, 10.0f);
	AddCubeToWorld(position + Vector3(15, 0, 0), Vector3(1, 1, 1), false, 0.0f);
	AddCubeToWorld(position + Vector3(15, 0, 5), Vector3(1, 1, 1), false, 0.0f);
	AddCubeToWorld(position + Vector3(15, 0, 10), Vector3(1, 1, 1), false, 0.0f);

}

void NCL::CSC8503::TutorialGame::InitSpringTest() {
	world->ClearAndErase();
	physics->Clear();
	Vector3 position = Vector3(0, 10.0f, 0);
	Vector3 cubeDims = Vector3(5, 2, 2);
	GameObject* obj1 = AddCubeToWorld(position, cubeDims, false, 1.0f);
	obj1->SetAsSpring();
	GameObject* obj2 = AddCubeToWorld(position - Vector3(10.0f, 0, 10.0f), cubeDims, false, 1.0f);
	AddSphereToWorld(position + Vector3(10, 0, 0), 2, 10.0f);
	GameObject* sphere = AddSphereToWorld(position + Vector3(20, 0, 0), 2, 10.0f);
	sphere->SetAsSpring();
	InitDefaultFloor();
}

void NCL::CSC8503::TutorialGame::LoadWorldFromFile(const string& filename) {
	std::ifstream infile(Assets::DATADIR + filename);

	int nodeSize;
	int nodeHeight;
	int mapWidth;
	int mapHeight;
	Vector3 offset;

	infile >> nodeSize;
	infile >> nodeHeight;
	infile >> mapWidth;
	infile >> mapHeight;
	infile >> offset.x;
	infile >> offset.y;
	infile >> offset.z;

	char* mapData = new char[mapWidth * mapHeight];

	for (int y = 0; y < mapHeight; ++y) {
		for (int x = 0; x < mapWidth; ++x) {
			//int tileIndex = ((mapHeight - 1 - y) * mapWidth) + x;
			int tileIndex = (mapWidth * y) + x;
			infile >> mapData[tileIndex];
		}
	}

	for (int y = 0; y < mapHeight; ++y) {
		for (int x = 0; x < mapWidth; ++x) {
			int tileType = mapData[(y * mapWidth) + x];

			switch (tileType) {
			case '.':
				continue;
				break;
			case 'w':
				float t = ((float)mapHeight / 2) * nodeSize;
				AddCubeToWorld(Vector3(offset.x + x * nodeSize, offset.y + nodeSize / 2, offset.z + (mapHeight / 2) * nodeSize),
					Vector3(nodeSize / 2, nodeHeight / 2, (mapHeight / 2) * nodeSize), false, 0.0f);
				break;
			}

		}
	}

	int lastPos = 0;
	for (int y = 0; y < mapHeight; ++y) {
		int tileCount = 0;
		for (int x = 0; x <= mapWidth; ++x) {
			char tileType = mapData[(y * mapWidth) + x];
			if (tileType == '.' || x == mapWidth) {
				if (tileCount > 0) {
					float midPoint = (float)((lastPos - tileCount + 1.0) + lastPos) / 2.0f;
					//AddCubeToWorld(Vector3(y * nodeSize, nodeSize / 2, midPoint * nodeSize), Vector3(((tileCount * nodeSize) / 2), nodeSize / 2, nodeSize / 2), Vector3(0, 90, 0), 0.0f);
				//	AddCubeToWorld(Vector3(y * nodeSize, nodeSize/2, midPoint * nodeSize), Vector3(nodeSize/2, nodeSize /2 , (tileCount * nodeSize) / 2), 0.0f);
					AddCubeToWorld(Vector3(offset.x + midPoint * nodeSize, offset.y + nodeSize / 2, offset.z + y * nodeSize), Vector3((tileCount * nodeSize) / 2, nodeHeight / 2, nodeSize / 2), false, 0.0f);
				}
				tileCount = 0;
			}
			else if (tileType == 'x') {
				tileCount += 1;
				lastPos = x;
			}
		}

	}

	AddCubeToWorld(Vector3(offset.x + (mapWidth / 2) * nodeSize, offset.y + 0, offset.z + (mapHeight / 2) * nodeSize), Vector3(nodeSize * mapWidth / 2, 2, nodeSize * mapHeight / 2), false, 0.0f);

}


void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);

	float invCubeMass = 5; //how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(0, 0, 0);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0)
		, cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2)
		* cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) *
			cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous,
			block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;

	}
	PositionConstraint* constraint = new PositionConstraint(previous,
		end, maxDistance);
	world->AddConstraint(constraint);
}


void NCL::CSC8503::TutorialGame::PendulumConstraint() {
	Vector3 cubeSize = Vector3(2, 2, 2);

	Vector3 startPos = playerSpawn + Vector3(100, 18, 0);


	GameObject* anchor = AddCubeToWorld(startPos + Vector3(0, 0, 0)
		, cubeSize, 0);
	anchor->GetPhysicsObject()->SetInverseMass(0.0f);

	GameObject* pendulum = AddPendulumToWorld(startPos + Vector3(0, -5, 10));

	PositionOrientationConstraint* constraint = new PositionOrientationConstraint(pendulum, anchor, 15, Vector3(0, 1, 0));
	world->AddConstraint(constraint);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("floor");

	Vector3 floorSize = Vector3(100, 2, 100);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();
	//floor->GetPhysicsObject()->SetFriction(0.5f);
	floor->GetPhysicsObject()->SetIsStatic(true);

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, bool hollow) {
	GameObject* sphere = new GameObject("sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	volume->SetObject(sphere);
	volume->SetVolumeMesh(sphereMesh);


	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia(hollow);
	sphere->GetPhysicsObject()->SetElasticity(0.8);


	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject("capsule");

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;

}

PlayerObject* NCL::CSC8503::TutorialGame::AddPlayerToWorld(const Vector3& position) {
	PlayerObject* sphere = new PlayerObject();

	float radius = 2.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	volume->SetObject(sphere);
	volume->SetVolumeMesh(sphereMesh);


	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader, Debug::GREEN));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(1.0f);
	sphere->GetPhysicsObject()->InitSphereInertia(false);
	sphere->GetPhysicsObject()->SetElasticity(0.7);
	sphere->GetPhysicsObject()->SetFriction(0.4);


	world->AddGameObject(sphere);
	this->player = sphere;

	return sphere;
}

EnemyObject* NCL::CSC8503::TutorialGame::AddEnemyToWorld(const Vector3& position) {
	EnemyObject* sphere = new EnemyObject(player);

	float radius = 2.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	volume->SetObject(sphere);
	volume->SetVolumeMesh(sphereMesh);


	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, NULL, basicShader, Debug::RED));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(1.0f);
	sphere->GetPhysicsObject()->InitSphereInertia(false);
	sphere->GetPhysicsObject()->SetElasticity(0.7);

	world->AddGameObject(sphere);

	return sphere;
}

JumpPadObject* NCL::CSC8503::TutorialGame::AddJumpPadToWorld(const Vector3& position, Vector3 dimensions, Vector3 target) {
	JumpPadObject* jumpPad = new JumpPadObject(player);

	AABBVolume* volume = new AABBVolume(dimensions);
	jumpPad->SetBoundingVolume((CollisionVolume*)volume);

	jumpPad->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);
	jumpPad->SetRenderObject(new RenderObject(&jumpPad->GetTransform(), cubeMesh, basicTex, basicShader, Debug::GREEN));
	jumpPad->SetPhysicsObject(new PhysicsObject(&jumpPad->GetTransform(), jumpPad->GetBoundingVolume()));
	jumpPad->GetPhysicsObject()->SetInverseMass(0.0f);
	jumpPad->GetPhysicsObject()->SetIsStatic(true);
	jumpPad->SetRange(dimensions.y * 2);
	jumpPad->SetTarget(target);
	world->AddGameObject(jumpPad);
	return jumpPad;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, bool isOBB, float inverseMass, Vector4 colour) {
	GameObject* cube = new GameObject("cube");

	if (isOBB) {
		OBBVolume* volume = new OBBVolume(dimensions);
		cube->SetBoundingVolume((CollisionVolume*)volume);

	}
	else {
		AABBVolume* volume = new AABBVolume(dimensions);
		cube->SetBoundingVolume((CollisionVolume*)volume);
	}

	//	volume->SetVolumeMesh(cubeMesh);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader, colour));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}


GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, Vector3 orientation, bool isOBB, float inverseMass, Vector4 colour) {
	GameObject* cube = new GameObject("cube");
	CollisionVolume* volume;

	if (isOBB) {
		volume = new OBBVolume(dimensions);
		cube->SetBoundingVolume(volume);

	}
	else {
		volume = new AABBVolume(dimensions);
		cube->SetBoundingVolume(volume);
	}

	volume->SetVolumeMesh(cubeMesh);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2)
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(orientation.x, orientation.y, orientation.z));

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader, colour));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* NCL::CSC8503::TutorialGame::AddCubeOBBToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("cube");

	OBBVolume* volume = new OBBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* NCL::CSC8503::TutorialGame::AddKillPlaneToWorld(const Vector3& position) {
	GameObject* plane = new GameObject("kill plane");
	Vector3 dimensions = Vector3(500, 1, 500);
	AABBVolume* volume = new AABBVolume(dimensions);
	plane->SetBoundingVolume(volume);
	plane->SetLayerMask(Layer::IgnoreRaycast | Layer::Player);

	plane->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	//plane->SetPhysicsObject(new PhysicsObject(&plane->GetTransform(), plane->GetBoundingVolume()));
	//	plane->GetPhysicsObject()->SetInverseMass(0.0f);
//	plane->SetRenderObject(new RenderObject(&plane->GetTransform(), cubeMesh, NULL, basicShader, Vector4(0, 0, 0, 0)));
	plane->SetTrigger();


	plane->SetTriggerFunc([&](GameObject* otherObj) {
		if (otherObj->GetName() == "player") {
			player->GetTransform().SetPosition(playerSpawn);
			player->TakeLife();
			return;
		}
		otherObj->Deactivate();
		});

	world->AddGameObject(plane);

	return plane;
}

GameObject* NCL::CSC8503::TutorialGame::AddVictoryTriggerToWorld(const Vector3& position, Vector3 dimensions) {
	GameObject* plane = new GameObject("victory plane");
	AABBVolume* volume = new AABBVolume(dimensions);
	plane->SetBoundingVolume(volume);
	plane->SetLayerMask(Layer::IgnoreRaycast | Layer::Player);


	plane->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	//plane->SetRenderObject(new RenderObject(&plane->GetTransform(), cubeMesh, NULL, basicShader, Vector4(0, 0, 0, 0)));
	plane->SetTrigger();


	plane->SetTriggerFunc([&](GameObject* otherObj) {
		if (otherObj->GetName() == "player") {
			EndGame(1);
			return;
		}
		});

	world->AddGameObject(plane);

	return plane;
}

PendulumObject* NCL::CSC8503::TutorialGame::AddPendulumToWorld(const Vector3& position) {
	PendulumObject* sphere = new PendulumObject("pendulum");

	float radius = 3.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	volume->SetObject(sphere);
	volume->SetVolumeMesh(sphereMesh);


	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(0.4f);
	sphere->GetPhysicsObject()->InitSphereInertia(false);
	sphere->GetPhysicsObject()->SetElasticity(0.7);
	sphere->GetPhysicsObject()->SetFriction(0.4);
	sphere->GetPhysicsObject()->SetLinearDamping(0.0); // Pendulum will always swing, no need for state machine
//	sphere->GetPhysicsObject()->SetAngularDamping(0.0);


	world->AddGameObject(sphere);

	return sphere;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims, true);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitGameExamples() {
	//AddPlayerToWorld(Vector3(0, 5, 0));
	//AddEnemyToWorld(Vector3(5, 5, 0));
	//AddBonusToWorld(Vector3(10, 5, 0));
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
				lockedObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			RayCollision c2;

			if (world->Raycast(ray, closestCollision, true)) {
				Debug::DrawLine(ray.GetPosition(), closestCollision.collidedAt, Debug::RED, 10.0f);
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				ray = CollisionDetection::BuildRayFromObject(*selectionObject, 100);
				if (world->Raycast(ray, c2, true)) {
					Debug::DrawLine(ray.GetPosition(), c2.collidedAt, Debug::RED, 10.0f);
				}
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if (selectionObject) {
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
		selectionObject->PrintDebugInfo();
		selectionObject->OnClick();
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

	}

	return false;
}


/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject() {
	renderer->DrawString(" Click Force :" + std::to_string(forceMagnitude),
		Vector2(10, 20)); // Draw debug text at 10 ,20
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PLUS)) {  // Add force when using laptop
		forceMagnitude += 10.0f;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::MINUS)) {
		forceMagnitude -= 10.0f;
	}

	if (!selectionObject) {
		return;
	}
	// Push the selected object !
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(
			*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				//selectionObject->GetPhysicsObject()->
					//AddForce(ray.GetDirection() * forceMagnitude);
				selectionObject->GetPhysicsObject()->
					AddForceAtPosition(ray.GetDirection() * forceMagnitude,
						closestCollision.collidedAt);
			}
		}
	}
}