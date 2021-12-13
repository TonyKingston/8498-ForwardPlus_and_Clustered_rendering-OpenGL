#pragma once
#include <stack>

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		class PushdownMachine {
		public:
			PushdownMachine(PushdownState* initialState);
			~PushdownMachine();

			bool Update(float dt);

			void Reset() {
				activeState = initialState;
			}

		protected:
			PushdownState* activeState;
			PushdownState* initialState;

			std::stack<PushdownState*> stateStack;
		};
	}
}

