#include "Win.h"

Win::Win()
{
	PointLights.push_back(new ScenePointLight(
		VEC4(0.3f, 0.5f, 0.3f, 1.0f),
		VEC3(0, 10, 20), 20.0f));

	DirectionalLights.push_back(SceneDirectionalLight(
		VEC4(0.1f, 0.1f, 0.1f, 1.0f),
		VEC4(1.0f, 1.0f, 1.0f, 1.0f),
		VEC3(0, 5, -10)));

	GameEntity* playEntity = new GameEntity("cube", "win");
	playEntity->SetScale(VEC3(4, 4, 4));
	playEntity->SetRotation(VEC3(0, 0, 3 * PI / 2));
	playEntity->SetPosition(VEC3(0, 0, 0));
	Panel* play = new Panel();
	play->SetEntity(playEntity);
	play->SetWidthHeight(1, 1);
	play->AddComponent(new WinComponent());
	blocks.push_back(play);
	GameObjects.push_back(play);

	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(5, 0, 4), VEC3(0.5f, 12.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(0.1, 0.1f, 1.0f, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));
	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(-5, 0, 4), VEC3(-0.5f, 12.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(0.1, 0.1f, 1.0f, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));
	ParticleEmitters.push_back(new ParticleEmitter(
		VEC3(0, 0, 4), VEC3(0.0f, 12.5f, -1.0f), VEC3(0, -5.0f, 0),
		VEC4(0.1, 0.1f, 1.0f, 0.4f), VEC4(0.1, 1, 1.0f, 0.1f), VEC4(0, 0.6f, 1.0f, 0),
		0.1f, 2.0f, 3.0f, 1000.0f, 15.0f));
}

void Win::Initialize()
{
}

void Win::Update()
{
	Shoot();

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

Win::~Win()
{
}

void Win::Shoot()
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
