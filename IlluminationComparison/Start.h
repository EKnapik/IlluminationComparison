#pragma once

#include "Scene.h"
#include "Ball.h"
#include "Material.h"
#include "Mesh.h"
#include "Panel.h"

class Start : public Scene {
public:
	Start();
	void Initialize();
	void Update();
	std::vector<Panel*> blocks;
	std::vector<Ball*> balls;
	ScenePointLight* pointLight;
	bool grow = true;

	~Start();

private:
	bool spacePressed = false;

	void Shoot();
};