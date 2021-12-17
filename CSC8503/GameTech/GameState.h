#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "TutorialGame.h"

namespace NCL {

	namespace CSC8503 {

		class GameState {
		public:
			GameState();
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
	}
}