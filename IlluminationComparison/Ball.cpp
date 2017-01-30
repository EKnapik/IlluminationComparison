#include "Ball.h"

Ball::Ball()
{
	kinematics = new KinematicComponent();
	components.push_back(kinematics);
}

void Ball::Update(float dt)
{
	
}

void Ball::SetEntity(GameEntity * entity)
{
	kinematics->entity = entity;

	if (this->entity != nullptr)
	{
		this->entity->Release();
	}
	this->entity = entity;
	entity->AddReference();
}
