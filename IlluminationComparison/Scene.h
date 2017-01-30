#pragma once
#include "GameObject.h"
#include "Lights.h"
#include "ParticleEmitter.h"

class Scene {
public:
	virtual void Initialize() {};
	virtual void Update() {};
	virtual void End() {};
	void MarkForDelete(GameObject* obj) { obj->ToDelete = true; ObjectsDirty = true; }
	std::vector<GameObject*> GameObjects;
	std::vector<SceneDirectionalLight> DirectionalLights;
	std::vector<ScenePointLight*> PointLights;
	std::vector<ParticleEmitter*> ParticleEmitters;
	std::string SkyBox = "skybox";
	bool ObjectsDirty = false;
	virtual ~Scene(){ for (int i = 0; i < GameObjects.size(); i++) { delete GameObjects.at(i); }
					  for (int i = 0; i < PointLights.size(); i++) { delete PointLights.at(i); }
					  for (int i = 0; i < ParticleEmitters.size(); i++) { delete ParticleEmitters.at(i); } };
};