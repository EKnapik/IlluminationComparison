#pragma once

#include "Game.h"
#include "Scene.h"
#include "GameObject.h"
#include "ParticleEmitter.h"

class Panel : public GameObject {
public:
	Panel();
	float width = 1, height = 1;
	void SetWidthHeight(float width, float height) { this->width = width; this->height = height; };
};

class ParticleComponent : public Component {
public:
	ParticleComponent(GameObject* parentObject, Scene* parentScene);
	void Update(float dt);
	virtual ~ParticleComponent();
	GameEntity* entity = nullptr;
private:
	ParticleEmitter* emitter;
	GameObject* ParentObject;
	Scene* ParentScene;
};

class PostProcessComponent : public Component {
public:
	PostProcessComponent() {};
	void Update(float dt) {};
	virtual ~PostProcessComponent() { Game::postProcessState = rand() % POST_PROCESS::NO_CHANGE; };
};

class StartComponent : public Component {
public:
	StartComponent() {};
	void Update(float dt) {};
	virtual ~StartComponent() { Game::levelstate = LEVEL_STATE::MAIN; };
};

class QuitComponent : public Component {
public:
	QuitComponent() {};
	void Update(float dt) {};
	virtual ~QuitComponent() { Game::levelstate = LEVEL_STATE::QUIT; };
};

class WinComponent : public Component {
public:
	WinComponent() {};
	void Update(float dt) {};
	virtual ~WinComponent() { Game::levelstate = LEVEL_STATE::START; };
};

class LoseComponent : public Component {
public:
	LoseComponent() {};
	void Update(float dt) {};
	virtual ~LoseComponent() { Game::levelstate = LEVEL_STATE::LOSE; };
};