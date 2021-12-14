#include "GameState.h"
#include "../../Common/Window.h"
#include "../CSC8503Common/Debug.h"
#include "../CSC8503Common/PushdownState.h"
using namespace NCL::CSC8503;

GameState gameState = GameState();


PushdownState::PushdownResult MainMenu::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP)) {
		currentOption = currentOption > 0 ? currentOption - 1 : MAX_OPTIONS - 1;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN)) {
		currentOption = currentOption < MAX_OPTIONS - 1 ? currentOption + 1 : 0;
	}
	//Debug::FlushRenderables(dt);
	renderer->Update(dt);
	renderer->Render();
	DrawMenu();

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN)) {
		switch (currentOption) {
		case 0:
			*newState = new Game();
			return PushdownResult::Push;
			break;
		case 1:
			// *newState = ai
			break;
		case 2:
			return PushdownResult::Pop;
			break;
		}
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}


void MainMenu::OnAwake() {
	//gameWorld = new GameWorld();
	//renderer = new GameTechRenderer(*gameWorld);
	renderer = gameState.GetRenderer();
	DrawMenu();
}

void MainMenu::OnSleep() {

}

void MainMenu::DrawMenu() {
	Vector2 pos1(35, 35);
	Vector2 pos2(35, 45);
	Vector2 pos3(35, 55);
	Vector4 selectedColour = Debug::GREEN;
	Vector4 normalColour = Debug::BLACK;
	
	switch (currentOption) {
	case 0:
		renderer->DrawString("Curveball Clammer", pos1, selectedColour);
		renderer->DrawString("Maze of Terror", pos2, normalColour);
		renderer->DrawString("Quit", pos3, normalColour);
		break;
	case 1:
		renderer->DrawString("Curveball Clammer", pos1, normalColour);
		renderer->DrawString("Maze of Terror", pos2, selectedColour);
		renderer->DrawString("Quit", pos3, normalColour);
		break;
	case 2:
		renderer->DrawString("Curveball Clammer", pos1, normalColour);
		renderer->DrawString("Maze of Terror", pos2, normalColour);
		renderer->DrawString("Quit", pos3, selectedColour);
		break;
	}
}

PushdownState::PushdownResult Game::OnUpdate(float dt, PushdownState** newState) {
	if (game) {
		game->UpdateGame(dt);

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
			*newState = new PauseMenu();
			
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}
}
void Game::OnAwake() {
	game = gameState.GetGame();
}

/*void Game::OnSleep() {
	delete game;
}*/

PushdownState::PushdownResult PauseMenu::OnUpdate(float dt, PushdownState** newState) {

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
		return PushdownResult::Pop;
	}
	
	Debug::FlushRenderables(dt);
	DrawMenu();
	return PushdownResult::NoChange;
}

void NCL::CSC8503::PauseMenu::OnAwake() {
	DrawMenu();
}

void NCL::CSC8503::PauseMenu::DrawMenu() {
	Vector2 pos1(35, 35);
	Vector2 pos2(35, 45);

	Vector4 selectedColour = Debug::GREEN;
	Vector4 normalColour = Debug::BLACK;

	Debug::Print("Paused", pos1, normalColour);
	Debug::Print("Return to Main Menu", pos2, selectedColour);
}
