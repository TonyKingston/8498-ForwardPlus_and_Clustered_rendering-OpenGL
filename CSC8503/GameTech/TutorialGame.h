#pragma once
#include "GameTechRenderer.h"
#include "CSC8503Common/PhysicsSystem.h"
#include "CSC8503Common/StateGameObject.h"
#include "CSC8503Common/PlayerObject.h"
#include "CSC8503Common/EnemyObject.h"
#include "CSC8503Common/BonusObject.h"
#include "CSC8503Common/NavigationGrid.h"
#include "CSC8503Common/PendulumObject.h"
#include "CSC8503Common/JumpPadObject.h"
#include "Plugins/OpenGLRendering/OGLResourceManager.h"
#include "Plugins/OpenGLRendering/Model.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class TutorialGame		{
		public:
			TutorialGame();
			TutorialGame(int level);
			TutorialGame(GameWorld* gameWorld, GameTechRenderer* gameRenderer);
			~TutorialGame();

			virtual void UpdateGame(float dt);
			GameWorld* GetWorld() {
				return world;
			}

			GameTechRenderer* GetRenderer() {
				return renderer;
			}

			int GetPlayerScore() { return player->GetScore(); }
			int GetEnemyScore() {
				if (enemy) {
					return enemy->GetScore();
				}
				else {
					return -1;
				}
			}
			float GetGameTime() { return timeTaken;}
			int GetGameStatus() { return gameStatus; }

			int AskRenderingMode();
			bool AskPrepass();
			bool AskForwardPlus();

		protected:
			void InitialiseAssets(int level = 1);

			void InitCamera();
			void UpdateKeys();

			void InitWorld();
			void InitSponza();
			void InitPhysicsLevel();
			void InitCapsuleTest();
			void InitOBBTest();
			void InitSleepTest();
			void InitSpringTest();

			void LoadWorldFromFile(const std::string& filename);

			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();
			void BridgeConstraintTest();
			void PendulumConstraint();
	
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, bool hollow = false);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, bool isOBB = false, float inverseMass = 10.0f, Vector4 colour = Debug::GRAY);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, Vector3 orientation = Vector3(), bool isOBB = false, float inverseMass = 10.0f, Vector4 colour = Debug::GRAY);
			GameObject* AddCubeOBBToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddKillPlaneToWorld(const Vector3& position);
			GameObject* AddVictoryTriggerToWorld(const Vector3& position, Vector3 dimensions);
			PendulumObject* AddPendulumToWorld(const Vector3& position);
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);
			PlayerObject* AddPlayerToWorld(const Vector3& position);
			EnemyObject* AddEnemyToWorld(const Vector3& position);
			JumpPadObject* AddJumpPadToWorld(const Vector3& position, Vector3 dimensions, Vector3 target);
			StateGameObject* testStateObject;

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;
			bool inDebugMode;

			float		forceMagnitude;
			int gameStatus = 0;
			float timeTaken = 0;

			GameObject* selectionObject = nullptr;
			PlayerObject* player;
			EnemyObject* enemy;
			NavigationGrid* grid = nullptr;
			Vector3 playerSpawn;


			MeshGeometry*	capsuleMesh = nullptr;
			MeshGeometry*	cubeMesh	= nullptr;
			MeshGeometry*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			ResourceManager* resourceManager;
			Model* sponza;

			const int GAME_LENGTH = 180.0f;

			unsigned int lightsToAdd = 16;

		};
	}
}

