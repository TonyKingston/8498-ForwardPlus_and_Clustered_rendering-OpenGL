#include "../../Common/Window.h"
#include "../GameTech/TutorialGame.h"
#include <iostream>
#include <fstream>

using namespace NCL;
using namespace CSC8503;

//extern "C" _declspec(dllexport) DWORD NvOptimusEnablement = 1;


int main() {
	
	int frameCount = 0;
	bool fileOpen = false;
	bool init = true;
	bool reachedCapacity = false;
	const std::string OUTPUTDIR("../../Data/");
	std::ofstream file;
	file.open(OUTPUTDIR + "forward-final.csv", std::ios_base::app);
	fileOpen = true;
	file << "frame time" << "," << "Number of lights" << std::endl;

	//GamePushdownState::InitGameState();
	//Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
	Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1920, 1080, true, 0, 0);
	//w->SetFullScreen(false);
	//Window* w = GamePushdownState::GetGameWindow();
	if (!w->HasInitialised()) {
		return -1;
	}
	srand(time(0));
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);
	
	TutorialGame* g = new TutorialGame();
	//PushdownMachine machine(new MainMenu());
	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE) && !reachedCapacity) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (dt > 0.12f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
			w->SetWindowPosition(0, 0);
		}
		float frameTime = 1000.0f * dt;
		w->SetTitle("Frame time:" + std::to_string( frameTime) + "    No. of Lights:" + std::to_string(g->GetRenderer()->GetNumLight()));
		if (frameCount > 300) {
			if (frameCount <= 800) {
				file << frameTime << "," << g->GetRenderer()->GetNumLight() << std::endl;
			}
			else {
				int count = g->GetRenderer()->GetNumLight() == 0 ? 1 : g->GetRenderer()->GetNumLight();
				reachedCapacity = !g->GetRenderer()->AddLights(count);
				frameCount = 240;
			}
		}
		frameCount++;

		g->UpdateGame(dt);

		/*if (!machine.Update(dt)) {
			return -1;
		}*/
		//DisplayPathfinding();
	}
	Window::DestroyGameWindow();
	if (file.is_open()) {
		file.close();
	}

}