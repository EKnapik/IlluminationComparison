#include "GameObject.h"

void GameObject::SetEntity(GameEntity* newEntity)
{
	if (entity)
	{
		entity->Release();
	}

	entity = newEntity;
	newEntity->AddReference();
}

void GameObject::AddComponent(Component* component)
{
	components.push_back(component);
}

GameObject::~GameObject()
{
	if (entity)
	{
		entity->Release();
	}

	for (int i = 0; i < components.size(); i++)
	{
		delete components.at(i);
	}
}
