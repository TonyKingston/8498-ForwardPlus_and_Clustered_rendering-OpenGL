#pragma once
#include <functional>

namespace NCL {
	namespace CSC8503 {
		class State {
			typedef std::function<void(float)> StateUpdateFunc;
		public:
			State() {}
			State(StateUpdateFunc someFunc) {
				func = someFunc;
			}
			virtual ~State() {}
			//virtual void Update() = 0; //Pure virtual base class
			void Update(float dt) {
				if (func != nullptr) {
					func(dt);
				}
			}
		protected:
			StateUpdateFunc func;
		};

		typedef void(*StateFunc)(void*);

	/*	class GenericState : public State		{
		public:
			GenericState(StateFunc someFunc, void* someData) {
				func		= someFunc;
				funcData	= someData;
			}
			virtual void Update() {
				if (funcData != nullptr) {
					func(funcData);
				}
			}
		protected:
			StateFunc func;
			void* funcData;
		};*/
	}
}