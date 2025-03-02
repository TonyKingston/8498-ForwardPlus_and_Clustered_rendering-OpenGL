#include "TutorialGame.h"
#include "CSC8503Common/GameWorld.h"
#include "Plugins/OpenGLRendering/OGLMesh.h"
#include "Plugins/OpenGLRendering/OGLShader.h"
#include "Plugins/OpenGLRendering/OGLTexture.h"
#include "Common/Graphics/TextureLoader.h"
#include "Common/Resources/Assets.h"
#include "CSC8503Common/PositionConstraint.h"
#include "CSC8503Common/PositionOrientationConstraint.h"
#include <fstream>
#include <algorithm>

using namespace NCL;
using namespace CSC8503;

using std::cout;
using std::endl;
using std::cin;

TutorialGame::TutorialGame() {
	world = new GameWorld();
	
	resourceManager = &OGLResourceManager::Get();
	bool prepass = false;
	int mode = AskRenderingMode();
	if (mode == 0 || mode == 3) {
		prepass = AskPrepass();
	}
	else if (mode == 2) {
		prepass = AskForwardPlus();
	}

	InitCamera();

	renderer = new GameTechRenderer(*world, resourceManager, mode, prepass);
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
	basicShader = (OGLShader*)resourceManager->LoadShader("GameTechVert.vert", "GameTechFrag.frag");

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

int TutorialGame::AskRenderingMode() {
	int input;
	int options[4] = { 1, 2, 3, 4 };
	cout << "\nPlease select a rendering mode from the following:\n" << endl;
	cout << "0. Forward" << endl;
	cout << "1. Deferred" << endl;
	cout << "2. Forward+" << endl;
	cout << "3. Clustered" << endl;

	cin >> input;
	if (cin.fail()) {
		cout << "Invalid Input" << endl;
		cin.clear();
		cin.ignore(256, '\n');
		AskRenderingMode();
	}

	switch (input) {
	case 0:
		cout << "Forward rendering selected." << endl;
		break;
	case 1:
		cout << "Deferred rendering selected." << endl;
		break;
	case 2:
		cout << "Forward+ rendering selected." << endl;
		break;
	case 3:
		cout << "Clustered rendering selected." << endl;
		break;
	default:
		cout << "Invalid input." << endl;
		cin.ignore();
		AskRenderingMode();
		break;
	}

	return input;
}

bool TutorialGame::AskPrepass() {
	int input;
	int options[4] = { 1, 2, 3, 4 };
	cout << "\nRender with depth prepass? 0 for no, 1 for yes:\n" << endl;

	cin >> input;
	if (cin.fail()) {
		cout << "Invalid Input" << endl;
		cin.clear();
		cin.ignore(256, '\n');
		AskRenderingMode();
	}
	switch (input) {
	case 0:
		cout << "Opted for no depth prepass." << endl;
		return false;
		break;
	case 1:
		cout << "Opted for depth prepass." << endl;
		return true;
		break;
	default:
		cout << "Invalid input." << endl;
		cin.ignore();
		AskPrepass();
		break;
	}

	return 0;

}

bool TutorialGame::AskForwardPlus() {
	int input;
	int options[4] = { 1, 2, 3, 4 };
	cout << "\nUse AABBs for light culling? 0 for no, 1 for yes:\n" << endl;

	cin >> input;
	if (cin.fail()) {
		cout << "Invalid Input" << endl;
		cin.clear();
		cin.ignore(256, '\n');
		AskForwardPlus();
	}
	switch (input) {
	case 0:
		cout << "Opted to not use AABBs." << endl;
		return false;
		break;
	case 1:
		cout << "Opted to use AABBs." << endl;
		return true;
		break;
	default:
		cout << "Invalid input." << endl;
		cin.ignore();
		AskForwardPlus();
		break;
	}

	return 0;
}

void TutorialGame::UpdateGame(float dt) {
	timeTaken += dt;

	world->GetMainCamera()->UpdateCamera(dt);

	UpdateKeys();

	//world->UpdateWorld(dt);
	renderer->Update(dt);
//	renderer->UpdateLights(dt);
	renderer->UpdateLightsGPU(dt);

	//Debug::FlushRenderables(dt);
	renderer->Render();
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject = nullptr;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
		renderer->AddLights(lightsToAdd);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
		Vector3 pos = world->GetMainCamera()->GetPosition();
		cout << "Camera Position (x,y,z): " << pos.x << " " << pos.y << " " << pos.z << endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM9)) {
		renderer->ToggleDebugMode();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::MINUS)) {
		lightsToAdd = (std::max)(lightsToAdd / 2, 1u);
		cout << "Decreased lights to " << lightsToAdd << endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PLUS)) {
		lightsToAdd = lightsToAdd * 2;
		cout << "Increased lights to " << lightsToAdd << endl;
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(2.0f);
	world->GetMainCamera()->SetFarPlane(1150.0f / WORLD_SCALE);
	//world->GetMainCamera()->SetFarPlane(575.0f);
	world->GetMainCamera()->SetYaw(270.0f);
	world->GetMainCamera()->SetPitch(-5.5f);

	world->GetMainCamera()->SetPosition(Vector3(-530, 71, -12) / WORLD_SCALE);
	//world->GetMainCamera()->SetPosition(Vector3(-265, 35, -6.5));
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
	sponza =  new Model("sponza.obj", resourceManager);
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