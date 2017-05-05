#include "CornellScene.h"

CornellScene::CornellScene()
{
	Ball* ball;
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "aluminumScuffed"));
	ball->kinematics->SetPosition(VEC3(-1, 3, 3));
	ball->entity->Scale(VEC3(6.0f, 6.0f, 6.0f));
	balls.push_back(ball);
	GameObjects.push_back(ball);

	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "redPlastic"));
	ball->kinematics->SetPosition(VEC3(4, 0, 1));
	ball->entity->Scale(VEC3(4.0f, 4.0f, 4.0f));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	

	// Add 5 Walls
	// left
	ball = new Ball();
	ball->SetEntity(new GameEntity("cube", "greenWall"));
	ball->entity->Scale(VEC3(0.75f, 16.0f, 16.0f));
	ball->kinematics->SetPosition(VEC3(-9.0, 0.0f, 0.0f));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	// right
	ball = new Ball();
	ball->SetEntity(new GameEntity("cube", "blueWall"));
	ball->entity->Scale(VEC3(0.75f, 16.0f, 16.0f));
	ball->kinematics->SetPosition(VEC3(9.0, 0.0f, 0.0f));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	// top
	/*
	ball = new Ball();
	ball->SetEntity(new GameEntity("cube", "whiteWall"));
	ball->entity->Scale(VEC3(16.0f, 0.125f, 16.0f));
	ball->kinematics->SetPosition(VEC3(0.0, 8.0f, 0.0f));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	*/
	// bottom
	ball = new Ball();
	ball->SetEntity(new GameEntity("cube", "whiteWall"));
	ball->entity->Scale(VEC3(16.0f, 0.75f, 16.0f));
	ball->kinematics->SetPosition(VEC3(0.0, -6.0f, 0.0f));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	// back
	ball = new Ball();
	ball->SetEntity(new GameEntity("cube", "whiteWall"));
	ball->entity->Scale(VEC3(16.0f, 16.0f, 0.75f));
	ball->kinematics->SetPosition(VEC3(0.0, 0.0f, 8.0f));
	balls.push_back(ball);
	GameObjects.push_back(ball);

	

	PointLights.push_back(new ScenePointLight(
		VEC4(1.0f, 0.5f, 0.3f, 1.0f),
		VEC3(0, 7, 0), 20.0f));

	DirectionalLights.push_back(SceneDirectionalLight(
		VEC4(0.1f, 0.1f, 0.1f, 1.0f),
		VEC4(1.0f, 1.0f, 1.0f, 1.0f),
		VEC3(0, 5, -10)));
}

void CornellScene::Initialize()
{

}

void CornellScene::Update()
{
}

CornellScene::~CornellScene()
{
}
