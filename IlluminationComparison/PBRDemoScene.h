#pragma once

#include "Scene.h"
#include "Ball.h"
#include "Material.h"
#include "Mesh.h"

class PBRDemoScene : public Scene {
public:
	PBRDemoScene();
	void Initialize();
	void Update();

	std::vector<Ball*> balls;

	~PBRDemoScene();
};
