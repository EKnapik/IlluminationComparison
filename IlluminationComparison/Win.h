#pragma once
#pragma once

#include "Scene.h"
#include "Ball.h"
#include "Material.h"
#include "Mesh.h"
#include "Panel.h"

class Win : public Scene {
public:
	Win();
	void Initialize();
	void Update();
	std::vector<Panel*> blocks;
	std::vector<ScenePointLight*> crazyLights;
	std::vector<Ball*> balls;
	GameObject* court;
	ScenePointLight* playerLight;
	ScenePointLight* ballLight;

	~Win();

private:
	bool spacePressed = false;

	void Shoot();
};