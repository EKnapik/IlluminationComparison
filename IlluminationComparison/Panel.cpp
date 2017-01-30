#include "Panel.h"

Panel::Panel()
{
}

ParticleComponent::ParticleComponent(GameObject * parentObject, Scene * parentScene)
{
	ParentObject = parentObject;
	ParentScene = parentScene;

	emitter = new ParticleEmitter(ParticleEmitter("particle", "default",
		VEC3(2.0f, 0, 0), VEC3(-2.0f, 2.0f, 0), VEC3(0, -1.0f, 0),
		VEC4(1, 0.1f, 0.1f, 0.2f), VEC4(1, 1, 0.1f, 0.1f), VEC4(1, 0.6f, 0.1f, 0),
		0.1f, 5.0f, 5.0f, 3000.0f, 5.0f));

	ParentScene->ParticleEmitters.push_back(emitter);
}

void ParticleComponent::Update(float dt)
{
	emitter->Position = ParentObject->entity->GetPosition();
}

ParticleComponent::~ParticleComponent()
{
	
}
