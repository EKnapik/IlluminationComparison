#include "KinematicsComponent.h"

KinematicComponent::KinematicComponent()
{
}

void KinematicComponent::Update(float dt)
{
	velocity.x += acceleration.x * dt;
	velocity.y += acceleration.y * dt;
	velocity.z += acceleration.z * dt;

	if (entity != nullptr)
	{
		oldPosition = entity->GetPosition();
		entity->SetPosition(VEC3(oldPosition.x + velocity.x * dt, oldPosition.y + velocity.y * dt, oldPosition.z + velocity.z * dt));
	}
	else
	{
		oldPosition = position;
		position = VEC3(oldPosition.x + velocity.x * dt, oldPosition.y + velocity.y * dt, oldPosition.z + velocity.z * dt);
	}
}

KinematicComponent::~KinematicComponent()
{
	
}
