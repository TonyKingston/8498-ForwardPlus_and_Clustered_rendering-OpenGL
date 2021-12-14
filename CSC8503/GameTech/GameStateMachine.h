#pragma once
#include "../CSC8503Common/PushdownMachine.h"

namespace NCL {
	namespace CSC8503 {

		class GameStateMachine : public PushdownMachine {
		public:
			GameStateMachine(PushdownState* initialState) : PushdownMachine(initialState) {

			}
		private:

		};
	}
}

