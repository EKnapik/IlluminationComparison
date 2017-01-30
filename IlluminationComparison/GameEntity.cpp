#include "GameEntity.h"

using namespace GMath;
GameEntity::GameEntity(std::string mesh, std::string material)
{
	// Initialize the member variables.
	SetIdentity4X4(&world);
	ZeroVec3(&position);
	ZeroVec3(&rotation);
	SetVector3(&scale, 1, 1, 1);

	this->mesh = mesh;
	this->material = material;
}

GameEntity::~GameEntity()
{
}

MAT4X4 * GameEntity::GetWorld()
{
#ifdef WITH_DX
	SetTransposeMatrix(&world, &(CreateScaleMatrix(&scale) * CreateRotationMatrix(&rotation) * CreateTranslationMatrix(&position)));
#else
	SetMatrx(&world, &(CreateScaleMatrix(&scale) * CreateRotationMatrix(&rotation) * CreateTranslationMatrix(&position)));
#endif
	return &world;
}

void GameEntity::Release()
{
	references--;
	if (references == 0)
	{
		delete this;
	}
}