#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "CSC8503Common/PushdownState.h"
#include "CSC8503Common/PushdownMachine.h"
#include "CSC8503Common/PushdownState.h"
#include "TutorialGame.h"

namespace NCL {
	class Window;

	namespace CSC8503 {
		class GameState;
		class GamePushdownState : public PushdownState {
		public:
			static void InitGameState();

			static NCL::Window* GetGameWindow();
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
			Game(int gameId = 1);
			PushdownResult OnUpdate(float dt,
				PushdownState** newState) override;
			void OnAwake() override;
		private:
			TutorialGame* game;
			int gameId;

		};


		class Results : public PushdownState {
		public:
			Results(TutorialGame* game);
			PushdownResult OnUpdate(float dt,
				PushdownState** newState) override;
			void OnAwake() override;
			void OnSleep() override;
		private:
			int score;
			int enemyScore;
			float time;
			TutorialGame* game;
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