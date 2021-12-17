#include "PushdownMachine.h"
#include "PushdownState.h"
using namespace NCL::CSC8503;

PushdownMachine::PushdownMachine(PushdownState* initialState)
{
	this->initialState = initialState;
	activeState = nullptr;
}

PushdownMachine::~PushdownMachine()
{
}

bool PushdownMachine::Update(float dt) {
	if (activeState) {
		PushdownState* newState = nullptr;
		PushdownState::PushdownResult result = 
			activeState->OnUpdate(dt, &newState);

		switch (result) {
			case PushdownState::Pop: {
				activeState->OnSleep();
				stateStack.pop();
				if (stateStack.empty()) {
					activeState = nullptr; //??????
					return false;
				}
				else {
					activeState = stateStack.top();
					activeState->OnAwake();
				}
			}break;
			case PushdownState::Push: {
				activeState->OnSleep();
				stateStack.push(newState);
				activeState = newState;
				newState->OnAwake();
			}break;
			case PushdownState::Reset: {
				activeState->OnSleep();
				Reset();
			}break;
		}
	}
	else {
		stateStack.push(initialState);
		activeState = initialState;
		activeState->OnAwake();
	}
	return true;
}

void NCL::CSC8503::PushdownMachine::Reset() {
	while (stateStack.size() > 1) {
		stateStack.pop();
	}
	activeState = initialState;
}
