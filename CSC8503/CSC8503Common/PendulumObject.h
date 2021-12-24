#pragma once
#include "GameObject.h"
#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"

namespace NCL {
	namespace CSC8503 {
		class PendulumObject : public GameObject {
		public:
			PendulumObject(string name) : GameObject(name) {
				stateMachine = new StateMachine();

				State* A = new State([&](float dt)->void
					{
						SwingRight(dt);
					}
				);

				State* B = new State([&](float dt)->void
					{
						SwingLeft(dt);
					}
				);

				stateMachine->AddState(A);
				stateMachine->AddState(B);


				StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool
					{
						return timer >= 2.5f;
					}
				);
				StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool
					{
						return timer <= 0.0f;
					}
				);

				stateMachine->AddTransition(stateAB);
				stateMachine->AddTransition(stateBA);

			}
			~PendulumObject() {
				delete stateMachine;
			}

			bool UpdateObject(float dt) override;

		protected:

			StateMachine* stateMachine;
			void SwingRight(float dt);
			void SwingLeft(float dt);
			float timer;
		};
	}
}