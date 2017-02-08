#include "PBRDemoScene.h"

PBRDemoScene::PBRDemoScene()
{
	Ball* ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "goldScuffed"));
	ball->kinematics->SetPosition(VEC3(-5, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "ironRusted4"));
	ball->kinematics->SetPosition(VEC3(-4, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "aluminumScuffed"));
	ball->kinematics->SetPosition(VEC3(-3, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "copperScuffed"));
	ball->kinematics->SetPosition(VEC3(-2, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "graniteSmooth"));
	ball->kinematics->SetPosition(VEC3(-1, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "greasyMetal"));
	ball->kinematics->SetPosition(VEC3(0, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "rust"));
	ball->kinematics->SetPosition(VEC3(1, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "bluePlastic"));
	ball->kinematics->SetPosition(VEC3(2, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "redPlastic"));
	ball->kinematics->SetPosition(VEC3(3, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "metalTest"));
	ball->kinematics->SetPosition(VEC3(4, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);
	ball = new Ball();
	ball->SetEntity(new GameEntity("sphere", "roughTest"));
	ball->kinematics->SetPosition(VEC3(5, 4, 0));
	balls.push_back(ball);
	GameObjects.push_back(ball);

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
	// Let's add some logic to make the balls bounce off the walls.
	for (int i = 0; i < balls.size(); i++)
	{
		float boxSize = 5;
		if (balls.at(i)->kinematics->GetPosition().x > boxSize)
		{
			balls.at(i)->kinematics->velocity.x = abs(balls.at(i)->kinematics->velocity.x) * -1;
		}
		if (balls.at(i)->kinematics->GetPosition().x < -boxSize)
		{
			balls.at(i)->kinematics->velocity.x = abs(balls.at(i)->kinematics->velocity.x);
		}

		if (balls.at(i)->kinematics->GetPosition().y > boxSize)
		{
			balls.at(i)->kinematics->velocity.y = abs(balls.at(i)->kinematics->velocity.y) * -1;
		}
		if (balls.at(i)->kinematics->GetPosition().y < -boxSize)
		{
			balls.at(i)->kinematics->velocity.y = abs(balls.at(i)->kinematics->velocity.y);
		}

		if (balls.at(i)->kinematics->GetPosition().z > boxSize)
		{
			balls.at(i)->kinematics->velocity.z = abs(balls.at(i)->kinematics->velocity.z) * -1;
		}
		if (balls.at(i)->kinematics->GetPosition().z < -boxSize)
		{
			balls.at(i)->kinematics->velocity.z = abs(balls.at(i)->kinematics->velocity.z);
		}

		for (int p = i + 1; p < balls.size(); p++)
		{
			VEC3 distanceVec;
			GMath::AddVec3(&distanceVec, &balls.at(i)->kinematics->GetPosition(),
				(GMath::VectorScale(&GMath::GetVector(&balls.at(p)->kinematics->GetPosition()), -1.0f)));

			FLOAT distance;
			GMath::GetMagnitude(&distance, &distanceVec);

			if (distance < balls.at(i)->radius + balls.at(p)->radius)
			{
				GMath::Vec3Normalize(&GMath::GetVector(&distanceVec));
				float velocityMag;
				GMath::GetMagnitude(&velocityMag, &balls.at(i)->kinematics->velocity);
				GMath::SetVector3(&balls.at(i)->kinematics->velocity, distanceVec.x, distanceVec.y, distanceVec.z);
				GMath::VectorScale(&balls.at(i)->kinematics->velocity, velocityMag);

				GMath::GetMagnitude(&velocityMag, &balls.at(p)->kinematics->velocity);
				GMath::SetVector3(&balls.at(p)->kinematics->velocity, distanceVec.x, distanceVec.y, distanceVec.z);
				GMath::VectorScale(&balls.at(p)->kinematics->velocity, -velocityMag);
			}
		}
	}
}

PBRDemoScene::~PBRDemoScene()
{
}
