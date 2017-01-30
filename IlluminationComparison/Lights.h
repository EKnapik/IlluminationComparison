#pragma once
#include "GameMath.h"

struct DirectionalLight {
	VEC4 AmbientColor;
	VEC4 DiffuseColor;
	VEC3 Direction;
};

struct PointLight {
	VEC4 Color;
	VEC3 Position;
	FLOAT Radius;
};

class SceneDirectionalLight {
public:
	SceneDirectionalLight(VEC4 aColor, VEC4 dColor, VEC3 direction);
	VEC4 AmbientColor;
	VEC4 DiffuseColor;
	VEC3 Direction;
	MAT4X4 ViewMatrix;
};

class ScenePointLight {
public:
	ScenePointLight(VEC4 color, VEC3 position, float radius);
	VEC4 Color;
	VEC3 Position;
	VEC3 Radius;
	MAT4X4 ViewMatrix;
};