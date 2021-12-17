#include "../../Common/Window.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "..//CSC8503Common/StateMachine.h"
#include "..//CSC8503Common/State.h"
#include "..//CSC8503Common/StateTransition.h"
#include "../CSC8503Common/BehaviourAction.h"
#include "../CSC8503Common/BehaviourSequence.h"
#include "../CSC8503Common/BehaviourSelector.h"
#include "..//CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "GamePushdownState.h"
#include "GameState.h"

using namespace NCL;
using namespace CSC8503;


/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead.

This time, we've added some extra functionality to the window class - we can
hide or show the

*/

class PauseScreen : public PushdownState {
	PushdownResult OnUpdate(float dt,
		PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::U)) {
			return PushdownResult::Pop;

		}
		return PushdownResult::NoChange;

	}
	void OnAwake() override {
		std::cout << " Press U to unpause game !\n";

	}

};

class GameScreen : public PushdownState {
	PushdownResult OnUpdate(float dt,
		PushdownState** newState) override {
		pauseReminder -= dt;
		if (pauseReminder < 0) {
			std::cout << " Coins mined : " << coinsMined << "\n";
			std::cout << " Press P to pause game , or F1 to return to main menu !\n";
			pauseReminder += 1.0f;

		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
			*newState = new PauseScreen();
			return PushdownResult::Push;

		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F1)) {
			std::cout << " Returning to main menu !\n";
			return PushdownResult::Pop;

		}
		if (rand() % 7 == 0) {
			coinsMined++;

		}
		return PushdownResult::NoChange;

	};
	void OnAwake() override {
		std::cout << " Preparing to mine coins !\n";

	}
protected:
	int coinsMined = 0;
	float pauseReminder = 1;

};

class IntroScreen : public PushdownState {
	PushdownResult OnUpdate(float dt,
		PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
			*newState = new GameScreen();
			return PushdownResult::Push;

		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
			return PushdownResult::Pop;

		}
		return PushdownResult::NoChange;

	};

	void OnAwake() override {
		std::cout << " Welcome to a really awesome game !\n";
		std::cout << " Press Space To Begin or escape to quit !\n";

	}

};

void TestStateMachine();
void TestBehaviourTree();
vector < Vector3 > testNodes;
void TestPathfinding() {
	NavigationGrid grid("MazeGrid.txt");

	NavigationPath outPath;

	//Vector3 startPos(8, 9, 11);
	//Vector3 endPos(130, 10, 147);
	//Vector3 startPos(140, 10, 10);
	//Vector3 endPos(20, 10, 130);
	Vector3 startPos(10, 10, 10);
	Vector3 endPos(140, 10, 130);
	Debug::DrawLine(startPos, startPos * 1.01, Debug::BLUE, 60.0f);
	Debug::DrawLine(endPos, endPos * 1.01, Debug::BLUE, 60.0f);
	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);

	}

}
void DisplayPathfinding() {
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));

	}

}

int main() {
	GamePushdownState::InitGameState();
	//Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
	Window* w = GamePushdownState::GetGameWindow();
	if (!w->HasInitialised()) {
		return -1;
	}
	srand(time(0));
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);


	PushdownMachine machine(new MainMenu());
	TestPathfinding();
	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (dt > 0.1f) {
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

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		if (!machine.Update(dt)) {
			return -1;
		}
		DisplayPathfinding();
	}
	Window::DestroyGameWindow();
}

void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();
	int data = 0;

	State* A = new State([&](float dt)->void
		{
			std::cout << "I’m in state A!\n";
			data++;
		}
	);

	State* B = new State([&](float dt)->void
		{
			std::cout << "I’m in state B!\n";
			data--;
		}
	);

	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool
		{
			return data > 10;
		}
	);
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool
		{
			return data < 0;
		}
	);

	testMachine->AddState(A);
	testMachine->AddState(B);
	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);

	for (int i = 0; i < 100; ++i) {
		testMachine->Update(1.0f);
	}
}

void TestBehaviourTree() {
	float behaviourTimer;
	float distanceToTarget;
	BehaviourAction* findKey = new BehaviourAction(" Find Key ",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << " Looking for a key !\n";
				behaviourTimer = rand() % 100;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				behaviourTimer -= dt;
				if (behaviourTimer <= 0.0f) {
					std::cout << " Found a key !\n";
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);

	BehaviourAction* goToRoom = new BehaviourAction("Go To Room",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << " Going to the loot room !\n";
				state = Ongoing;
			}
			else if (state == Ongoing) {
				distanceToTarget -= dt;
				if (distanceToTarget <= 0.0f) {
					std::cout << "Reached room !\n";
					return Success;

				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);

	BehaviourAction* openDoor = new BehaviourAction(" Open Door ",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Opening Door !\n";
				return Success;
			}
			return state;
		}
	);

	BehaviourAction* lookForTreasure = new BehaviourAction(
		" Look For Treasure ",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << " Looking for treasure !\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some treasure !\n";
					return Success;

				}
				std::cout << "No treasure in here ...\ n";
				return Failure;

			}
			return state;
		}
	);

	BehaviourAction* lookForItems = new BehaviourAction(
		" Look For Items ",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << " Looking for items !\n";
				return Ongoing;

			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some items !\n";
					return Success;

				}
				std::cout << "No items in here ...\ n";
				return Failure;

			}
			return state;
		}
	);
	BehaviourSequence* sequence =
		new BehaviourSequence(" Room Sequence ");
	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	BehaviourSelector* selection =
		new BehaviourSelector("Loot Selection");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);

	BehaviourSequence* rootSequence =
		new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);

	for (int i = 0; i < 5; ++i) {
		rootSequence->Reset();
		behaviourTimer = 0.0f;
		distanceToTarget = rand() % 250;
		BehaviourState state = Ongoing;
		std::cout << "We ’re going on an adventure !\n";
		while (state == Ongoing) {
			state = rootSequence->Execute(1.0f); // fake dt

		}
		if (state == Success) {
			std::cout << " What a successful adventure !\n";

		}
		else if (state == Failure) {
			std::cout << " What a waste of time !\n";

		}
	}
	std::cout << "All done !\n";

}