#pragma once

#include "Scene.h"
#include "Ball.h"
#include "Material.h"
#include "Mesh.h"

class CornellScene : public Scene {
public:
	CornellScene();
	void Initialize();
	void Update();

	std::vector<Ball*> balls;

	~CornellScene();
};
