#include "PBRDemoScene.h"

PBRDemoScene::PBRDemoScene()
{
	Ball* ball;
	int depthNum = 4;
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "goldScuffed"));
		ball->kinematics->SetPosition(VEC3(-3.5, 1, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "ironRusted4"));
		ball->kinematics->SetPosition(VEC3(-2.5, 1, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "aluminumScuffed"));
		ball->kinematics->SetPosition(VEC3(-1.5, 1, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "copperScuffed"));
		ball->kinematics->SetPosition(VEC3(-.5, 1, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "graniteSmooth"));
		ball->kinematics->SetPosition(VEC3(0.5, 1, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "greasyMetal"));
		ball->kinematics->SetPosition(VEC3(1.5, 1, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "rust"));
		ball->kinematics->SetPosition(VEC3(2.5, 1, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "bluePlastic"));
		ball->kinematics->SetPosition(VEC3(3.5, 1, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "redPlastic"));
		ball->kinematics->SetPosition(VEC3(4.5, 1, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	/*
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "metalTest"));
		ball->kinematics->SetPosition(VEC3(5, 4, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	for (int i = 0; i < depthNum; i++)
	{
		ball = new Ball();
		ball->SetEntity(new GameEntity("sphere", "roughTest"));
		ball->kinematics->SetPosition(VEC3(6, 4, i));
		balls.push_back(ball);
		GameObjects.push_back(ball);
	}
	*/

	/*
	ball = new Ball();
	ball->SetEntity(new GameEntity("mountains", "caveTexture"));
	ball->entity->Scale(VEC3(3.0f, 3.0f, 3.0f));
	ball->kinematics->SetPosition(VEC3(0, -25, 50));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	*/

	/*
	ball = new Ball();
	ball->SetEntity(new GameEntity("cube", "goldScuffed"));
	ball->entity->Scale(VEC3(9.0f, 2.0f, 4.0f));
	ball->kinematics->SetPosition(VEC3(0.0, -1.25f, 2.0f));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	*/


	PointLights.push_back(new ScenePointLight(
		VEC4(1.0f, 0.5f, 0.3f, 1.0f),
		VEC3(0, 0, 0), 20.0f));

	PointLights.push_back(new ScenePointLight(
		VEC4(0.1f, 1.0f, 0.3f, 1.0f),
		VEC3(0, 2, 0), 6.0f));

	DirectionalLights.push_back(SceneDirectionalLight(
		VEC4(0.1f, 0.1f, 0.1f, 1.0f),
		VEC4(1.0f, 1.0f, 1.0f, 1.0f),
		VEC3(0, 5, -10)));
}

void PBRDemoScene::Initialize()
{
	
}

void PBRDemoScene::Update()
{
}

PBRDemoScene::~PBRDemoScene()
{
}
