#pragma once
#include "GameObject.h"

class KinematicComponent : public Component {
public:
	KinematicComponent();
	void Update(float dt);
	VEC3 GetPosition() { return entity != nullptr ? entity->GetPosition() : position; }
	void SetPosition(VEC3 pos) { entity != nullptr ? entity->SetPosition(pos) : position = pos; }
	VEC3 oldPosition = VEC3(0, 0, 0);
	VEC3 velocity = VEC3(0, 0, 0);
	VEC3 acceleration = VEC3(0, 0, 0);
	virtual ~KinematicComponent();
	GameEntity* entity = nullptr;
private:
	VEC3 position;
};