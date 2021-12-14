#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "TutorialGame.h"

namespace NCL {
	namespace CSC8503 {

		class GameState {
		public:
			GameState() {
				w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
				gameWorld = new GameWorld();
				renderer = new GameTechRenderer(*gameWorld);
				game = new TutorialGame(gameWorld, renderer);
				
			}
			~GameState() {
				delete game;
			}
			GameWorld* GetWorld() { return gameWorld; }
			GameTechRenderer* GetRenderer() { return renderer; }
			TutorialGame* GetGame() { return game; }
			Window* GetWindow() { return w; }

		private:
			GameWorld* gameWorld;
			GameTechRenderer* renderer;
			TutorialGame* game;
			Window* w;
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
		//	GameWorld* gameWorld;
		};

		class Game : public PushdownState {
		public:
			/*Game(GameWorld* world, GameTechRenderer* renderer) {
				gameWorld = world;
				this->renderer = renderer;
			}*/
			PushdownResult OnUpdate(float dt,
				PushdownState** newState) override;
			void OnAwake() override;
			//void OnSleep() override;
		private:
			TutorialGame* game;
		//	GameWorld* gameWorld;
			//GameTechRenderer* renderer;

			int gameStatus;
		};

		class PauseMenu : public PushdownState {
		public:
			PushdownResult OnUpdate(float dt,
				PushdownState** newState) override;
			void OnAwake() override;
		private:
			void DrawMenu();
			int currentOption;
			const int MAX_OPTIONS = 3;
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