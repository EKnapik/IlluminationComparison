#pragma once
#include <stdio.h>
#include <vector>

#include "GameMath.h"
#include "GameObject.h"
#include "Scene.h"

class GameManager {
public:
	std::vector<GameObject*>* GameObjects;
	std::vector<GameEntity*> GameEntities;

	void Update(float dt);

	void SetActiveScene(Scene* nextScene);

	void AddObject(GameObject* object) { GameObjects->push_back(object); }

	Scene* GetActiveScene() { return activeScene; }

	std::vector<SceneDirectionalLight>* GetDirectionalLights() { return &activeScene->DirectionalLights; }
	std::vector<ScenePointLight*>* GetPointLights() { return &activeScene->PointLights; }
	std::vector<ParticleEmitter*>* GetParticleEmitters() { return &activeScene->ParticleEmitters; }

	GameManager();

	~GameManager();

	bool EntitiesDirty, DirectionalLightsDirty, PointLightsDirty, ParticleEmittersDirty;

	int entitesSize;
	int gameObjectsSize;


private:
	Scene* activeScene = nullptr;
};