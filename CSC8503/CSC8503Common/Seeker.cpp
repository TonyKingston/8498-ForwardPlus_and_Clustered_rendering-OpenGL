#include "Seeker.h"

/*NCL::CSC8503::Seeker::Seeker() {
}

NCL::CSC8503::Seeker::~Seeker() {
}

void NCL::CSC8503::Seeker::Flocking(float dt) {
	for (Seeker* seeker : game->GetSeekers()) {
		Vector3 pos = seeker->GetTransform().GetPosition();
		Vector2 dir;
		dir += Alignment();
		dir += Separation();
		dir += Cohesion();
		//dir += Avoidance();
		seeker->GetTransform().SetPosition(pos + dir * dt);
	}
}

Vector2 NCL::CSC8503::Seeker::Alignment() {
	Vector2 direction = GetPhysicsObject()->GetLinearVelocity().Normalised();
	for (Seeker* seeker : game->GetSeekers()) {
		Vector3 pos = seeker->GetTransform().GetPosition();

		if (seeker == this) {
			continue;
		}
		float distance = (this->GetTransform().GetPosition() - pos).Length();
		if (distance > 16.0f) {
			continue;
		}
		direction += seeker->GetPhysicsObject()->GetLinearVelocity().Normalised();
	}
	return direction.Normalised();
}

Vector2 NCL::CSC8503::Seeker::Separation() {
	Vector2 direction = GetPhysicsObject()->GetLinearVelocity();
	for (Seeker* seeker : game->GetSeekers()) {
		Vector3 pos = seeker->GetTransform().GetPosition();
		if (seeker == this) {
			continue;
		}
		float distance = (transform.GetPosition() - pos).Length();
		if (distance > 20.0f) {
			continue;
		}
		float strength = 1.0f - (distance / 20);
		if (distance <= 5.0f) {
			strength *= 10;
		}
		direction += (transform.GetPosition() - pos).Normalised() * strength;
	}
	return direction.Normalised();
}

Vector2 NCL::CSC8503::Seeker::Cohesion() {
	Vector2 avgPos = transform.GetPosition();
	float count = 1;
	for (Seeker* seeker : game->GetSeekers()) {
		Vector3 pos = seeker->GetTransform().GetPosition();
		if (seeker == this) {
			continue;
		}
		float distance = (transform.GetPosition() - pos).Length();
		if (distance > 16.0f) {
			continue;
		}
		avgPos += pos;
		count += 1;
	}
	avgPos = avgPos / count;
	return (avgPos - transform.GetPosition()).Normalised();
}

Vector2 NCL::CSC8503::Seeker::Avoidance() {
	Vector2 direction;
	/*for (Guard* guard : game->getGuards()) {
		float distance = (position - guard->GetPosition()).Length();
		if (distance < 32.0f) {
			//AddForce((position - guard->GetPosition()).Normalised());
			direction += (position - guard->GetPosition()).Normalised();
		}
	}
	return direction.Normalised();
}
*/