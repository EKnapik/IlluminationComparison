#include "Start.h"
#include "Game.h"

Start::Start()
{
	pointLight = new ScenePointLight(
		VEC4(0.3f, 0.5f, 0.3f, 1.0f),
		VEC3(0, 0, 0), 40.0f);
	PointLights.push_back(pointLight);

	DirectionalLights.push_back(SceneDirectionalLight(
		VEC4(0.1f, 0.1f, 0.1f, 1.0f),
		VEC4(1.0f, 1.0f, 1.0f, 1.0f),
		VEC3(0, 5, -10)));
	
	GameEntity* playEntity = new GameEntity("cube", "play");
	playEntity->SetScale(VEC3(4, 4, 4));
	playEntity->SetRotation(VEC3(0, 0, 3 * PI / 2));
	playEntity->SetPosition(VEC3(-5, 0, 5));
	Panel* play = new Panel();
	play->SetEntity(playEntity);
	play->SetWidthHeight(1, 1);
	play->AddComponent(new StartComponent());
	blocks.push_back(play);
	GameObjects.push_back(play);

	GameEntity* quitEntity = new GameEntity("cube", "quit");
	quitEntity->SetScale(VEC3(4, 4, 4));
	quitEntity->SetRotation(VEC3(0, 0, 3 * PI / 2));
	quitEntity->SetPosition(VEC3(5, 0, 5));
	Panel* quit = new Panel();
	quit->SetEntity(quitEntity);
	quit->SetWidthHeight(1, 1);
	quit->AddComponent(new QuitComponent());
	blocks.push_back(quit);
	GameObjects.push_back(quit);

	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(-6, 0, 4), VEC3(0.0f, 8.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(1.0f, 0, 0, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));
	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(-4, 0, 4), VEC3(0.0f, 8.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(1.0f, 0.5f, 0.0f, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));
	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(-2, 0, 4), VEC3(0.0f, 8.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(1.0f, 1.0f, 0.0f, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));
	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(0, 0, 4), VEC3(0.0f, 8.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(0.0f, 1.0f, 0.0f, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));
	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(2, 0, 4), VEC3(0.0f, 8.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(0.0, 0.0f, 1.0f, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));
	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(4, 0, 4), VEC3(0.0f, 8.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(0.25f, 0.0f, 0.5f, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));
	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(6, 0, 4), VEC3(0.0f, 8.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(0.95f, 0.9f, 1.0f, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));

}

void Start::Initialize()
{
}

void Start::Update()
{
	Shoot();

	if (grow == true)
	{
		pointLight->Radius.x++;
		pointLight->Radius.y++;
		pointLight->Radius.z++;
		if (pointLight->Radius.x > 200)
		{
			grow = false;
		}
	}
	else
	{
		pointLight->Radius.x--;
		pointLight->Radius.y--;
		pointLight->Radius.z--;
		if (pointLight->Radius.x < 100)
		{
			grow = true;
		}
	}

	for (int b = 0; b < balls.size(); b++)
	{
		if (balls.at(b)->kinematics->GetPosition().y < 0.0f)
		{
			balls.at(b)->kinematics->velocity.y = abs(balls.at(b)->kinematics->velocity.y);
		}

		for (int p = 0; p < blocks.size(); p++)
		{
			VEC3 distanceVec;
			GMath::AddVec3(&distanceVec, &balls.at(b)->kinematics->GetPosition(),
				(GMath::VectorScale(&GMath::GetVector(&blocks.at(p)->entity->GetPosition()), -1.0f)));

			FLOAT distance;
			GMath::GetMagnitude(&distance, &distanceVec);

			if (distance < balls.at(b)->radius + blocks.at(p)->width)
			{
				GMath::Vec3Normalize(&GMath::GetVector(&distanceVec));
				float velocityMag;
				GMath::GetMagnitude(&velocityMag, &balls.at(b)->kinematics->velocity);
				VEC3 bounce;
				GMath::SetVector3(&bounce, distanceVec.x, distanceVec.y, distanceVec.z);
				GMath::VectorScale(&bounce, 3.0f);
				GMath::AddVec3(&balls.at(b)->kinematics->velocity, &balls.at(b)->kinematics->velocity, &bounce);
				MarkForDelete(blocks.at(p));
				blocks.erase(blocks.begin() + p);

				p--;
			}
		}
	}
}

Start::~Start()
{
}

void Start::Shoot()
{
	if (GetAsyncKeyState(' ') & 0x8000)
	{
		if (spacePressed == false)
		{
			GameEntity* BallEntity = new GameEntity("soccer", "white");
			Ball* ball = new Ball();
			ball->SetEntity(BallEntity);
			ball->kinematics->velocity = *Game::GetCamera()->GetDirection();
			GMath::VectorScale(&ball->kinematics->velocity, 10);
			ball->kinematics->acceleration = VEC3(0, -2.0f, 0);
			ball->kinematics->SetPosition(*Game::GetCamera()->GetPosition());
			ball->entity->SetScale(VEC3(0.1f, 0.1f, 0.1f));
			GameObjects.push_back(ball);
			balls.push_back(ball);
			ObjectsDirty = true;
		}
		spacePressed = true;
	}
	else
	{
		spacePressed = false;
	}
}
