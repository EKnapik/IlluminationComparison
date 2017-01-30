#include "Lights.h"

using namespace GMath;

SceneDirectionalLight::SceneDirectionalLight(VEC4 aColor, VEC4 dColor, VEC3 direction)
{
	AmbientColor = aColor;
	DiffuseColor = dColor;
	Direction = direction;

	SetMatrixLookAtLH(&ViewMatrix, 
		&VEC4(direction.x, direction.y, direction.z, 0), 
		&VEC4(0, 0, 0, 0),
		&VEC4(0, 1, 0, 0));
}


ScenePointLight::ScenePointLight(VEC4 color, VEC3 position, float radius)
{
	Color = color;
	Position = position;
	// Multiple by 2 so the radius of sphere for light looks good
	Radius = VEC3(radius, radius, radius);
}
