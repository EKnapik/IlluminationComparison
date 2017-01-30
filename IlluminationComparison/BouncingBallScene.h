#pragma once

#include "Scene.h"
#include "Ball.h"
#include "Material.h"
#include "Mesh.h"

class BouncingBallScene : public Scene {
public:
	BouncingBallScene();
	void Initialize();
	void Update();

	std::vector<Ball*> balls;

	~BouncingBallScene();
};
