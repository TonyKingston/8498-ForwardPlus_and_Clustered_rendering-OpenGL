#include "GamePushdownState.h"
#include "../CSC8503Common/Debug.h"
#include "GameState.h"
#include "../../Common/Window.cpp"


using namespace NCL::CSC8503;

GameState* GamePushdownState::gameState = nullptr;

PushdownState::PushdownResult MainMenu::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP)) {
		currentOption = currentOption > 0 ? currentOption - 1 : MAX_OPTIONS - 1;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN)) {
		currentOption = currentOption < MAX_OPTIONS - 1 ? currentOption + 1 : 0;
	}
	Debug::FlushRenderables(dt);
	renderer->Update(dt);
	renderer->Render();
	DrawMenu();

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN)) {
		switch (currentOption) {
		case 0:
			*newState = new Game(1);
			return PushdownResult::Push;
			break;
		case 1:
			*newState = new Game(2);
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
	gameWorld = new GameWorld();
	renderer = new GameTechRenderer(*gameWorld);
	Debug::SetRenderer(renderer);
	//renderer = gameState->GetRenderer();
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

NCL::CSC8503::Game::Game(int gameId) {
	this->gameId = gameId;
	game = new TutorialGame(gameId);
}

PushdownState::PushdownResult Game::OnUpdate(float dt, PushdownState** newState) {
	if (game) {
		
		game->UpdateGame(dt);

		if (game->GetGameStatus() == -1 || game->GetGameStatus() == 1) {
			*newState = new Results(game);

			return PushdownResult::Push;
		} 

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
			*newState = new PauseMenu(game);

			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}
}
void Game::OnAwake() {
	//game = gameState->GetGame();
}

/*void Game::OnSleep() {
	delete game;
}*/

NCL::CSC8503::PauseMenu::PauseMenu(TutorialGame* game) {
	this->game = game;
}

PushdownState::PushdownResult PauseMenu::OnUpdate(float dt, PushdownState** newState) {

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		return PushdownResult::Pop;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN)) {
		return PushdownResult::Reset;
	}

	game->GetRenderer()->Update(dt);
	game->GetRenderer()->Render();
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

	game->GetRenderer()->DrawString("Paused", pos1, normalColour);
	game->GetRenderer()->DrawString("Return to Main Menu", pos2, selectedColour);
}

void NCL::CSC8503::GamePushdownState::InitGameState() {
	gameState = new GameState();
}

Window* NCL::CSC8503::GamePushdownState::GetGameWindow() {
	return gameState->GetWindow();
}

NCL::CSC8503::Results::Results(TutorialGame* game) {
	score = game->GetPlayerScore();
	time = game->GetGameTime();
	this->game = game;
}

PushdownState::PushdownResult Results::OnUpdate(float dt, PushdownState** newState) {
	Vector2 pos1(35, 35);
	Vector2 pos2(25, 45);
	Vector2 pos3(45, 45);
	if (game->GetGameStatus() == -1) {
		game->GetRenderer()->DrawString("You Lose", pos1, Debug::RED);
	}
	else {
		game->GetRenderer()->DrawString("Victory!", pos1, Debug::GREEN);
	}
	game->GetRenderer()->DrawString("Score: " + std::to_string(score), pos2, Debug::BLACK);
	game->GetRenderer()->DrawString("Time: " + std::to_string(time), pos3, Debug::BLACK);
	game->GetRenderer()->Update(dt);
	game->GetRenderer()->Render();
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN)) {
		return PushdownResult::Reset;
	}
	return PushdownResult::NoChange;
}

void Results::OnAwake() {

}

void Results::OnSleep() {
	delete game;
}
