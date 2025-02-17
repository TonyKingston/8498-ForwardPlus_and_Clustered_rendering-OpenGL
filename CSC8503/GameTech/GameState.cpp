#include "GameState.h"
#include "TutorialGame.h"
NCL::CSC8503::GameState::GameState() {
	w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
	gameWorld = new GameWorld();
	renderer = new GameTechRenderer(*gameWorld, &OGLResourceManager::Get());
	game = new TutorialGame(gameWorld, renderer);
}
