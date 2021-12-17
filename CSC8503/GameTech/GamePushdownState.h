#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"
#include "TutorialGame.h"

namespace NCL {

	namespace CSC8503 {
		class GameState;
		class GamePushdownState : public PushdownState {
		public:
			static void InitGameState();

			static Window* GetGameWindow();
		protected:
			static GameState* gameState;
		};

		class MainMenu : public PushdownState {
		public:
			PushdownResult OnUpdate(float dt,
				PushdownState** newState) override;
			void OnAwake() override;
			void OnSleep() override;
		private:
			void DrawMenu();
			int currentOption;
			const int MAX_OPTIONS = 3;
			GameTechRenderer* renderer;
			GameWorld* gameWorld;
		};

		class Game : public PushdownState {
		public:
			/*Game(GameWorld* world, GameTechRenderer* renderer) {
				gameWorld = world;
				this->renderer = renderer;
			}*/
			Game(int gameId = 1);
			PushdownResult OnUpdate(float dt,
				PushdownState** newState) override;
			void OnAwake() override;
			//void OnSleep() override;
		private:
			TutorialGame* game;
			int gameId;
			//	GameWorld* gameWorld;
			//	GameTechRenderer* renderer;

			int gameStatus;
		};

		class PauseMenu : public PushdownState {
		public:
			PauseMenu(TutorialGame* game);
			PushdownResult OnUpdate(float dt,
				PushdownState** newState) override;
			void OnAwake() override;
		private:
			TutorialGame* game;
			void DrawMenu();
		};

		class MazeGame : public PushdownState {
		public:
			PushdownResult OnUpdate(float dt,
				PushdownState** newState) override;
			void OnAwake() override;
		private:
		};

	}
}